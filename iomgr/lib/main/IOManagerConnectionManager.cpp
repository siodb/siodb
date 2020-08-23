// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerConnectionManager.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/net/ConnectionError.h>
#include <siodb/common/net/SocketDomain.h>
#include <siodb/common/net/TcpServer.h>
#include <siodb/common/utils/Debug.h>
#include <siodb/common/utils/FdGuard.h>

// System headers
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>

namespace siodb::iomgr {

IOManagerConnectionManager::IOManagerConnectionManager(const char* name, int socketDomain, int port,
        unsigned connectionListenerBacklog, unsigned deadConnectionCleanupInterval,
        IOManagerRequestDispatcher& requestDispatcher,
        IOManagerConnectionHandlerFactory& connectionHandlerFactory)
    : m_socketDomain(net::checkSocketDomain(socketDomain))
    , m_logContext(createLogContextName(name))
    , m_port(port)
    , m_userConnectionListenerBacklog(connectionListenerBacklog)
    , m_deadConnectionCleanupInterval(deadConnectionCleanupInterval)
    , m_requestDispatcher(requestDispatcher)
    , m_connectionHandlerFactory(connectionHandlerFactory)
    , m_exitRequested(false)
    // IMPORTANT: all next class members must be declared and initialized
    // exactly in this order and after all other members
    , m_connectionListenerThread(&IOManagerConnectionManager::connectionListenerThreadMain, this)
    , m_deadConnectionCleanupThread(&IOManagerConnectionManager::removeDeadConnections, this)
{
}

IOManagerConnectionManager::~IOManagerConnectionManager()
{
    // Indicate exit request
    m_exitRequested = true;

    // Stop connection listener thread
    if (m_connectionListenerThread.joinable()) {
        ::pthread_kill(m_connectionListenerThread.native_handle(), SIGUSR1);
        m_connectionListenerThread.join();
    }

    // Stop dead connection recycler thread
    if (m_deadConnectionCleanupThread.joinable()) {
        ::pthread_kill(m_deadConnectionCleanupThread.native_handle(), SIGUSR1);
        m_deadConnectionCleanupThread.join();
    }
}

void IOManagerConnectionManager::connectionListenerThreadMain()
{
    // Set up server socket
    FdGuard server;
    std::string socketPath;
    try {
        server.reset(net::createTcpServer(
                m_socketDomain, nullptr, m_port, m_userConnectionListenerBacklog));

        // Check socket
        if (!server.isValidFd()) {
            const int errorCode = errno;
            LOG_FATAL << m_logContext << "Can't create " << net::getSocketDomainName(m_socketDomain)
                      << " connection listener socket: " << std::strerror(errorCode) << '.';

            if (::kill(::getpid(), SIGTERM) < 0) {
                LOG_ERROR << m_logContext << "Sending SIGTERM to IoMgr process failed: "
                          << std::strerror(errorCode);
            }
            return;
        }

        // Report successful opening of listener socket
        LOG_INFO << m_logContext << "Listening for TCP connections via "
                 << (m_socketDomain == AF_INET ? "IPv4" : "IPv6") << " on the port " << m_port
                 << '.';
    } catch (std::exception& ex) {
        LOG_ERROR << ex.what();
        if (::kill(::getpid(), SIGTERM) < 0) {
            const int errorCode = errno;
            LOG_ERROR << m_logContext
                      << "Sending SIGTERM to IoMgr process failed: " << std::strerror(errorCode);
        }
        return;
    }

    while (!m_exitRequested) {
        if (server.getFD() == -2) return;

        FdGuard fdGuard(acceptTcpConnection(server.getFD()));
        if (!fdGuard.isValidFd()) continue;

        std::unique_ptr<IOManagerConnectionHandler> handler(
                m_connectionHandlerFactory.createConnectionHandler(
                        m_requestDispatcher, std::move(fdGuard)));
        m_connectionHandlers.push_back(
                std::shared_ptr<IOManagerConnectionHandler>(std::move(handler)));
    }
}

void IOManagerConnectionManager::deadConnectionCleanupThreadMain()
{
    while (!m_exitRequested) {
        {
            std::unique_lock lock(m_connectionHandlersMutex);
            const auto waitResult = m_deadConnectionCleanupThreadAwakeCondition.wait_for(
                    lock, m_deadConnectionCleanupInterval);
            if (waitResult != std::cv_status::timeout) continue;
        }
        removeDeadConnections();
    }
}

void IOManagerConnectionManager::removeDeadConnections()
{
    LOG_DEBUG << m_logContext << "Cleaning up dead connections...";
    std::lock_guard lock(m_connectionHandlersMutex);

    LOG_DEBUG << m_logContext
              << "Number of connections before cleanup: " << m_connectionHandlers.size();

    auto it = m_connectionHandlers.begin();
    while (it != m_connectionHandlers.end() && !m_exitRequested) {
        if (!(*it)->isConnected())
            m_connectionHandlers.erase(it++);
        else
            ++it;
    }

    LOG_DEBUG << m_logContext
              << "Number of connections after cleanup: " << m_connectionHandlers.size();
}

int IOManagerConnectionManager::acceptTcpConnection(int serverFd)
{
    union {
        struct sockaddr_in v4;
        struct sockaddr_in6 v6;
    } addr;

    socklen_t addrLength = m_socketDomain == AF_INET ? sizeof(addr.v4) : sizeof(addr.v6);

    FdGuard client(
            ::accept4(serverFd, reinterpret_cast<sockaddr*>(&addr), &addrLength, SOCK_CLOEXEC));
    if (!client.isValidFd()) {
        const int errorCode = errno;
        if (errorCode == EINTR && m_exitRequested) {
            LOG_INFO << m_logContext << "TCP connection listener thread is exiting"
                     << " because database is shutting down.";
        } else {
            LOG_ERROR << m_logContext << "Can't accept TCP connection: " << std::strerror(errorCode)
                      << '.';
        }
        return -1;
    }

    char addrStr[std::max(INET6_ADDRSTRLEN, INET_ADDRSTRLEN) + 1];
    *addrStr = '\0';
    inet_ntop(m_socketDomain, &addr, addrStr, addrLength);
    LOG_INFO << m_logContext << "Accepted new TCP connection from " << addrStr << '.';

    return client.release();
}

std::string IOManagerConnectionManager::createLogContextName(const char* name) const
{
    std::ostringstream oss;
    oss << net::getSocketDomainName(m_socketDomain) << '-' << name << ": ";
    return oss.str();
}

}  // namespace siodb::iomgr
