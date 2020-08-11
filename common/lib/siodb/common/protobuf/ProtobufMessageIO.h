// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "CustomProtobufInputStream.h"
#include "CustomProtobufOutputStream.h"
#include "SiodbProtocolError.h"
#include "SiodbProtocolMessageType.h"
#include "../io/IODevice.h"

// STL headers
#include <system_error>

// Protobuf headers
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message_lite.h>

namespace siodb::protobuf {

/**
 * Reads protobuf message from an IO stream.
 * @param messageType Message type identifier.
 * @param message A message.
 * @param device Input device.
 * @param errorCodeChecker I/O error code checker object.
 * @throw std::system_error when I/O error happens.
 * @throw SiodbProtocolError when protocol error happens.
 */
void readMessage(ProtocolMessageType messageType, google::protobuf::MessageLite& message,
        io::IODevice& device,
        const utils::ErrorCodeChecker& errorCodeChecker = utils::DefaultErrorCodeChecker());

/**
 * Reads protobuf message from an IO stream.
 * @param messageType message type identifier
 * @param message a message
 * @param inputStream Input stream with a message
 * @throw std::system_error when I/O error happens.
 * @throw SiodbProtocolError when protocol error happens.
 */
void readMessage(ProtocolMessageType messageType, google::protobuf::MessageLite& message,
        CustomProtobufInputStream& inputStream);

/**
 * Writes protobuf message to an IO stream.
 * @param messageType message type identifier.
 * @param message a message.
 * @param device Output device.
 * @param errorCodeChecker I/O error code checker object.
 * @throw std::system_error when I/O error happens.
 * @throw SiodbProtocolError when protocol error happens.
 */
void writeMessage(ProtocolMessageType messageType, const google::protobuf::MessageLite& message,
        io::IODevice& device,
        const utils::ErrorCodeChecker& errorCodeChecker = utils::DefaultErrorCodeChecker());

/**
 * Writes protobuf message to an IO stream.
 * @param messageType message type identifier
 * @param message a message
 * @param io Output stream.
 * @throw std::system_error when I/O error happens.
 * @throw SiodbProtocolError when protocol error happens.
 */
void writeMessage(ProtocolMessageType messageType, const google::protobuf::MessageLite& message,
        CustomProtobufOutputStream& rawOutput);

/**
 * Reports protobuf stream read error.
 * @param errorCode errno compatible error code
 * @param message Custom error description. if nullprt, standard one is used.
 * @throw std::system_error contains I/O error description.
 */
void reportStreamReadError(int errorCode, const char* message = nullptr);

/**
 * Reports protobuf stream write error.
 * @param errorCode errno compatible error code
 * @param message Custom error description. if nullprt, standard one is used.
 * @throw std::system_error contains I/O error description.
 */
void reportStreamWriteError(int errorCode, const char* message = nullptr);

/**
 * Checks input stream state, reports error via exception, if error detected.
 * @param rawStream Associated raw stream.
 */
void checkInputStreamError(const CustomProtobufInputStream& rawStream);

/**
 * Checks output stream state, reports error via exception, if error detected.
 * @param rawStream Associated raw stream.
 */
void checkOutputStreamError(const CustomProtobufOutputStream& rawStream);

}  // namespace siodb::protobuf
