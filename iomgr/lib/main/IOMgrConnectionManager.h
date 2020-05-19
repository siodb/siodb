// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOMgrConnectionHandler.h"
#include "UniversalWorker.h"

// Common project headers
#include <siodb/common/options/InstanceOptions.h>
#include <siodb/common/utils/HelperMacros.h>

// STL headers
#include <list>

namespace siodb::iomgr {

class IOMgrConnectionManager {
public:
    /**
     * Initialized object of class IOMgrConnectionManager.
     * @param socketDomain TCP/IP socket domain, can be AF_INET or AF_INET6.
     * @param instanceOptions Database options.
     * @param instance DBMS instance
     */
    IOMgrConnectionManager(int socketDomain, const config::ConstInstaceOptionsPtr& instanceOptions,
            const dbengine::InstancePtr& instance);

    /** Cleans up object */
    ~IOMgrConnectionManager();

    DECLARE_NONCOPYABLE(IOMgrConnectionManager);

private:
    /** Connection listener thread entry point */
    void connectionListenerThreadMain();

    /** Dead connection recycler thread entry point */
    void deadConnectionRecyclerThreadMain();

    /**
     * Removes dead connection handlers.
     */
    void removeDeadConnections();

    /**
     * Accepts TCP connection.
     * @param serverFd Server socket file descriptor.
     * @return Client connection file descriptor, or -1 on error.
     */
    int acceptTcpConnection(int serverFd);

    /**
     * Validates listener socket domain.
     * @param socketDomain TCP/IP socket domain type.
     * @return socketDomain if it is valid.
     * @throw std::invalid_argument if socketDomain is invalid.
     */
    static int checkSocketDomain(int socketDomain);

    /**
     * Creates worker thread pool.
     * @param size Pool size (number of threads in the pool).
     */
    static std::vector<std::unique_ptr<UniversalWorker>> createWorkerThreadPool(std::size_t size);

private:
    /** Socket domain */
    const int m_socketDomain;

    /** Socket type string */
    const char* const m_socketTypeName;

    /** Database options */
    const config::ConstInstaceOptionsPtr m_dbOptions;

    /** Exit request flag */
    std::atomic<bool> m_exitRequested;

    /** Connection threads access synchronization object */
    std::mutex m_connectionHandlersMutex;

    /** DBMS instance */
    const dbengine::InstancePtr m_instance;

    /** Worker thread pool */
    std::vector<std::unique_ptr<UniversalWorker>> m_workerThreadPool;

    /** Connection handlers */
    std::list<IOMgrConnectionHandler> m_connectionHandlers;

    /** Connection listener thread */
    std::thread m_connectionListenerThread;

    /** Dead connection recycling thread awake condition */
    std::condition_variable m_deadConnectionRecyclerThreadAwakeCondition;

    /** Dead connection monitor thread */
    std::thread m_deadConnectionRecyclerThread;

    /** Log context name */
    static constexpr const char* kLogContext = "IOMgrConnectionManager: ";

    /** Period of checking that IO manager connection handler is active */
    static constexpr std::chrono::seconds kDeadConnectionsRecyclePeriod = std::chrono::seconds(30);
};

}  // namespace siodb::iomgr
