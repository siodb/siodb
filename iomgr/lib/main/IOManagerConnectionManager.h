// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerConnectionHandler.h"

// Common project headers
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/utils/HelperMacros.h>

// STL headers
#include <condition_variable>
#include <list>

namespace siodb::iomgr {

class IOManagerConnectionManager {
public:
    /**
     * Initialized object of class IOManagerConnectionManager.
     * @param socketDomain TCP/IP socket domain, can be AF_INET or AF_INET6.
     * @param instanceOptions Database options.
     * @param requestDispatcher Request dispatcher.
     */
    IOManagerConnectionManager(int socketDomain,
            const config::ConstInstaceOptionsPtr& instanceOptions,
            IOManagerRequestDispatcher& requestDispatcher);

    /** De-initializes object */
    ~IOManagerConnectionManager();

    DECLARE_NONCOPYABLE(IOManagerConnectionManager);

private:
    /** Connection listener thread entry point */
    void connectionListenerThreadMain();

    /** Dead connection recycler thread entry point */
    void deadConnectionCleanupThreadMain();

    /** Removes dead connection handlers. */
    void removeDeadConnections();

    /**
     * Accepts TCP connection.
     * @param serverFd Server socket file descriptor.
     * @return Client connection file descriptor, or -1 on error.
     */
    int acceptTcpConnection(int serverFd);

    /**
     * Creates log context name.
     * @return Log context name.
     */
    std::string createLogContextName() const;

private:
    /** Socket domain */
    const int m_socketDomain;

    /** Socket type string */
    const std::string m_logContext;

    /** Database options */
    const config::ConstInstaceOptionsPtr m_dbOptions;

    /** Request dispartcher */
    IOManagerRequestDispatcher& m_requestDispatcher;

    /** Exit request flag */
    std::atomic<bool> m_exitRequested;

    /** Connection threads access synchronization object */
    std::mutex m_connectionHandlersMutex;

    /** Connection handlers */
    std::list<std::shared_ptr<IOManagerConnectionHandler>> m_connectionHandlers;

    /** Connection listener thread */
    std::thread m_connectionListenerThread;

    /** Dead connection cleanup thread awake condition */
    std::condition_variable m_deadConnectionCleanupThreadAwakeCondition;

    /** Dead connection monitor thread */
    std::thread m_deadConnectionCleanupThread;

    /** Log context name */
    static constexpr const char* kLogContextBase = "IOManagerConnectionManager";
};

}  // namespace siodb::iomgr
