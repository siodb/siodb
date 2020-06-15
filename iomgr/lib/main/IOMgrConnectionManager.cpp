// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOMgrConnectionManager.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/net/ConnectionError.h>
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

IOMgrConnectionManager::IOMgrConnectionManager(int socketDomain,
        const config::ConstInstaceOptionsPtr& instanceOptions,
        const dbengine::InstancePtr& instance)
    : m_socketDomain(checkSocketDomain(socketDomain))
    , m_socketTypeName(socketDomain == AF_INET ? "IPv4" : "IPv6")
    , m_dbOptions(instanceOptions)
    , m_exitRequested(false)
    , m_instance(instance)
    // IMPORTANT: all next class members must be declared and initialized
    // exactly in this order and after all other members
    , m_workerThreadPool(
              createWorkerThreadPool(instanceOptions->m_ioManagerOptions.m_workerThreadNumber))
    , m_connectionListenerThread(&IOMgrConnectionManager::connectionListenerThreadMain, this)
    , m_deadConnectionRecyclerThread(&IOMgrConnectionManager::removeDeadConnections, this)
{
}

IOMgrConnectionManager::~IOMgrConnectionManager()
{
    // Indicate exit request
    m_exitRequested = true;

    // Stop connection listener thread
    if (m_connectionListenerThread.joinable()) {
        ::pthread_kill(m_connectionListenerThread.native_handle(), SIGUSR1);
        m_connectionListenerThread.join();
    }

    // Stop dead connection recycler thread
    if (m_deadConnectionRecyclerThread.joinable()) {
        ::pthread_kill(m_deadConnectionRecyclerThread.native_handle(), SIGUSR1);
        m_deadConnectionRecyclerThread.join();
    }
}

void IOMgrConnectionManager::connectionListenerThreadMain()
{
    // Set up server socket
    FdGuard server;
    std::string socketPath;
    int port = -1;
    try {
        port = m_socketDomain == AF_INET ? m_dbOptions->m_ioManagerOptions.m_ipv4port
                                         : m_dbOptions->m_ioManagerOptions.m_ipv6port;
        server.reset(net::createTcpServer(m_socketDomain, nullptr, port,
                m_dbOptions->m_generalOptions.m_userConnectionListenerBacklog));

        // Check socket
        if (!server.isValidFd()) {
            const int errorCode = errno;
            LOG_FATAL << m_socketTypeName << kLogContext << "Can't create " << m_socketTypeName
                      << " connection listener socket: " << std::strerror(errorCode) << '.';

            if (kill(::getpid(), SIGTERM) < 0) {
                LOG_ERROR << kLogContext << "Sending SIGTERM to IoMgr process failed: "
                          << std::strerror(errorCode);
            }
            return;
        }

        // Report successful opening of listener socket
        LOG_INFO << m_socketTypeName << kLogContext << "Listening for TCP connections via "
                 << (m_socketDomain == AF_INET ? "IPv4" : "IPv6") << " on the port " << port << '.';
    } catch (std::exception& ex) {
        LOG_ERROR << ex.what();
        if (kill(::getpid(), SIGTERM) < 0) {
            const int errorCode = errno;
            LOG_ERROR << kLogContext
                      << "Sending SIGTERM to IoMgr process failed: " << std::strerror(errorCode);
        }
        return;
    }

    while (!m_exitRequested) {
        if (server.getFd() == -2) return;

        // Accept connection
        FdGuard fdGuard(acceptTcpConnection(server.getFd()));

        // Validate connection file descriptor
        if (!fdGuard.isValidFd()) continue;

        m_connectionHandlers.emplace_back(std::move(fdGuard), m_instance);
    }
}

void IOMgrConnectionManager::deadConnectionRecyclerThreadMain()
{
    while (!m_exitRequested) {
        {
            std::unique_lock lock(m_connectionHandlersMutex);
            const auto waitResult = m_deadConnectionRecyclerThreadAwakeCondition.wait_for(
                    lock, kDeadConnectionsRecyclePeriod);
            if (waitResult != std::cv_status::timeout) continue;
        }
        removeDeadConnections();
    }
}

void IOMgrConnectionManager::removeDeadConnections()
{
    LOG_DEBUG << m_socketTypeName << kLogContext << "Recycling dead connections...";
    std::lock_guard lock(m_connectionHandlersMutex);

    LOG_DEBUG << m_socketTypeName << kLogContext
              << "Number of connections before recycling: " << m_connectionHandlers.size();

    auto connectionHandlerIt = m_connectionHandlers.begin();
    while (connectionHandlerIt != m_connectionHandlers.end() && !m_exitRequested) {
        if (!connectionHandlerIt->isConnected()) {
            m_connectionHandlers.erase(connectionHandlerIt++);
        } else {
            ++connectionHandlerIt;
        }
    }

    LOG_DEBUG << m_socketTypeName << kLogContext
              << "Number of connections after recycling: " << m_connectionHandlers.size();
}

int IOMgrConnectionManager::acceptTcpConnection(int serverFd)
{
    union {
        struct sockaddr_in v4;
        struct sockaddr_in6 v6;
    } addr;

    socklen_t addrLength = m_socketDomain == AF_INET ? sizeof(addr.v4) : sizeof(addr.v6);

    // Note that last parameter of the accept4() is zero, so we intentionally want
    // resulting file descriptor to be inherited by child process.
    FdGuard client(::accept4(serverFd, reinterpret_cast<sockaddr*>(&addr), &addrLength, 0));

    if (!client.isValidFd()) {
        const int errorCode = errno;
        if (errorCode == EINTR && m_exitRequested) {
            LOG_INFO << m_socketTypeName << kLogContext
                     << "TCP connection listener thread is exiting"
                     << " because database is shutting down.";
        } else {
            LOG_ERROR << m_socketTypeName << kLogContext
                      << "Can't accept user client connection: " << std::strerror(errorCode) << '.';
        }
        return -1;
    }

    char addrBuffer[std::max(INET6_ADDRSTRLEN, INET_ADDRSTRLEN) + 1];
    *addrBuffer = '\0';
    inet_ntop(m_socketDomain, &addr, addrBuffer, addrLength);
    LOG_INFO << m_socketTypeName << kLogContext << "Accepted new user connection from "
             << addrBuffer << '.';
    return client.release();
}

int IOMgrConnectionManager::checkSocketDomain(int socketDomain)
{
    switch (socketDomain) {
        case AF_INET:
        case AF_INET6: return socketDomain;
        default: {
            throw std::invalid_argument(
                    "Invalid connection listener socket domain,"
                    " only IPv4 and IPv6 sockets are supported");
        }
    }
}

std::vector<std::unique_ptr<UniversalWorker>> IOMgrConnectionManager::createWorkerThreadPool(
        std::size_t size)
{
    std::vector<std::unique_ptr<UniversalWorker>> pool;
    pool.reserve(size);
    for (std::size_t id = 0; id < size; ++id)
        pool.push_back(std::make_unique<UniversalWorker>(id));
    return pool;
}

}  // namespace siodb::iomgr
