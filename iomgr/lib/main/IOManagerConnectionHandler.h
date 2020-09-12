// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IOManagerRequestDispatcher.h"

// Common project headers
#include <siodb/common/io/InputOutputStream.h>
#include <siodb/common/utils/FDGuard.h>
#include <siodb/common/utils/HelperMacros.h>

// STL headers
#include <atomic>
#include <thread>

namespace siodb::iomgr::dbengine {
class RequestHandler;
}  // namespace siodb::iomgr::dbengine

namespace siodb::iomgr::dbengine::requests {
class DBEngineRequest;
}  // namespace siodb::iomgr::dbengine::requests

namespace siodb::iomgr {

/**
 * Handler for the connection with downstream client like client connection worker
 * or REST server.
 */
class IOManagerConnectionHandler : public std::enable_shared_from_this<IOManagerConnectionHandler> {
protected:
    /**
     * Initializes object of class IOManagerConnectionHandler.
     * @param clientFd Client connection file descriptor guard.
     * @param requestDispatcher Request dispatcher.
     */
    IOManagerConnectionHandler(IOManagerRequestDispatcher& requestDispatcher, FDGuard&& clientFd);

public:
    /** De-initializes object */
    virtual ~IOManagerConnectionHandler();

    DECLARE_NONCOPYABLE(IOManagerConnectionHandler);

    /** 
     * Returns indication that connection still active.
     * @return true if conneciton still active, false otherwise.
     */
    bool isConnected() noexcept
    {
        return m_clientConnection->isValid();
    }

    /** Starts connection handler thread */
    void start();

    /** 
     * Executes database engine request.
     * @param request Request to execute.
     * @return true if request execution was successful, false otherwise.
     */
    bool executeIOManagerRequest(const IOManagerRequest& request);

protected:
    /** Thread logic implementation. */
    virtual void threadLogicImpl() = 0;

    /**
     * Responds to server with error code.
     * @param requestId id of request for response.
     * @param text Text of error.
     * @param errorCode Error code.
     * @throw std::system_error when I/O error happens.
     * @throw ProtocolError when protocol error happens.
     */
    void sendErrorReponse(int requestId, const char* text, int errorCode);

    /** Closes connection. */
    void closeConnection() noexcept;

private:
    /** Entry point of the connection handler thread. */
    void threadMain();

    /**
     * Creates log context name.
     * @param fd Connection file descriptor.
     * @return Log context name.
     */
    std::string createLogContextName(int fd) const;

protected:
    /** Connection handler ID */
    const std::uint64_t m_id;

    /** Log context name */
    const std::string m_logContext;

    /** Request dispatcher */
    IOManagerRequestDispatcher& m_requestDispatcher;

    /** A file descriptor for polling connection with Client */
    FDGuard m_clientEpollFd;

    /** Client connection IO */
    std::unique_ptr<siodb::io::InputOutputStream> m_clientConnection;

    /** Client communication handler thread */
    std::unique_ptr<std::thread> m_thread;

    /** Conneciton handler ID counter */
    static std::atomic<std::uint64_t> s_idCounter;

    /* Internal error message code */
    static constexpr int kInternalError = 4;

    /** Log context name */
    static constexpr const char* kLogContextBase = "IOManagerConnectionHandler";
};

}  // namespace siodb::iomgr
