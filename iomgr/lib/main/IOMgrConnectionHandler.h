// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ClientSession.h"
#include "../dbengine/InstancePtr.h"

// Common project headers
#include <siodb/common/io/IoBase.h>
#include <siodb/common/utils/FileDescriptorGuard.h>
#include <siodb/common/utils/HelperMacros.h>

// STL headers
#include <atomic>
#include <thread>

namespace siodb::iomgr {

/** Handler for the Siodb server connection */
class IOMgrConnectionHandler final {
public:
    /**
     * Initializes object of class IOMgrConnectionHandler.
     * @param clientFd Client connection file descriptor guard.
     * @param instance Instance
     */
    IOMgrConnectionHandler(FileDescriptorGuard&& clientFd, const dbengine::InstancePtr& instance);

    /**
     * Cleans up object
     */
    ~IOMgrConnectionHandler();

    DECLARE_NONCOPYABLE(IOMgrConnectionHandler);

    /** returns wheiter thread body is active or not
     * @return wheiter thread body is active or not
     */
    bool isConnected() noexcept
    {
        return m_clientIo->isValid();
    }

    /**
     * Closes connection with Siodb server
     */
    void closeConnection() noexcept;

private:
    /**
     * Response to server with error code
     * @param requestId id of request for response
     * @param text Text of error
     * @param errCode Error code
     * @param ioMgrInputStream Input steam
     * @return Database engine response from IO manager
     * @throw std::system_error when I/O error happens.
     * @throw SiodbProtocolError when protocol error happens.
     */
    void respondToServerWithError(int requestId, const char* text, int errCode);

    /**
     * Receives BeginAuthenticateUser request and verifies user name and active keys count.
     */
    void beginUserAuthentication();

    /**
     * Receives authentication request and verifies user challenge, name and signature.
     * @return Pair (user ID, new session UUID).
     */
    std::pair<std::uint32_t, Uuid> authenticateUser();

    /** Thread body of connection handler, waits for command, executes it and sends response.
     */
    void threadMain();

private:
    /** Error codes enumeration */
    enum {
        /** SQL parsing error */
        kSqlParseError = 2,

        /* Internal error happened */
        kInternalError = 3,
    };

    /** A file descriptor for polling connection with Client */
    FileDescriptorGuard m_clientEpollFd;

    /** Client connection IO */
    std::unique_ptr<siodb::io::IoBase> m_clientIo;

    /** User name */
    std::string m_userName;

    /** DBMS instance */
    dbengine::InstancePtr m_instance;

    /** Thread for protobuf commmunication with Siodb */
    std::unique_ptr<std::thread> m_thread;

    /** Log context name */
    static constexpr const char* kLogContext = "IOMgrConnectionHandler: ";
};

}  // namespace siodb::iomgr
