// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ClientSession.h"
#include "IOManagerRequestDispatcher.h"
#include "../dbengine/AuthenticationResult.h"

// Common project headers
#include <siodb/common/io/IoBase.h>
#include <siodb/common/utils/FdGuard.h>
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
class IOManagerConnectionHandler final
    : public std::enable_shared_from_this<IOManagerConnectionHandler> {
public:
    /**
     * Initializes object of class IOManagerConnectionHandler.
     * @param clientFd Client connection file descriptor guard.
     * @param requestDispatcher Request dispatcher.
     */
    IOManagerConnectionHandler(IOManagerRequestDispatcher& requestDispatcher, FdGuard&& clientFd);

    /** Deinitializes object */
    ~IOManagerConnectionHandler();

    DECLARE_NONCOPYABLE(IOManagerConnectionHandler);

    /** 
     * Returns indication that connection still active.
     * @return true if conneciton still active, false otherwise.
     */
    bool isConnected() noexcept
    {
        return m_clientIo->isValid();
    }

    /** 
     * Executes database engine request.
     * @param request Request to execute.
     * @return true if request execution was successful, false otherwise.
     */
    bool executeIOManagerRequest(const IOManagerRequest& request);

    /** Closes connection. */
    void closeConnection() noexcept;

private:
    /** Entry point of the connection handler thread. */
    void threadMain();

    /** Authenticates user on the connection. */
    void authenticateUser();

    /**
     * Responds to server with error code.
     * @param requestId id of request for response.
     * @param text Text of error.
     * @param errorCode Error code.
     * @throw std::system_error when I/O error happens.
     * @throw SiodbProtocolError when protocol error happens.
     */
    void sendErrorReponse(int requestId, const char* text, int errorCode);

    /**
     * Creates log context name.
     * @param fd Connection file descriptor.
     * @return Log context name.
     */
    std::string createLogContextName(int fd) const;

private:
    /** Error codes enumeration */
    enum {
        /** SQL parsing error */
        kSqlParseError = 2,

        /* Internal error happened */
        kInternalError = 3,
    };

    /** Connection handler ID */
    const std::uint64_t m_id;

    /** Log context name */
    const std::string m_logContext;

    /** Request dispatcher */
    IOManagerRequestDispatcher& m_requestDispatcher;

    /** A file descriptor for polling connection with Client */
    FdGuard m_clientEpollFd;

    /** Client connection IO */
    std::unique_ptr<siodb::io::IoBase> m_clientIo;

    /** User name */
    std::string m_userName;

    /** Authentication result */
    dbengine::AuthenticationResult m_authResult;

    /** Request handler object */
    std::shared_ptr<dbengine::RequestHandler> m_requestHandler;

    /** Client communication handler thread */
    std::unique_ptr<std::thread> m_thread;

    /** Conneciton handler ID counter */
    static std::atomic<std::uint64_t> s_idCounter;

    /** Log context name */
    static constexpr const char* kLogContextBase = "IOManagerConnectionHandler";
};

}  // namespace siodb::iomgr
