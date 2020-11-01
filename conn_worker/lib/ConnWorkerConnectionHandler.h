// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/crypto/TlsConnection.h>
#include <siodb/common/crypto/TlsServer.h>
#include <siodb/common/io/InputOutputStream.h>
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/protobuf/StreamInputStream.h>
#include <siodb/common/protobuf/StreamOutputStream.h>
#include <siodb/common/utils/FDGuard.h>
#include <siodb/common/utils/HelperMacros.h>

// Protobuf message headers
#include <siodb/common/proto/ClientProtocol.pb.h>
#include <siodb/common/proto/IOManagerProtocol.pb.h>

namespace siodb::conn_worker {

/** Handler for the client connection */
class ConnWorkerConnectionHandler {
public:
    /**
     * Initializes object of class AdminConnWorkerConnectionHandler.
     * @param client Client file descriptor.
     * @param instanceOptions Database instance options.
     * @param adminMode Database administrator mode.
     */
    ConnWorkerConnectionHandler(FDGuard&& client,
            const config::ConstInstaceOptionsPtr& instanceOptions, bool adminMode);

    DECLARE_NONCOPYABLE(ConnWorkerConnectionHandler);

    /** Handles user connection until it is disconnected */
    void run();

    /** Forcibly closes connection */
    void closeConnection();

private:
    /**
     * Response to client with error code
     * @param requestId id of request for response
     * @param text Text of error
     * @param errorCode Error code
     * @param ioMgrInputStream Input steam
     * @return Database engine response from IO manager
     * @throw std::system_error when I/O error happens.
     * @throw ProtocolError when protocol error happens.
     */
    void responseToClientWithError(int requestId, const char* text, int errorCode);

    /**
     * Receives row data from IO manager and sends to client.
     * @param ioMgrInputStream Input stream.
     * @throw std::system_error when I/O error happens.
     * */
    void forwardRowData(protobuf::StreamInputStream& ioMgrInputStream);

    /**
     * Updates used database to @ref m_lastUsedDatabase (Sends USE DATABASE to Iomgr).
     * @param ioMgrInputStream Input stream.
     * @throw std::system_error when I/O error happens.
     * @throw std::runtime_error when Iomgr responses with non expected answer.
     */
    void selectLastUsedDatabase(protobuf::StreamInputStream& ioMgrInputStream);

    /**
     * Processes Iomgr response tag.
     * @param tag Response tag
     */
    void processTag(const iomgr_protocol::Tag& tag);

    /** Starts authentication sequence:
     * 1) Waits BeginSessionRequest from client to begin session.
     * 2) Sends BeginAuthenticateUserRequest to iomgr.
     * 3) Waits BeginAuthenticateUserResponse from iomgr.
     * 4a) [Session started] Responses to client BeginSessionResponse with challenge(random sequence of bytes(128-1024).
     * 4b) [Session not started] Responses to client with error message.
     * 5) Waits for the message from the client with signed challenge with his key.
     * 6) Sends challenge, signed challenge and user name to Io manager for authentication.
     * 7) Waits for the message from IO manager with authentication result.
     * 8a) [Authenticated] Sends authentication result to the client and continues working.
     * 8b) [Non-Authenticated] Sends authentication result to the client and closes connection.
     * @param ioMgrInputStream Iomgr input stream.
     */
    void authenticateUser(protobuf::StreamInputStream& ioMgrInputStream);

    /**
     * Creates TLS server.
     * @param clientOptions Client options.
     * @return TLS server.
     */
    std::unique_ptr<siodb::crypto::TlsServer> createTlsServer(
            const config::ClientOptions& clientOptions) const;

private:
    /** Database options */
    const config::ConstInstaceOptionsPtr m_dbOptions;

    /** Database administrator mode flag */
    const bool m_adminMode;

    /** IO for connection with Siodb client */
    std::unique_ptr<io::InputOutputStream> m_clientConnection;

    /** IO for connection with IO manager */
    std::unique_ptr<io::InputOutputStream> m_iomgrConnection;

    /** TLS server for handling secure connnection */
    std::unique_ptr<crypto::TlsServer> m_tlsServer;

    /** Last used database */
    std::string m_lastUsedDatabase;

    /** A file descriptor for polling connection with the client */
    FDGuard m_clientEpollFd;

    /** Log context name */
    static constexpr const char* kLogContext = "ConnWorkerConnectionHandler: ";

    /** Request ID for used database reset after Iomgr connection error */
    static constexpr std::uint64_t kUseDatabaseRequestId = 0xDB1D;

    /** Error codes enumeration */
    enum {
        /** Connection with IO manager failed or unexpectedly closed */
        kIoMgrConnectionError = 3,
    };
};

}  // namespace siodb::conn_worker
