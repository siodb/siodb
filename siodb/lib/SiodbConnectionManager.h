// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/utils/HelperMacros.h>

// STL headers
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_set>

// System headers
#include <unistd.h>

namespace siodb {

class SiodbConnectionManager {
public:
    /**
     * Initializes object of class SiodbConnectionManager.
     * @param socketDomain Socket domain, can be AF_UNIX, AF_INET or AF_INET6.
     * @param checkUser Check user for UNIX socket.
     * @param instanceOptions Database options.
     */
    SiodbConnectionManager(int socketDomain, bool checkUser,
            const config::ConstInstaceOptionsPtr& instanceOptions);

    /** De-initializes object */
    ~SiodbConnectionManager();

    DECLARE_NONCOPYABLE(SiodbConnectionManager);

private:
    /** Connection listener thread entry point */
    void connectionListenerThreadMain();

    /** Dead connection cleanup thread entry point */
    void deadConnectionCleanupThreadMain();

    /**
     * Removes dead connection handlers.
     * @param ignoreExitRequested Whether to ignore "exit requested" flag.
     */
    void removeDeadConnections(bool ignoreExitRequested = false);

    /**
     * Accepts TCP connection.
     * @param serverFd Server socket file descriptor.
     * @return Client connection file descriptor, or -1 on error.
     */
    int acceptTcpConnection(int serverFd);

    /**
     * Accepts Unix connection.
     * @param serverFd Server socket file descriptor.
     * @param checkUser Check that use belongs to admin group.
     * @return Client connection file descriptor, or -1 on error.
     */
    int acceptUnixConnection(int serverFd);

    /**
     * Creates log context name.
     * @return Log context name.
     */
    std::string createLogContextName() const;

private:
    /** Socket domain */
    const int m_socketDomain;

    /** Log context name */
    const std::string m_logContext;

    /** Whether to check user for UNIX socket */
    const bool m_checkUser;

    /** Database options */
    const config::ConstInstaceOptionsPtr m_dbOptions;

    /** Connection worker executable path */
    const std::string m_workerExecutablePath;

    /** Exit request flag */
    std::atomic<bool> m_exitRequested;

    /** Connection threads access synchronization object */
    std::mutex m_connectionHandlersMutex;

    /** Dead connection cleanup therad awake condition */
    std::condition_variable m_deadConnectionCleanupThreadAwakeCondition;

    /** Connection handlers */
    std::unordered_set<pid_t> m_connectionHandlers;

    /** Dead connection monitor thread */
    std::thread m_deadConnectionCleanupThread;

    /** Connection listener thread */
    std::thread m_connectionListenerThread;

    /** Log context name */
    static constexpr const char* kLogContextBase = "SiodbConnectionManager";

    /** Period of checking that connection handler process is dead when termination is requested */
    static constexpr auto kTerminateConnectionsCheckPeriod = std::chrono::milliseconds(500);
};

}  // namespace siodb
