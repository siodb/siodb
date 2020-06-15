// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOMgrConnectionHandler.h"

// Project headers
#include "../dbengine/SessionGuard.h"
#include "../dbengine/handlers/RequestHandler.h"
#include "../dbengine/parser/DBEngineRequestFactory.h"
#include "../dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/common/io/FdIo.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/net/ConnectionError.h>
#include <siodb/common/net/EpollHelpers.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/utils/ErrorCodeChecker.h>
#include <siodb/common/utils/SignalHandlers.h>

// Protobuf message headers
#include <siodb/common/proto/IOManagerProtocol.pb.h>

// System headers
#include <sys/epoll.h>

namespace siodb::iomgr {

IOMgrConnectionHandler::IOMgrConnectionHandler(
        FdGuard&& clientFd, const dbengine::InstancePtr& instance)
    : m_instance(instance)
{
    auto clientIo = std::make_unique<siodb::io::FdIo>(clientFd.getFd(), false);
    m_clientEpollFd.reset(net::createEpollFd(clientFd.getFd(), EPOLLIN));
    clientFd.release();
    clientIo->setAutoClose(true);
    m_clientIo = std::move(clientIo);

    // Start thread only after IO is initialized
    m_thread = std::make_unique<std::thread>(&IOMgrConnectionHandler::threadMain, this);
}

IOMgrConnectionHandler::~IOMgrConnectionHandler()
{
    closeConnection();

    if (m_thread->joinable()) {
        ::pthread_kill(m_thread->native_handle(), SIGUSR1);
        m_thread->join();
    }
}

void IOMgrConnectionHandler::closeConnection() noexcept
{
    LOG_DEBUG << kLogContext << "Closing connection";
    m_clientEpollFd.reset();
    m_clientIo.reset();
}

void IOMgrConnectionHandler::respondToServerWithError(int requestId, const char* text, int errCode)
{
    iomgr_protocol::DatabaseEngineResponse response;
    response.set_request_id(requestId);
    const auto message = response.add_message();
    message->set_status_code(errCode);
    message->set_text(text);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, *m_clientIo);
}

void IOMgrConnectionHandler::beginUserAuthentication()
{
    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;
    iomgr_protocol::BeginAuthenticateUserRequest beginUserAuthenticationRequest;
    LOG_DEBUG << kLogContext << "Waiting for BeginAuthenticateUserRequest...";
    protobuf::readMessage(protobuf::ProtocolMessageType::kBeginAuthenticateUserRequest,
            beginUserAuthenticationRequest, *m_clientIo, errorCodeChecker);

    LOG_DEBUG << kLogContext << "BeginAuthenticateUserRequest received";

    iomgr_protocol::BeginAuthenticateUserResponse beginAuthenticateUserResponse;
    try {
        m_instance->beginUserAuthentication(beginUserAuthenticationRequest.user_name());
        m_userName = beginUserAuthenticationRequest.user_name();
        beginAuthenticateUserResponse.set_session_started(true);
    } catch (dbengine::DatabaseError& dbError) {
        LOG_ERROR << kLogContext << '[' << dbError.getErrorCode() << "] " << dbError.what();
        beginAuthenticateUserResponse.set_session_started(false);
        siodb::StatusMessage* msg = new siodb::StatusMessage();
        msg->set_status_code(dbError.getErrorCode());
        msg->set_text(dbError.what());
        beginAuthenticateUserResponse.set_allocated_message(msg);
    }

    LOG_DEBUG << kLogContext << "Sending BeginAuthenticateUserResponse";
    protobuf::writeMessage(protobuf::ProtocolMessageType::kBeginAuthenticateUserResponse,
            beginAuthenticateUserResponse, *m_clientIo);
    LOG_DEBUG << kLogContext << "Sent BeginAuthenticateUserResponse to client";

    if (!beginAuthenticateUserResponse.session_started())
        throw std::runtime_error("Session isn't started");
}

std::pair<std::uint32_t, Uuid> IOMgrConnectionHandler::authenticateUser()
{
    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;
    iomgr_protocol::AuthenticateUserRequest authRequest;
    LOG_DEBUG << kLogContext << "Waiting for authentication request...";
    protobuf::readMessage(protobuf::ProtocolMessageType::kAuthenticateUserRequest, authRequest,
            *m_clientIo, errorCodeChecker);

    LOG_DEBUG << kLogContext << "Client authentication request received";

    iomgr_protocol::AuthenticateUserResponse authenticateUserResponse;
    std::pair<std::uint32_t, Uuid> authPair;
    try {
        authPair = m_instance->authenticateUser(
                m_userName, authRequest.signature(), authRequest.challenge());
        authenticateUserResponse.set_authenticated(true);
        authenticateUserResponse.set_session_id(boost::uuids::to_string(authPair.second));
    } catch (dbengine::DatabaseError& dbError) {
        LOG_ERROR << kLogContext << '[' << dbError.getErrorCode() << "] " << dbError.what();
        authenticateUserResponse.set_authenticated(false);
        siodb::StatusMessage* msg = new siodb::StatusMessage();
        msg->set_status_code(dbError.getErrorCode());
        msg->set_text(dbError.what());
        authenticateUserResponse.set_allocated_message(msg);
    }

    LOG_DEBUG << kLogContext << "Sending AuthenticateUserResponse";
    protobuf::writeMessage(protobuf::ProtocolMessageType::kAuthenticateUserResponse,
            authenticateUserResponse, *m_clientIo);
    LOG_DEBUG << kLogContext << "Sent AuthenticateUserResponse to client";

    if (!authenticateUserResponse.authenticated()) throw std::runtime_error("User access denied");

    return authPair;
}

void IOMgrConnectionHandler::threadMain()
{
    std::pair<std::uint32_t, Uuid> authPair;
    try {
        beginUserAuthentication();
        authPair = authenticateUser();
    } catch (std::exception& e) {
        closeConnection();
        return;
    }

    dbengine::SessionGuard guard(m_instance, authPair.second);
    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;
    dbengine::RequestHandler requestHandler(*m_instance, *m_clientIo, authPair.first);
    while (m_clientIo->isValid()) {
        try {
            // Read message from client
            iomgr_protocol::DatabaseEngineRequest request;
            LOG_DEBUG << kLogContext << "Waiting for request...";
            try {
                // NOTE: In case of the TCP connection close or abort,
                // we can receive an empty message
                net::epollWaitForData(m_clientEpollFd.getFd(), true);
                protobuf::readMessage(protobuf::ProtocolMessageType::kDatabaseEngineRequest,
                        request, *m_clientIo, errorCodeChecker);
            } catch (net::ConnectionError& err) {
                LOG_DEBUG << kLogContext << "Client disconnected.";
                // Connection was closed or hangup. No reading operation was in progress
                closeConnection();
                return;
            } catch (std::exception& ex) {
                closeConnection();
                if (!utils::isExitEventSignaled()) LOG_ERROR << kLogContext << ex.what() << '.';
                return;
            }

            LOG_DEBUG << kLogContext << "Received request: id: " << request.request_id()
                      << ", text: " << request.text();

            dbengine::parser::SqlParser parser(request.text());
            try {
                parser.parse();
            } catch (std::exception& ex) {
                LOG_DEBUG << kLogContext << "Sending common parse error: " << ex.what();
                respondToServerWithError(request.request_id(), ex.what(), kSqlParseError);
                LOG_DEBUG << kLogContext << "Sent common parse error.";
                continue;
            }

            // For now just dump each statement
            const auto statementCount = parser.getStatementCount();
            for (std::size_t i = 0; i < statementCount; ++i) {
                // Fill request
                dbengine::requests::DBEngineRequestPtr dbeRequest;
                try {
                    LOG_DEBUG << [i, &parser]() {
                        std::ostringstream oss;
                        oss << kLogContext << "Statement #" << i << ":\n";
                        parser.dump(parser.findStatement(i), oss);
                        return oss.str();
                    }();
                    LOG_DEBUG << kLogContext << "Parsing statement #" << i;
                    dbeRequest = dbengine::parser::DBEngineRequestFactory::createRequest(
                            parser.findStatement(i));
                } catch (std::exception& ex) {
                    LOG_DEBUG << kLogContext << "Sending request parse error " << ex.what();
                    respondToServerWithError(request.request_id(), ex.what(), kSqlParseError);
                    LOG_DEBUG << kLogContext << "Sent request parse error";
                    // Stop loop  after response with an error
                    break;
                }

                // Execute request
                try {
                    LOG_DEBUG << kLogContext << "Executing statement #" << i;
                    requestHandler.executeRequest(
                            *dbeRequest, request.request_id(), i, statementCount);
                } catch (std::exception& ex) {
                    LOG_ERROR << kLogContext << "Request execution exception: " << ex.what() << '.';
                    respondToServerWithError(request.request_id(), ex.what(), kInternalError);
                    // Stop loop  after response with an error
                    break;
                }
            }
        } catch (std::exception& ex) {
            LOG_ERROR << kLogContext << ex.what() << '.';
        }
    }
}

}  // namespace siodb::iomgr
