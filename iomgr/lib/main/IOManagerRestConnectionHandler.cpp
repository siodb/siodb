// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerRestConnectionHandler.h"

// Project headers
#include "IOManagerRequest.h"
#include "../dbengine/handlers/RequestHandler.h"
#include "../dbengine/parser/DBEngineRequestFactoryError.h"
#include "../dbengine/parser/DBEngineRestRequestFactory.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/net/ConnectionError.h>
#include <siodb/common/net/EpollHelpers.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/utils/ErrorCodeChecker.h>
#include <siodb/common/utils/SignalHandlers.h>

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>

namespace siodb::iomgr {

// ----- internals -----

void IOManagerRestConnectionHandler::threadLogicImpl()
{
    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;

    while (m_clientConnection->isValid()) {
        try {
            // Read message from client
            iomgr_protocol::DatabaseEngineRestRequest requestMsg;
            LOG_DEBUG << m_logContext << "Waiting for request...";
            try {
                // NOTE: We can receive an empty message if TCP connection is closed or aborted
                net::epollWaitForData(m_clientEpollFd.getFD(), true);
                protobuf::readMessage(protobuf::ProtocolMessageType::kDatabaseEngineRestRequest,
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

            DBG_LOG_DEBUG(m_logContext
                          << "Received request: id: " << requestMsg.request_id()
                          << ", verb: " << static_cast<int>(requestMsg.verb()) << ", object_type "
                          << static_cast<int>(requestMsg.object_type()) << ", object_id: "
                          << requestMsg.object_id() << ", object_name: " << requestMsg.object_name()
                          << ", user: " << requestMsg.user_name());

            // Authenticate user with token
            std::uint32_t userId;
            try {
                const auto userName = boost::to_upper_copy(requestMsg.user_name());
                userId = m_requestDispatcher.getInstance().authenticateUser(
                        userName, requestMsg.token());
            } catch (dbengine::UserVisibleDatabaseError& ex) {
                LOG_ERROR << "Authentication error: " << '[' << ex.getErrorCode() << "] "
                          << ex.what();
                LOG_DEBUG << m_logContext << "Sending authentication error";
                sendErrorReponse(requestMsg.request_id(), ex.what(), ex.getErrorCode());
                LOG_DEBUG << m_logContext << "Sent request parse error";
                continue;
            } catch (dbengine::DatabaseError& ex) {
                LOG_DEBUG << m_logContext << "Sending authentication error";
                const auto uuid = boost::uuids::random_generator()();
                LOG_ERROR << m_logContext << '[' << ex.getErrorCode() << "] " << ex.what()
                          << " (MSG_UUID " << uuid << ')';
                auto msg = "Internal error, see log for details, message UUID "
                           + boost::uuids::to_string(uuid);
                sendErrorReponse(requestMsg.request_id(), ex.what(), kRestAuthenticationError);
                LOG_DEBUG << m_logContext << "Sent authentication error";
                continue;
            } catch (std::exception& ex) {
                LOG_DEBUG << m_logContext << "Sending authentication error";
                const auto uuid = boost::uuids::random_generator()();
                LOG_ERROR << m_logContext << ex.what() << " (MSG_UUID " << uuid << ')';
                auto msg = "Internal error, see log for details, message UUID "
                           + boost::uuids::to_string(uuid);
                sendErrorReponse(requestMsg.request_id(), ex.what(), kRestAuthenticationError);
                LOG_DEBUG << m_logContext << "Sent authentication error";
                continue;
            }

            // Create request handler
            const auto requestHandler = std::make_shared<dbengine::RequestHandler>(
                    m_requestDispatcher.getInstance(), *m_clientConnection, userId);

            // Parse incoming request
            dbengine::requests::DBEngineRequestPtr dbEngineRequest;
            try {
                dbEngineRequest = dbengine::parser::DBEngineRestRequestFactory::createRestRequest(
                        requestMsg, m_clientConnection.get());
            } catch (dbengine::parser::DBEngineRequestFactoryError& ex) {
                LOG_DEBUG << m_logContext << "Sending request parsing error " << ex.what();
                sendErrorReponse(requestMsg.request_id(), ex.what(), kRestParseError);
                LOG_DEBUG << m_logContext << "Sent request parse error";
                continue;
            }

            // Create IO Manager request
            LOG_DEBUG << m_logContext << "Scheduling REST request for execution";
            const auto ioManagerRequest =
                    std::make_shared<IOManagerRequest>(requestMsg.request_id(), 0U, 1U,
                            shared_from_this(), requestHandler, dbEngineRequest);

            // Execute IO Manager request
            auto future = ioManagerRequest->getFuture();
            m_requestDispatcher.addRequest(ioManagerRequest);

            // QUESTION: Should we wait here for some timeout ???
            LOG_DEBUG << m_logContext << "Waiting for REST request to complete...";
            future.wait();

            // Check execution result, stop on error
            if (!future.get()) break;

        } catch (std::exception& ex) {
            LOG_ERROR << m_logContext << ex.what() << '.';
        }
    }  // while
}

}  // namespace siodb::iomgr
