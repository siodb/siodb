// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SiodbConnectionManager.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/net/SocketDomain.h>
#include <siodb/common/net/TcpServer.h>
#include <siodb/common/net/UnixServer.h>
#include <siodb/common/options/SiodbInstance.h>
#include <siodb/common/stl_ext/utility_ext.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/CheckOSUser.h>
#include <siodb/common/utils/Debug.h>
#include <siodb/common/utils/FDGuard.h>

// STL headers
#include <chrono>
#include <sstream>

// System headers
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace siodb {

SiodbConnectionManager::SiodbConnectionManager(
        int socketDomain, bool checkUser, const config::ConstInstaceOptionsPtr& instanceOptions)
    : m_socketDomain(net::checkSocketDomain(socketDomain))
    , m_logContext(createLogContextName())
    , m_checkUser(checkUser)
    , m_dbOptions(instanceOptions)
    , m_workerExecutablePath(instanceOptions->getExecutableDir() + fs::path::preferred_separator
                             + kUserConnectionWorkerExecutable)
    , m_exitRequested(false)
    , m_deadConnectionCleanupThreadAwakeCondition()
    // IMPORTANT: all next class members must be declared and initialized
    // exactly in this order and after all other members
    , m_deadConnectionCleanupThread(&SiodbConnectionManager::deadConnectionCleanupThreadMain, this)
    , m_connectionListenerThread(&SiodbConnectionManager::connectionListenerThreadMain, this)
{
}

SiodbConnectionManager::~SiodbConnectionManager()
{
    // Indicate exit request
    m_exitRequested = true;

    // Stop connection listener thread
    if (m_connectionListenerThread.joinable()) {
        ::pthread_kill(m_connectionListenerThread.native_handle(), SIGUSR1);
        m_connectionListenerThread.join();
    }

    // Signal dead connection recycler thread and wait for it to finish
    {
        std::lock_guard lock(m_connectionHandlersMutex);
        m_deadConnectionCleanupThreadAwakeCondition.notify_one();
    }
    m_deadConnectionCleanupThread.join();

    // Stop remaining child processes
    if (!m_connectionHandlers.empty()) {
        LOG_INFO << m_logContext << "Shutting down active connection handlers...";
        for (auto pid : m_connectionHandlers) {
            LOG_INFO << m_logContext << "Sending interrupt signal to PID " << pid;
            ::kill(pid, SIGTERM);
        }

        LOG_INFO << m_logContext << "Waiting for connection handler processes to shut down...";
        removeDeadConnections(true);
        const auto startTime = std::chrono::steady_clock::now();
        while (!m_connectionHandlers.empty()
                && std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - startTime)
                                   .count()
                           < kUserConnectionWorkerShutdownTimeoutMs) {
            removeDeadConnections(true);
            std::this_thread::sleep_for(kTerminateConnectionsCheckPeriod);
        }

        if (!m_connectionHandlers.empty()) {
            LOG_INFO << m_logContext << "Killing remaining active connection handlers...";
            for (auto pid : m_connectionHandlers) {
                LOG_INFO << m_logContext << "Sending kill signal to PID " << pid;
                ::kill(pid, SIGKILL);
            }
            removeDeadConnections(true);
            while (!m_connectionHandlers.empty()) {
                removeDeadConnections(true);
                std::this_thread::sleep_for(kTerminateConnectionsCheckPeriod);
            }
        }
        LOG_INFO << m_logContext << "All connection handler processes finished.";
    }
}

void SiodbConnectionManager::connectionListenerThreadMain()
{
    // Set up server socket
    FDGuard server;
    std::string socketPath;
    int port = -1;
    try {
        if (m_socketDomain == AF_UNIX) {
            socketPath = composeInstanceSocketPath(m_dbOptions->m_generalOptions.m_name);
            server.reset(net::createUnixServer(socketPath,
                    m_dbOptions->m_generalOptions.m_adminConnectionListenerBacklog, true));
        } else {
            port = m_socketDomain == AF_INET ? m_dbOptions->m_generalOptions.m_ipv4Port
                                             : m_dbOptions->m_generalOptions.m_ipv6Port;
            server.reset(net::createTcpServer(m_socketDomain, nullptr, port,
                    m_dbOptions->m_generalOptions.m_userConnectionListenerBacklog));
        }

        // Check socket
        if (!server.isValidFd()) {
            const int errorCode = errno;
            LOG_FATAL << m_logContext << "Can't create " << net::getSocketDomainName(m_socketDomain)
                      << " connection listener socket: " << std::strerror(errorCode) << '.';
            if (::kill(::getpid(), SIGTERM) < 0) {
                LOG_ERROR << m_logContext << "Sending SIGTERM to Siodb process failed: "
                          << std::strerror(errorCode);
            }
            return;
        }

        // Report successful opening of listener socket
        if (m_socketDomain == AF_UNIX) {
            LOG_INFO << m_logContext << "Listening for UNIX connections on the " << socketPath
                     << '.';
        } else {
            LOG_INFO << m_logContext << "Listening for TCP connections via "
                     << (m_socketDomain == AF_INET ? "IPv4" : "IPv6") << " on the port " << port
                     << '.';
        }
    } catch (std::exception& ex) {
        LOG_ERROR << ex.what();
        if (::kill(::getpid(), SIGTERM) < 0) {
            const int errorCode = errno;
            LOG_ERROR << m_logContext << "Sending SIGTERM to Siodb process failed: [" << errorCode
                      << "] " << std::strerror(errorCode);
        }
        return;
    }

    while (!m_exitRequested) {
        try {
            // Accept connection
            FDGuard client(m_socketDomain == AF_UNIX ? acceptUnixConnection(server.getFD())
                                                     : acceptTcpConnection(server.getFD()));
            if (!client.isValidFd()) continue;

            // Prepare user connection worker command-line parameters
            std::vector<std::string> args;
            args.reserve(10);
            args.push_back(m_workerExecutablePath);
            args.push_back("--instance");
            args.push_back(m_dbOptions->m_generalOptions.m_name);
            args.push_back("--client-fd");
            args.push_back(std::to_string(client.getFD()));
            if (m_checkUser && m_socketDomain == AF_UNIX) {
                args.push_back("--admin");
            }
            std::vector<char*> execArgs(args.size() + 1);
            std::transform(args.cbegin(), args.cend(), execArgs.begin(),
                    [](auto& s) noexcept { return stdext::as_mutable_ptr(s.c_str()); });
            char* envp[] = {nullptr};

            // Start worker process
            const auto pid = fork();
            if (pid < 0) {
                // Error occurred
                const int errorCode = errno;
                LOG_ERROR << m_logContext
                          << "Can't create new process: " << std::strerror(errorCode);
                continue;
            }

            if (pid > 0) {
                // Parent process
                {
                    std::lock_guard lock(m_connectionHandlersMutex);
                    m_connectionHandlers.insert(pid);
                }
                LOG_INFO << m_logContext << "Started new user connection worker, PID " << pid;
                continue;
            }

            // Child process
            execve(execArgs.front(), execArgs.data(), envp);
            // If we have reached here, execve() failed.
            _exit(5);
        } catch (std::exception& ex) {
            LOG_ERROR << m_logContext << ex.what();
        }
    }
}

void SiodbConnectionManager::deadConnectionCleanupThreadMain()
{
    while (!m_exitRequested) {
        {
            std::unique_lock lock(m_connectionHandlersMutex);
            const auto waitResult = m_deadConnectionCleanupThreadAwakeCondition.wait_for(
                    lock, std::chrono::seconds(
                                  m_dbOptions->m_generalOptions.m_deadConnectionCleanupInterval));
            if (waitResult != std::cv_status::timeout) continue;
        }
        removeDeadConnections();
    }
}

void SiodbConnectionManager::removeDeadConnections(bool ignoreExitRequested)
{
    LOG_DEBUG << m_logContext << "Cleaning up dead connections...";
    std::lock_guard lock(m_connectionHandlersMutex);
    LOG_DEBUG << m_logContext
              << "Number of connections before cleanup: " << m_connectionHandlers.size();
    auto it = m_connectionHandlers.begin();
    while (it != m_connectionHandlers.end()) {
        if (!ignoreExitRequested && m_exitRequested) return;
        // Check that client connection worker process still running
        const auto childPid = *it;
        int status = 0;
        const auto pid = waitpid(childPid, &status, WNOHANG);
        const int errorCode = errno;
        if (pid == 0) {
            // Process still running
            ++it;
            LOG_DEBUG << m_logContext << "Child PID " << childPid << " still running";
        } else if (pid == childPid || errorCode == ECHILD) {
            // Process exited or doesn't exist.
            it = m_connectionHandlers.erase(it);
            LOG_INFO << m_logContext << "Child PID " << childPid << " exited with code "
                     << WEXITSTATUS(status);
        } else {
            // Other error occurred, ignore by now.
            ++it;
            LOG_WARNING << m_logContext << "Check child PID " << childPid << " status failed: ["
                        << errorCode << "] " << std::strerror(errorCode);
        }
    }
    LOG_INFO << m_logContext
             << "Number of connections after cleanup: " << m_connectionHandlers.size();
}

int SiodbConnectionManager::acceptTcpConnection(int serverFd)
{
    union {
        struct sockaddr_in v4;
        struct sockaddr_in6 v6;
    } addr;

    socklen_t addrLength = m_socketDomain == AF_INET ? sizeof(addr.v4) : sizeof(addr.v6);

    // Note that last parameter of the accept4() is zero, so we intentionally want
    // resulting file descriptor to be inherited by child process.
    FDGuard client(::accept4(serverFd, reinterpret_cast<sockaddr*>(&addr), &addrLength, 0));

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

int SiodbConnectionManager::acceptUnixConnection(int serverFd)
{
    // Note that last parameter of the accept4() is zero, so we intentionally want
    // resulting file descriptor to be inherited by child process.
    FDGuard client(::accept4(serverFd, nullptr, nullptr, 0));

    if (!client.isValidFd()) {
        const int errorCode = errno;
        if (errorCode == EINTR && m_exitRequested) {
            LOG_INFO << m_logContext
                     << "UNIX connection listener thread"
                        " is exiting because database is shutting down.";
        } else {
            LOG_ERROR << m_logContext
                      << "Can't accept UNIX client connection: " << std::strerror(errorCode) << '.';
        }
        return -1;
    }

    LOG_INFO << m_logContext << "Accepted new UNIX connection.";

    // Authenticate admin user - must be member of administrative UNIX group.
    // See https://stackoverflow.com/a/18946355/1540501
    // See https://doxygen.postgresql.org/getpeereid_8c_source.html

    // Get other socket end uid and gid
    struct ucred peerCredentials;
    socklen_t len = sizeof(peerCredentials);
    if (::getsockopt(client.getFD(), SOL_SOCKET, SO_PEERCRED, &peerCredentials, &len) != 0) {
        const int errorCode = errno;
        LOG_ERROR << m_logContext << "Can't get peer credentials for incoming UNIX connection: "
                  << std::strerror(errorCode) << '.';
        return -1;
    }
    if (len != sizeof(peerCredentials)) {
        LOG_ERROR << m_logContext << "Peer credentials information differs in length: expected "
                  << sizeof(peerCredentials) << " but received " << len << '.';
        return -1;
    }

    // Check user
    std::string peerUserName;
    try {
        peerUserName = m_checkUser ? utils::checkUserBelongsToSiodbAdminGroup(
                               peerCredentials.uid, peerCredentials.gid)
                                   : utils::getOsUserName(peerCredentials.uid);
    } catch (std::exception& ex) {
        LOG_ERROR << ex.what() << '.';
        return -1;
    }

    // Report that admin connection accepted.
    LOG_INFO << m_logContext << "UNIX connection from user #" << peerCredentials.uid << " ("
             << peerUserName << ") accepted.";

    return client.release();
}

std::string SiodbConnectionManager::createLogContextName() const
{
    std::ostringstream oss;
    oss << net::getSocketDomainName(m_socketDomain) << '-' << kLogContextBase << ": ";
    return oss.str();
}

}  // namespace siodb
