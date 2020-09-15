// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerConnectionHandlerFactory.h"

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
     * @param name Connection managr name, used for logging.
     * @param socketDomain TCP/IP socket domain, can be AF_INET or AF_INET6.
     * @param port Server port.
     * @param connectionListenerBacklog Conneciton listener queue size.
     * @param deadConnectionCleanupInterval Dead connection cleanup interval in seconds.
     * @param requestDispatcher Request dispatcher.
     * @param connectionHandlerFactory Connection handler factory.
     */
    IOManagerConnectionManager(const char* name, int socketDomain, int port,
            unsigned connectionListenerBacklog, unsigned deadConnectionCleanupInterval,
            IOManagerRequestDispatcher& requestDispatcher,
            IOManagerConnectionHandlerFactory& connectionHandlerFactory);

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
     * @param Connection manager name.
     * @return Log context name.
     */
    std::string createLogContextName(const char* name) const;

private:
    /** Socket domain */
    const int m_socketDomain;

    /** Socket type string */
    const std::string m_logContext;

    /** Server port */
    const int m_port;

    /** Connection listener backlog */
    const unsigned m_userConnectionListenerBacklog;

    /** Dead connection cleanup interval */
    const std::chrono::seconds m_deadConnectionCleanupInterval;

    /** Request dispartcher */
    IOManagerRequestDispatcher& m_requestDispatcher;

    /** Connection handler factory object */
    IOManagerConnectionHandlerFactory& m_connectionHandlerFactory;

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
};

}  // namespace siodb::iomgr
