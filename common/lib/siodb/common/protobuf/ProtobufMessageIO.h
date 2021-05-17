// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ProtocolError.h"
#include "ProtocolMessageType.h"
#include "StreamInputStream.h"
#include "StreamOutputStream.h"

// Protobuf headers
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/message_lite.h>

namespace siodb::protobuf {

/** Siodb protocol message object factory interface. */
class ProtocolMessageFactory {
public:
    /** De-initializes object of class ProtobufMessageFactory. */
    virtual ~ProtocolMessageFactory() = default;

    /**
     * Creates new protocol message object of a designated type.
     * @param messageType Message type.
     * @return New message object.
     */
    virtual google::protobuf::MessageLite* createMessage(ProtocolMessageType messageType) = 0;
};

/**
 * Reads one of protocol messages from a stream.
 * @param messageTypes List of expected message types.
 * @param messageTypeCount Number of message types in the list.
 * @param input Input stream.
 * @param messageFactory Message factory.
 * @param errorCodeChecker I/O error code checker object.
 * @return Consumed message.
 * @throw std::system_error when I/O error happens.
 * @throw ProtocolError when protocol error happens.
 */
std::unique_ptr<google::protobuf::MessageLite> readMessage(const ProtocolMessageType* messageTypes,
        std::size_t messageTypeCount, io::InputStream& input,
        ProtocolMessageFactory& messageFactory,
        const utils::ErrorCodeChecker& errorCodeChecker = utils::DefaultErrorCodeChecker());

/**
 * Reads one of protocol messages from a stream.
 * @param messageTypes List of expected message types.
 * @param messageTypeCount Number of message types in the list.
 * @param input Input stream.
 * @param messageFactory Message factory.
 * @return Consumed message.
 * @throw std::system_error when I/O error happens.
 * @throw ProtocolError when protocol error happens.
 */
std::unique_ptr<google::protobuf::MessageLite> readMessage(const ProtocolMessageType* messageTypes,
        std::size_t messageTypeCount, StreamInputStream& input,
        ProtocolMessageFactory& messageFactory);

/**
 * Reads protocol message from a stream.
 * @param messageType Message type.
 * @param message A message.
 * @param input Input stream.
 * @param errorCodeChecker I/O error code checker object.
 * @throw std::system_error when I/O error happens.
 * @throw ProtocolError when protocol error happens.
 */
void readMessage(ProtocolMessageType messageType, google::protobuf::MessageLite& message,
        io::InputStream& input,
        const utils::ErrorCodeChecker& errorCodeChecker = utils::DefaultErrorCodeChecker());

/**
 * Reads protocol message from a stream.
 * @param messageType Message type.
 * @param message A message.
 * @param input Input stream.
 * @throw std::system_error when I/O error happens.
 * @throw ProtocolError when protocol error happens.
 */
void readMessage(ProtocolMessageType messageType, google::protobuf::MessageLite& message,
        StreamInputStream& input);

/**
 * Writes protocol message to a stream.
 * @param messageType message type identifier.
 * @param message a message.
 * @param output Output stream.
 * @param errorCodeChecker I/O error code checker object.
 * @throw std::system_error when I/O error happens.
 * @throw ProtocolError when protocol error happens.
 */
void writeMessage(ProtocolMessageType messageType, const google::protobuf::MessageLite& message,
        io::OutputStream& output,
        const utils::ErrorCodeChecker& errorCodeChecker = utils::DefaultErrorCodeChecker());

/**
 * Writes protocol message to a stream.
 * @param messageType message type identifier
 * @param message a message
 * @param output Output stream.
 * @throw std::system_error when I/O error happens.
 * @throw ProtocolError when protocol error happens.
 */
void writeMessage(ProtocolMessageType messageType, const google::protobuf::MessageLite& message,
        StreamOutputStream& output);

/**
 * Writes protocol message to a coded stream.
 * @param messageType message type identifier
 * @param message a message
 * @param output Output stream.
 * @param codedOutput Coded output stream, linked with "output".
 * @throw std::system_error when I/O error happens.
 * @throw ProtocolError when protocol error happens.
 */
void writeMessage(ProtocolMessageType messageType, const google::protobuf::MessageLite& message,
        StreamOutputStream& output, google::protobuf::io::CodedOutputStream& codedOutput);

}  // namespace siodb::protobuf
