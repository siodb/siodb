// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerSqlConnectionHandler.h"

// Project headers
#include "IOManagerRequest.h"
#include "../dbengine/SessionGuard.h"
#include "../dbengine/handlers/RequestHandler.h"
#include "../dbengine/parser/DBEngineRequestFactoryError.h"
#include "../dbengine/parser/DBEngineSqlRequestFactory.h"
#include "../dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/net/ConnectionError.h>
#include <siodb/common/net/EpollHelpers.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/utils/ErrorCodeChecker.h>
#include <siodb/common/utils/SignalHandlers.h>

// Protobuf message headers
#include <siodb/common/proto/IOManagerProtocol.pb.h>

namespace siodb::iomgr {

// ----- internals -----

void IOManagerSqlConnectionHandler::threadLogicImpl()
{
    dbengine::AuthenticationResult authResult;
    try {
        authResult = authenticateUser();
    } catch (std::exception& e) {
        closeConnection();
        return;
    }

    dbengine::SessionGuard guard(m_requestDispatcher.getInstance(), authResult.m_sessionUuid);
    const auto requestHandler = std::make_shared<dbengine::RequestHandler>(
            m_requestDispatcher.getInstance(), *m_clientConnection, authResult.m_userId);

    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;

    while (m_clientConnection->isValid()) {
        try {
            // Read message from client
            iomgr_protocol::DatabaseEngineRequest requestMsg;
            LOG_DEBUG << m_logContext << "Waiting for request...";
            try {
                // NOTE: We can receive an empty message if TCP connection is closed or aborted
                net::epollWaitForData(m_clientEpollFd.getFD(), true);
                protobuf::readMessage(protobuf::ProtocolMessageType::kDatabaseEngineRequest,
                        requestMsg, *m_clientConnection, errorCodeChecker);
            } catch (net::ConnectionError& err) {
                LOG_DEBUG << m_logContext << "Client disconnected.";
                // Connection was closed or hangup. No reading operation was in progress
                closeConnection();
                return;
            } catch (std::exception& ex) {
                closeConnection();
                if (!utils::isExitEventSignaled()) LOG_ERROR << m_logContext << ex.what() << '.';
                return;
            }

            DBG_LOG_DEBUG(m_logContext << "Received request: id: " << requestMsg.request_id()
                                       << ",\ntext: " << requestMsg.text());

            dbengine::parser::SqlParser parser(requestMsg.text());
            try {
                parser.parse();
            } catch (std::exception& ex) {
                LOG_DEBUG << m_logContext << "Sending common parse error: " << ex.what();
                sendErrorReponse(requestMsg.request_id(), kSqlParseError, ex.what());
                LOG_DEBUG << m_logContext << "Sent common parse error.";
                continue;
            }

            const auto statementCount = parser.getStatementCount();
            for (std::size_t i = 0; i < statementCount; ++i) {
                // Fill request
                dbengine::requests::DBEngineRequestPtr dbEngineRequest;
                try {
#ifdef _DEBUG
                    LOG_DEBUG << [this, i, &parser]() {
                        std::ostringstream oss;
                        oss << m_logContext << "Statement #" << i << ":\n";
                        parser.dump(parser.findStatement(i), oss);
                        return oss.str();
                    }();
#endif
                    LOG_DEBUG << m_logContext << "Parsing statement #" << i;
                    dbengine::parser::DBEngineSqlRequestFactory factory(parser);
                    dbEngineRequest = factory.createSqlRequest(i);
                } catch (dbengine::parser::DBEngineRequestFactoryError& ex) {
                    LOG_ERROR << m_logContext << "SQL parse error: " << ex.what();
                    sendErrorReponse(requestMsg.request_id(), kSqlParseError, ex.what());
                    // Stop loop  after error response
                    break;
                } catch (std::exception& ex) {
                    const auto uuid = boost::uuids::random_generator()();
                    LOG_ERROR << m_logContext << "SQL parse error: internal error: '" << ex.what()
                              << "' (MSG_UUID " << uuid << ')';
                    const auto msg = "Internal error, see log for details, message UUID "
                                     + boost::uuids::to_string(uuid);
                    sendErrorReponse(requestMsg.request_id(), kSqlParseError, msg.c_str());
                    // Stop loop  after error response
                    break;
                }

                // Create IO Manager request
                LOG_DEBUG << m_logContext << "Scheduling statement #" << i << " for execution";
                const auto ioManagerRequest = std::make_shared<IOManagerRequest>(
                        requestMsg.request_id(), i, statementCount, shared_from_this(),
                        requestHandler, dbEngineRequest);

                // Execute IO Manager request
                auto future = ioManagerRequest->getFuture();
                m_requestDispatcher.addRequest(ioManagerRequest);

                // QUESTION: Should we wait here for some timeout ???
                LOG_DEBUG << m_logContext << "Waiting for statement #" << i << " to complete...";
                future.wait();

                // Check execution result, stop on error
                if (!future.get()) break;
            }  // for loop
        } catch (std::exception& ex) {
            LOG_ERROR << m_logContext << ex.what() << '.';
        }
    }  // while
}

dbengine::AuthenticationResult IOManagerSqlConnectionHandler::authenticateUser()
{
    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;
    iomgr_protocol::BeginAuthenticateUserRequest beginUserAuthenticationRequest;
    LOG_DEBUG << m_logContext << "Waiting for BeginAuthenticateUserRequest...";
    protobuf::readMessage(protobuf::ProtocolMessageType::kBeginAuthenticateUserRequest,
            beginUserAuthenticationRequest, *m_clientConnection, errorCodeChecker);

    LOG_DEBUG << m_logContext << "BeginAuthenticateUserRequest received";

    std::string userName;
    iomgr_protocol::BeginAuthenticateUserResponse beginAuthenticateUserResponse;
    try {
        m_requestDispatcher.getInstance().beginUserAuthentication(
                beginUserAuthenticationRequest.user_name());
        userName = beginUserAuthenticationRequest.user_name();
        beginAuthenticateUserResponse.set_session_started(true);
    } catch (dbengine::DatabaseError& dbError) {
        LOG_ERROR << m_logContext << '[' << dbError.getErrorCode() << "] " << dbError.what();
        beginAuthenticateUserResponse.set_session_started(false);
        auto msg = std::make_unique<siodb::StatusMessage>();
        msg->set_status_code(dbError.getErrorCode());
        msg->set_text(dbError.what());
        beginAuthenticateUserResponse.set_allocated_message(msg.release());
    }

    LOG_DEBUG << m_logContext << "Sending BeginAuthenticateUserResponse";
    protobuf::writeMessage(protobuf::ProtocolMessageType::kBeginAuthenticateUserResponse,
            beginAuthenticateUserResponse, *m_clientConnection);
    LOG_DEBUG << m_logContext << "Sent BeginAuthenticateUserResponse to client";

    if (!beginAuthenticateUserResponse.session_started())
        throw std::runtime_error("Session not started");

    iomgr_protocol::AuthenticateUserRequest authRequest;
    LOG_DEBUG << m_logContext << "Waiting for authentication request...";
    protobuf::readMessage(protobuf::ProtocolMessageType::kAuthenticateUserRequest, authRequest,
            *m_clientConnection, errorCodeChecker);

    LOG_DEBUG << m_logContext << "Client authentication request received";

    dbengine::AuthenticationResult authResult;
    iomgr_protocol::AuthenticateUserResponse authenticateUserResponse;
    try {
        authResult = m_requestDispatcher.getInstance().authenticateUser(
                userName, authRequest.signature(), authRequest.challenge());
        authenticateUserResponse.set_authenticated(true);
        authenticateUserResponse.set_session_id(boost::uuids::to_string(authResult.m_sessionUuid));
        authenticateUserResponse.add_server_info(m_requestDispatcher.getInstance().getName());
    } catch (dbengine::DatabaseError& dbError) {
        LOG_ERROR << m_logContext << '[' << dbError.getErrorCode() << "] " << dbError.what();
        authenticateUserResponse.set_authenticated(false);
        auto msg = std::make_unique<siodb::StatusMessage>();
        msg->set_status_code(dbError.getErrorCode());
        msg->set_text(dbError.what());
        authenticateUserResponse.set_allocated_message(msg.release());
    }

    LOG_DEBUG << m_logContext << "Sending AuthenticateUserResponse";
    protobuf::writeMessage(protobuf::ProtocolMessageType::kAuthenticateUserResponse,
            authenticateUserResponse, *m_clientConnection);
    LOG_DEBUG << m_logContext << "Sent AuthenticateUserResponse to client";

    if (authenticateUserResponse.authenticated()) return authResult;
    throw std::runtime_error("Access denied");
}

}  // namespace siodb::iomgr
