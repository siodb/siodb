// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerConnectionHandler.h"

// Project headers
#include "IOManagerRequest.h"
#include "../dbengine/handlers/RequestHandler.h"

// Common project headers
#include <siodb/common/io/FDStream.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/net/EpollHelpers.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>

// Protobuf message headers
#include <siodb/common/proto/IOManagerProtocol.pb.h>

// System headers
#include <sys/epoll.h>

namespace siodb::iomgr {

std::atomic<std::uint64_t> IOManagerConnectionHandler::s_idCounter(0);

IOManagerConnectionHandler::IOManagerConnectionHandler(
        IOManagerRequestDispatcher& requestDispatcher, FDGuard&& clientFd)
    : m_id(++s_idCounter)
    , m_logContext(createLogContextName(clientFd.getFD()))
    , m_requestDispatcher(requestDispatcher)
    , m_clientEpollFd(net::createEpollFd(clientFd.getFD(), EPOLLIN))
    , m_clientConnection(std::make_unique<siodb::io::FDStream>(clientFd.getFD(), false))
{
    dynamic_cast<siodb::io::FDStream*>(m_clientConnection.get())->setAutoClose();
    clientFd.release();

    // Start thread only after connection object is initialized
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

bool IOManagerConnectionHandler::executeIOManagerRequest(const IOManagerRequest& request)
{
    try {
        LOG_DEBUG << m_logContext << "Executing statement #" << request.getResponseId();
        request.execute();
        LOG_DEBUG << m_logContext << "Executed statement #" << request.getResponseId();
    } catch (std::exception& ex) {
        LOG_ERROR << m_logContext << "Request execution exception: " << ex.what() << '.';
        return false;
    }
    return true;
}

void IOManagerConnectionHandler::sendErrorReponse(int requestId, const char* text, int errorCode)
{
    iomgr_protocol::DatabaseEngineResponse response;
    response.set_request_id(requestId);
    const auto message = response.add_message();
    message->set_status_code(errorCode);
    message->set_text(text);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, *m_clientConnection);
}

void IOManagerConnectionHandler::closeConnection() noexcept
{
    LOG_DEBUG << m_logContext << "Closing connection";
    m_clientEpollFd.reset();
    m_clientConnection.reset();
}

void IOManagerConnectionHandler::threadMain()
{
    threadLogicImpl();
    closeConnection();
}

std::string IOManagerConnectionHandler::createLogContextName(int fd) const
{
    std::ostringstream oss;
    oss << kLogContextBase << '-' << m_id << '[' << fd << "]: ";
    return oss.str();
}

}  // namespace siodb::iomgr
