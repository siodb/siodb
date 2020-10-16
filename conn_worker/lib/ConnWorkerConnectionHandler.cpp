// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConnWorkerConnectionHandler.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/io/FDStream.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/net/ConnectionError.h>
#include <siodb/common/net/EpollHelpers.h>
#include <siodb/common/net/TcpConnection.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/protobuf/ProtocolTag.h>
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/common/stl_ext/system_error_ext.h>
#include <siodb/common/utils/ErrorCodeChecker.h>
#include <siodb/common/utils/RandomUtils.h>
#include <siodb/common/utils/SignalHandlers.h>

// STL headers
#include <iostream>
#include <sstream>

// Boost headers
#include <boost/algorithm/string.hpp>
#include <boost/endian/conversion.hpp>

// System headers
#include <sys/epoll.h>

// Protobuf headers
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace siodb::conn_worker {

namespace {

std::string createChallenge()
{
    constexpr auto kMinMaxDifference = kMaxChallengeSize - kMinChallengeSize;
    std::size_t rv = 0;
    siodb::utils::getRandomBytes(reinterpret_cast<std::uint8_t*>(&rv), sizeof(rv));
    const auto challengeSize = kMinChallengeSize + rv % (kMinMaxDifference + 1);
    std::string challenge(challengeSize, '\0');
    siodb::utils::getRandomBytes(
            reinterpret_cast<std::uint8_t*>(challenge.data()), challenge.size());
    return challenge;
}

}  // namespace

ConnWorkerConnectionHandler::ConnWorkerConnectionHandler(
        FDGuard&& client, const config::ConstInstaceOptionsPtr& instanceOptions, bool adminMode)
    : m_dbOptions(instanceOptions)
    , m_adminMode(adminMode)
{
    m_clientEpollFd.reset(net::createEpollFd(client.getFD(), EPOLLIN));
    if (!m_adminMode && m_dbOptions->m_clientOptions.m_enableEncryption) {
        LOG_DEBUG << kLogContext << "Established secure connection with client";
        m_tlsServer = createTlsServer(m_dbOptions->m_clientOptions);
        m_clientConnection = std::move(m_tlsServer->acceptConnection(client.release(), true));
    } else {
        LOG_DEBUG << kLogContext << " established non-secure connection with client";
        m_clientConnection = std::make_unique<siodb::io::FDStream>(client.release(), true);
    }

    if (!m_clientConnection->isValid())
        throw std::invalid_argument("Invalid client communication channel");

    int port = m_dbOptions->m_ioManagerOptions.m_ipv4SqlPort != 0
                       ? m_dbOptions->m_ioManagerOptions.m_ipv4SqlPort
                       : m_dbOptions->m_ioManagerOptions.m_ipv6SqlPort;

    FDGuard fdGuard(net::openTcpConnection("localhost", port, true));
    auto iomgrConnection = std::make_unique<io::FDStream>(fdGuard.getFD(), false);
    fdGuard.release();
    iomgrConnection->setAutoClose();
    m_iomgrConnection = std::move(iomgrConnection);
}

void ConnWorkerConnectionHandler::run()
{
    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;

    auto ioMgrInputStream =
            std::make_unique<protobuf::StreamInputStream>(*m_iomgrConnection, errorCodeChecker);

    authenticateUser(*ioMgrInputStream);

    while (true) {
        try {
            // Read message from client
            client_protocol::Command command;
            LOG_DEBUG << kLogContext << "Waiting for command...";
            try {
                // NOTE: In case of the TCP connection close or abort
                // we can receive an empty message
                net::epollWaitForData(m_clientEpollFd.getFD(), true);
                protobuf::readMessage(protobuf::ProtocolMessageType::kCommand, command,
                        *m_clientConnection, errorCodeChecker);
            } catch (net::ConnectionError& err) {
                // Connection was closed or hangup. No reading operation was in progress.
                LOG_DEBUG << kLogContext << "Client disconnected";
                closeConnection();
                return;
            } catch (std::exception& ex) {
                if (!utils::isExitEventSignaled()) {
                    LOG_ERROR << kLogContext << ex.what() << '.';
                }
                closeConnection();
                return;
            }

            LOG_DEBUG << kLogContext << "Received command: " << command.text();

            try {
                try {
                    LOG_DEBUG << kLogContext << "Sending database engine request";

                    iomgr_protocol::DatabaseEngineRequest dbeRequest;
                    dbeRequest.set_text(command.text());
                    dbeRequest.set_request_id(command.request_id());

                    // Connect to server
                    protobuf::writeMessage(protobuf::ProtocolMessageType::kDatabaseEngineRequest,
                            dbeRequest, *m_iomgrConnection);
                } catch (const ProtocolError& ex) {
                    LOG_ERROR << kLogContext << ex.what();
                    m_iomgrConnection->close();

                    int port = m_dbOptions->m_ioManagerOptions.m_ipv4SqlPort != 0
                                       ? m_dbOptions->m_ioManagerOptions.m_ipv4SqlPort
                                       : m_dbOptions->m_ioManagerOptions.m_ipv6SqlPort;

                    FDGuard fdGuard(net::openTcpConnection("localhost", port, true));
                    auto iomgrConnection = std::make_unique<io::FDStream>(fdGuard.getFD(), false);
                    fdGuard.release();
                    iomgrConnection->setAutoClose();
                    m_iomgrConnection = std::move(iomgrConnection);

                    ioMgrInputStream = std::make_unique<protobuf::StreamInputStream>(
                            *m_iomgrConnection, errorCodeChecker);

                    responseToClientWithError(
                            command.request_id(), ex.what(), kIoMgrConnectionError);

                    if (!m_lastUsedDatabase.empty()) selectLastUsedDatabase(*ioMgrInputStream);

                    continue;
                }

                std::size_t responseId = 0, responseCount = 0;
                do {
                    client_protocol::ServerResponse response;
                    iomgr_protocol::DatabaseEngineResponse dbeResponse;

                    protobuf::readMessage(protobuf::ProtocolMessageType::kDatabaseEngineResponse,
                            dbeResponse, *ioMgrInputStream);

                    LOG_DEBUG << kLogContext << "Received response for the request #"
                              << dbeResponse.request_id();

                    // Prepare response
                    response.set_request_id(command.request_id());
                    response.set_response_id(dbeResponse.response_id());
                    response.set_response_count(dbeResponse.response_count());
                    response.mutable_column_description()->Swap(
                            dbeResponse.mutable_column_description());
                    response.mutable_message()->Swap(dbeResponse.mutable_message());
                    response.mutable_freetext_message()->Swap(
                            dbeResponse.mutable_freetext_message());
                    response.set_affected_row_count(dbeResponse.affected_row_count());
                    response.set_has_affected_row_count(dbeResponse.has_affected_row_count());

                    // Send response
                    protobuf::writeMessage(protobuf::ProtocolMessageType::kServerResponse, response,
                            *m_clientConnection);

                    // Capture response count
                    if (responseId == 0) {
                        responseCount = response.response_count();
                        if (responseCount == 0) responseCount = 1;
                    }

                    LOG_DEBUG << kLogContext << "Sent response #" << response.response_id() << '/'
                              << responseCount;

                    bool error = false;
                    const int messageCount = response.message_size();
                    if (messageCount > 0) {
                        for (int i = 0; i < messageCount && !error; ++i)
                            error |= response.message(i).status_code() != 0;
                    }

                    if (!error) {
                        if (response.column_description_size() > 0)
                            transmitRowData(*ioMgrInputStream);

                        for (int i = 0; i < dbeResponse.tag_size(); ++i)
                            processTag(dbeResponse.tag(i));
                    }

                    ++responseId;
                } while (responseId < responseCount);
            } catch (std::exception& ex) {
                LOG_ERROR << kLogContext << ex.what() << '.';
                closeConnection();
                return;
            }
        } catch (std::exception& ex) {
            LOG_ERROR << kLogContext << ex.what() << '.';
        }
    }
}

void ConnWorkerConnectionHandler::closeConnection()
{
    LOG_DEBUG << kLogContext << "Closing connection";
    m_iomgrConnection.reset();
    m_clientEpollFd.reset();
    m_clientConnection.reset();
    m_tlsServer.reset();
}

// ----- internals -----

void ConnWorkerConnectionHandler::responseToClientWithError(
        int requestId, const char* text, int errorCode)
{
    client_protocol::ServerResponse response;
    response.set_request_id(requestId);
    const auto message = response.add_message();
    message->set_status_code(errorCode);
    message->set_text(text);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kServerResponse, response, *m_clientConnection);
}

void ConnWorkerConnectionHandler::transmitRowData(protobuf::StreamInputStream& ioMgrInputStream)
{
    std::uint64_t totalBytesSent = 0;

    google::protobuf::io::CodedInputStream codedInput(&ioMgrInputStream);

    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;
    protobuf::StreamOutputStream clientOutputStream(*m_clientConnection, errorCodeChecker);
    google::protobuf::io::CodedOutputStream codedOutput(&clientOutputStream);
    while (true) {
        std::uint64_t rowLength = 0;
        if (!codedInput.ReadVarint64(&rowLength))
            stdext::throw_system_error("IO manager socket read error");

        std::uint8_t codedRowLength[9];
        const auto codedRowLengthEnd =
                google::protobuf::io::CodedOutputStream::WriteVarint64ToArray(
                        rowLength, codedRowLength);
        const std::size_t readLengthBytes = codedRowLengthEnd - codedRowLength;

        codedOutput.WriteRaw(codedRowLength, readLengthBytes);
        totalBytesSent += readLengthBytes;

        if (rowLength == 0) break;  // IOManager finished sending row data

        std::uint64_t rowDataBytesSent = 0;
        while (rowDataBytesSent < rowLength) {
            const void* data = nullptr;
            int bufferSize = 0;
            codedInput.GetDirectBufferPointer(&data, &bufferSize);

            const std::size_t dataSize =
                    std::min(static_cast<std::size_t>(bufferSize), rowLength - rowDataBytesSent);
            codedOutput.WriteRaw(data, dataSize);

            codedInput.Skip(dataSize);
            rowDataBytesSent += dataSize;
        }
        totalBytesSent += rowLength;
    }

    LOG_DEBUG << kLogContext << "Sent " << totalBytesSent << " bytes of row data";
}

void ConnWorkerConnectionHandler::selectLastUsedDatabase(
        protobuf::StreamInputStream& ioMgrInputStream)
{
    LOG_DEBUG << kLogContext << "Selecting last used database";

    iomgr_protocol::DatabaseEngineRequest dbeRequest;
    dbeRequest.set_request_id(kUseDatabaseRequestId);
    dbeRequest.set_text(stdext::string_builder() << "USE DATABASE " << m_lastUsedDatabase);

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineRequest, dbeRequest, *m_iomgrConnection);

    iomgr_protocol::DatabaseEngineResponse dbeResponse;

    protobuf::readMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, dbeResponse, ioMgrInputStream);

    LOG_DEBUG << kLogContext << "USE DATABASE response" << dbeResponse.request_id();

    if (dbeResponse.request_id() != kUseDatabaseRequestId) {
        throw std::runtime_error("USE DATABASE response got invalid request ID");
    }

    if (dbeResponse.response_count() != 1)
        throw std::runtime_error("USE DATABASE response got multiple responses");

    bool errorFound = false;
    for (int i = 0, messageCount = dbeResponse.message_size(); !errorFound && i < messageCount; ++i)
        errorFound |= dbeResponse.message(i).status_code() != 0;

    if (errorFound) throw std::runtime_error("USE DATABASE response contains errors");

    if (dbeResponse.column_description_size() > 0)
        throw std::runtime_error("USE DATABASE response contains unexpected row data");
}

void ConnWorkerConnectionHandler::processTag(const iomgr_protocol::Tag& tag)
{
    if (tag.name() == kCurrentDatabaseTag) {
        if (tag.value_case() == iomgr_protocol::Tag::kStringValue)
            m_lastUsedDatabase = tag.string_value();
        else
            throw std::runtime_error(
                    std::string(kCurrentDatabaseTag) + " tag value is not a string");
    }
}

void ConnWorkerConnectionHandler::authenticateUser(
        [[maybe_unused]] protobuf::StreamInputStream& ioMgrInputStream)
{
    client_protocol::BeginSessionRequest beginSessionRequest;
    // Allow EINTR to cause I/O error when exit signal detected.
    const utils::ExitSignalAwareErrorCodeChecker errorCodeChecker;

    LOG_DEBUG << kLogContext << "Waiting for BeginSessionRequest request...";
    protobuf::readMessage(protobuf::ProtocolMessageType::kClientBeginSessionRequest,
            beginSessionRequest, *m_clientConnection, errorCodeChecker);
    LOG_DEBUG << kLogContext << "Received BeginSessionRequest from client";

    iomgr_protocol::BeginAuthenticateUserRequest beginAuthenticateUserRequest;
    const auto& userName = beginSessionRequest.user_name();
    if (userName.length() > 2 && userName.front() == '\"' && userName.back() == '\"')
        beginAuthenticateUserRequest.set_user_name(userName.substr(1, userName.length() - 2));
    else
        beginAuthenticateUserRequest.set_user_name(boost::to_upper_copy(userName));

    protobuf::writeMessage(protobuf::ProtocolMessageType::kBeginAuthenticateUserRequest,
            beginAuthenticateUserRequest, *m_iomgrConnection);
    LOG_DEBUG << kLogContext << "Sent BeginAuthenticateUserRequest to client";

    LOG_DEBUG << kLogContext << "Waiting for iomgr BeginAuthenticateUserResponse...";
    iomgr_protocol::BeginAuthenticateUserResponse beginAuthenticateUserResponse;
    protobuf::readMessage(protobuf::ProtocolMessageType::kBeginAuthenticateUserResponse,
            beginAuthenticateUserResponse, *m_iomgrConnection, errorCodeChecker);
    LOG_DEBUG << kLogContext << "Received BeginAuthenticateUserResponse from iomgr";

    client_protocol::BeginSessionResponse clientBeginSessionResponse;
    clientBeginSessionResponse.set_session_started(beginAuthenticateUserResponse.session_started());
    if (beginAuthenticateUserResponse.has_message()) {
        clientBeginSessionResponse.set_allocated_message(
                beginAuthenticateUserResponse.release_message());
    }

    if (beginAuthenticateUserResponse.session_started())
        clientBeginSessionResponse.set_challenge(createChallenge());

    protobuf::writeMessage(protobuf::ProtocolMessageType::kClientBeginSessionResponse,
            clientBeginSessionResponse, *m_clientConnection);
    LOG_DEBUG << kLogContext << "Sent BeginSessionResponse to client";

    if (!clientBeginSessionResponse.session_started()) {
        closeConnection();
        throw std::runtime_error("Begin session failed");
    }

    LOG_DEBUG << kLogContext << "Waiting for authentication request...";
    client_protocol::ClientAuthenticationRequest authRequest;
    protobuf::readMessage(protobuf::ProtocolMessageType::kClientAuthenticationRequest, authRequest,
            *m_clientConnection, errorCodeChecker);
    LOG_DEBUG << kLogContext << "Received client authentication request";

    iomgr_protocol::AuthenticateUserRequest authenticateUserRequest;
    authenticateUserRequest.set_allocated_challenge(clientBeginSessionResponse.release_challenge());
    authenticateUserRequest.set_signature(authRequest.signature());

    protobuf::writeMessage(protobuf::ProtocolMessageType::kAuthenticateUserRequest,
            authenticateUserRequest, *m_iomgrConnection);
    LOG_DEBUG << kLogContext << "Sent AuthenticateUserRequest to iomgr";

    LOG_DEBUG << kLogContext << "Waiting for iomgr authentication response...";
    iomgr_protocol::AuthenticateUserResponse iomgrAuthResponse;
    protobuf::readMessage(protobuf::ProtocolMessageType::kAuthenticateUserResponse,
            iomgrAuthResponse, *m_iomgrConnection, errorCodeChecker);
    LOG_DEBUG << kLogContext << "Received authentication response from iomgr";

    client_protocol::ClientAuthenticationResponse clientAuthResponse;
    clientAuthResponse.set_authenticated(iomgrAuthResponse.authenticated());
    clientAuthResponse.set_allocated_session_id(iomgrAuthResponse.release_session_id());
    if (iomgrAuthResponse.has_message())
        clientAuthResponse.set_allocated_message(iomgrAuthResponse.release_message());

    protobuf::writeMessage(protobuf::ProtocolMessageType::kClientAuthenticationResponse,
            clientAuthResponse, *m_clientConnection);

    if (!clientAuthResponse.authenticated()) {
        closeConnection();
        throw std::runtime_error("User authentication failed");
    }
}

std::unique_ptr<siodb::crypto::TlsServer> ConnWorkerConnectionHandler::createTlsServer(
        const config::ClientOptions& clientOptions) const
{
    auto tlsServer = std::make_unique<siodb::crypto::TlsServer>();

    if (!clientOptions.m_tlsCertificateChain.empty())
        tlsServer->useCertificateChain(clientOptions.m_tlsCertificateChain.c_str());
    else
        tlsServer->useCertificate(clientOptions.m_tlsCertificate.c_str());

    tlsServer->usePrivateKey(clientOptions.m_tlsPrivateKey.c_str());

    return tlsServer;
}

}  // namespace siodb::conn_worker
