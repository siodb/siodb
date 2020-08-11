// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerConnectionHandler.h"

// Project headers
#include "IOManagerRequest.h"
#include "../dbengine/SessionGuard.h"
#include "../dbengine/handlers/RequestHandler.h"
#include "../dbengine/parser/DBEngineRequestFactory.h"
#include "../dbengine/parser/SqlParser.h"

// Common project headers
#include <siodb/common/io/FdDevice.h>
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

std::atomic<std::uint64_t> IOManagerConnectionHandler::s_idCounter(0);

IOManagerConnectionHandler::IOManagerConnectionHandler(
        IOManagerRequestDispatcher& requestDispatcher, FdGuard&& clientFd)
    : m_id(++s_idCounter)
    , m_logContext(createLogContextName(clientFd.getFd()))
    , m_requestDispatcher(requestDispatcher)
{
    auto clientIo = std::make_unique<siodb::io::FdDevice>(clientFd.getFd(), false);
    m_clientEpollFd.reset(net::createEpollFd(clientFd.getFd(), EPOLLIN));
    clientFd.release();
    clientIo->setAutoClose(true);
    m_clientIo = std::move(clientIo);

    // Start thread only after IO is initialized
    m_thread = std::make_unique<std::thread>(&IOManagerConnectionHandler::threadMain, this);
}

IOManagerConnectionHandler::~IOManagerConnectionHandler()
{
    closeConnection();
    if (m_thread->joinable()) {
        ::pthread_kill(m_thread->native_handle(), SIGUSR1);
        m_thread->join();
    }
}

void IOManagerConnectionHandler::threadMain()
{
    try {
        authenticateUser();
    } catch (std::exception& e) {
        closeConnection();
        return;
    }

    dbengine::SessionGuard guard(m_requestDispatcher.getInstance(), m_authResult.m_sessionUuid);
    m_requestHandler = std::make_shared<dbengine::RequestHandler>(
            m_requestDispatcher.getInstance(), *m_clientIo, m_authResult.m_userId);
    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;
    while (m_clientIo->isValid()) {
        try {
            // Read message from client
            iomgr_protocol::DatabaseEngineRequest requestMsg;
            LOG_DEBUG << m_logContext << "Waiting for request...";
            try {
                // NOTE: In case of the TCP connection close or abort,
                // we can receive an empty message
                net::epollWaitForData(m_clientEpollFd.getFd(), true);
                protobuf::readMessage(protobuf::ProtocolMessageType::kDatabaseEngineRequest,
                        requestMsg, *m_clientIo, errorCodeChecker);
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
                                       << ", text: " << requestMsg.text());

            dbengine::parser::SqlParser parser(requestMsg.text());
            try {
                parser.parse();
            } catch (std::exception& ex) {
                LOG_DEBUG << m_logContext << "Sending common parse error: " << ex.what();
                sendErrorReponse(requestMsg.request_id(), ex.what(), kSqlParseError);
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
                    dbEngineRequest = dbengine::parser::DBEngineRequestFactory::createRequest(
                            parser.findStatement(i));
                } catch (std::exception& ex) {
                    LOG_DEBUG << m_logContext << "Sending request parse error " << ex.what();
                    sendErrorReponse(requestMsg.request_id(), ex.what(), kSqlParseError);
                    LOG_DEBUG << m_logContext << "Sent request parse error";
                    // Stop loop  after error response
                    break;
                }

                // Create IO Manager request
                LOG_DEBUG << m_logContext << "Scheduling statement #" << i << " for execution";
                const auto ioManagerRequest =
                        std::make_shared<IOManagerRequest>(requestMsg.request_id(), i,
                                statementCount, shared_from_this(), dbEngineRequest);

                // Execute IO Manager request
                auto future = ioManagerRequest->getFuture();
                m_requestDispatcher.addRequest(ioManagerRequest);

                // QUESTION: Should we wait here for some timeout ???
                LOG_DEBUG << m_logContext << "Waiting statement #" << i << " to complete execution";
                future.wait();

                // Check execution result, stop on error
                if (!future.get()) break;
            }  // for loop
        } catch (std::exception& ex) {
            LOG_ERROR << m_logContext << ex.what() << '.';
        }
    }
}

bool IOManagerConnectionHandler::executeIOManagerRequest(const IOManagerRequest& request)
{
    try {
        LOG_DEBUG << m_logContext << "Executing statement #" << request.getResponseId();
        m_requestHandler->executeRequest(request.getDBEngineRequest(), request.getRequestId(),
                request.getResponseId(), request.getStatementCount());
    } catch (std::exception& ex) {
        LOG_ERROR << m_logContext << "Request execution exception: " << ex.what() << '.';
        sendErrorReponse(request.getRequestId(), ex.what(), kInternalError);
        return false;
    }
    return true;
}

void IOManagerConnectionHandler::closeConnection() noexcept
{
    LOG_DEBUG << m_logContext << "Closing connection";
    m_clientEpollFd.reset();
    m_clientIo.reset();
}

void IOManagerConnectionHandler::authenticateUser()
{
    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;
    iomgr_protocol::BeginAuthenticateUserRequest beginUserAuthenticationRequest;
    LOG_DEBUG << m_logContext << "Waiting for BeginAuthenticateUserRequest...";
    protobuf::readMessage(protobuf::ProtocolMessageType::kBeginAuthenticateUserRequest,
            beginUserAuthenticationRequest, *m_clientIo, errorCodeChecker);

    LOG_DEBUG << m_logContext << "BeginAuthenticateUserRequest received";

    iomgr_protocol::BeginAuthenticateUserResponse beginAuthenticateUserResponse;
    try {
        m_requestDispatcher.getInstance().beginUserAuthentication(
                beginUserAuthenticationRequest.user_name());
        m_userName = beginUserAuthenticationRequest.user_name();
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
            beginAuthenticateUserResponse, *m_clientIo);
    LOG_DEBUG << m_logContext << "Sent BeginAuthenticateUserResponse to client";

    if (!beginAuthenticateUserResponse.session_started())
        throw std::runtime_error("Session isn't started");

    iomgr_protocol::AuthenticateUserRequest authRequest;
    LOG_DEBUG << m_logContext << "Waiting for authentication request...";
    protobuf::readMessage(protobuf::ProtocolMessageType::kAuthenticateUserRequest, authRequest,
            *m_clientIo, errorCodeChecker);

    LOG_DEBUG << m_logContext << "Client authentication request received";

    iomgr_protocol::AuthenticateUserResponse authenticateUserResponse;
    try {
        m_authResult = m_requestDispatcher.getInstance().authenticateUser(
                m_userName, authRequest.signature(), authRequest.challenge());
        authenticateUserResponse.set_authenticated(true);
        authenticateUserResponse.set_session_id(
                boost::uuids::to_string(m_authResult.m_sessionUuid));
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
            authenticateUserResponse, *m_clientIo);
    LOG_DEBUG << m_logContext << "Sent AuthenticateUserResponse to client";

    if (!authenticateUserResponse.authenticated()) throw std::runtime_error("Access denied");
}

void IOManagerConnectionHandler::sendErrorReponse(int requestId, const char* text, int errorCode)
{
    iomgr_protocol::DatabaseEngineResponse response;
    response.set_request_id(requestId);
    const auto message = response.add_message();
    message->set_status_code(errorCode);
    message->set_text(text);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, *m_clientIo);
}

std::string IOManagerConnectionHandler::createLogContextName(int fd) const
{
    std::ostringstream oss;
    oss << kLogContextBase << '-' << m_id << '[' << fd << "]: ";
    return oss.str();
}

}  // namespace siodb::iomgr
