// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/options/InstanceOptions.h>
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
     * Initialized object of class SiodbConnectionManager.
     * @param socketDomain Socket domain, can be AF_UNIX, AF_INET or AF_INET6.
     * @param checkUser Check user for UNIX socket.
     * @param instanceOptions Database options.
     */
    explicit SiodbConnectionManager(int socketDomain, bool checkUser,
            const config::ConstInstaceOptionsPtr& instanceOptions);

    /** Cleans up object */
    ~SiodbConnectionManager();

    DECLARE_NONCOPYABLE(SiodbConnectionManager);

private:
    /** Connection listener thread entry point */
    void connectionListenerThreadMain();

    /** Dead connection recycler thread entry point */
    void deadConnectionRecyclerThreadMain();

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
     * Validates listener socket domain.
     * @param socketDomain TCP/IP socket domain type.
     * @return socketDomain if it is valid.
     * @throw std::invalid_argument if socketDomain is invalid.
     */
    static int checkSocketDomain(int socketDomain);

private:
    /** Socket domain */
    const int m_socketDomain;

    /** Socket type string */
    const char* const m_socketTypeName;

    /** Whether to check user for UNIX socket */
    const bool m_checkUser;

    /** Database options */
    const config::ConstInstaceOptionsPtr m_dbOptions;

    /** Connection worker executable path */
    const std::string m_workerExecutablePath;

    /** Exit request flag */
    std::atomic<bool> m_exitRequested;

    /** Dead connection recycling period */
    std::chrono::milliseconds m_deadConnectionRecyclingPeriod;

    /** Connection threads access synchronization object */
    std::mutex m_connectionHandlersMutex;

    /** Dead connection recycling therad awake condition */
    std::condition_variable m_deadConnectionRecyclerThreadAwakeCondition;

    /** Connection handlers */
    std::unordered_set<pid_t> m_connectionHandlers;

    /** Dead connection monitor thread */
    std::thread m_deadConnectionRecyclerThread;

    /** Connection listener thread */
    std::thread m_connectionListenerThread;

    /** Log context name */
    static constexpr const char* kLogContext = "SiodbConnectionManager: ";

    /** Period of checking that connection handler process is dead
     *  when termination is requested
     */
    static constexpr auto kTerminateConnectionsCheckPeriod = std::chrono::milliseconds(500);
};

}  // namespace siodb
