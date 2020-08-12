// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ProtobufMessageIO.h"

// Project headers
#include "CustomProtobufOutputStream.h"
#include "../stl_ext/system_error_ext.h"
#include "../stl_ext/utility_ext.h"

// CRT headers
#include <cstring>

// STL headers
#include <sstream>

namespace siodb::protobuf {

namespace {

void reportInputStreamError(const CustomProtobufInputStream& stream)
{
    const int errorCode = stream.GetErrno();
    if (errorCode != 0)
        stdext::throw_system_error(errorCode, "Read error");
    else
        throw SiodbProtocolError("Protocol error: Protobuf message decoding error");
}

ProtocolMessageType readMessageType(const ProtocolMessageType* messageTypes,
        std::size_t messageTypeCount, CustomProtobufInputStream& input)
{
    std::uint32_t messageTypeId = 0;
    google::protobuf::io::CodedInputStream codedInput(&input);

    if (!codedInput.ReadVarint32(&messageTypeId)) reportInputStreamError(input);

    if (messageTypeId >= static_cast<std::uint32_t>(ProtocolMessageType::kMax)) {
        std::ostringstream err;
        err << "Protocol error: Unsupported message type " << messageTypeId;
        throw SiodbProtocolError(err.str());
    }

    if (std::find_if(messageTypes, messageTypes + messageTypeCount,
                [messageTypeId](auto messageType) noexcept {
                    return messageTypeId == static_cast<std::uint32_t>(messageType);
                })
            == messageTypes + messageTypeCount) {
        std::ostringstream err;
        err << "Protocol error: Unexpected message type " << messageTypeId
            << " while waiting for one of: ";
        for (std::size_t i = 0; i < messageTypeCount; ++i) {
            if (i > 0) err << ", ";
            err << static_cast<std::uint32_t>(messageTypes[i]);
        }
        throw SiodbProtocolError(err.str());
    }

    return static_cast<ProtocolMessageType>(messageTypeId);
}

}  // namespace

std::unique_ptr<google::protobuf::MessageLite> readMessage(const ProtocolMessageType* messageTypes,
        std::size_t messageTypeCount, io::IODevice& input, ProtocolMessageFactory& messageFactory,
        const utils::ErrorCodeChecker& errorCodeChecker)
{
    CustomProtobufInputStream rawInput(input, errorCodeChecker);
    return readMessage(messageTypes, messageTypeCount, rawInput, messageFactory);
}

void readMessage(ProtocolMessageType messageType, google::protobuf::MessageLite& message,
        io::IODevice& input, const utils::ErrorCodeChecker& errorCodeChecker)
{
    CustomProtobufInputStream rawInput(input, errorCodeChecker);
    readMessage(messageType, message, rawInput);
}

std::unique_ptr<google::protobuf::MessageLite> readMessage(const ProtocolMessageType* messageTypes,
        std::size_t messageTypeCount, CustomProtobufInputStream& input,
        ProtocolMessageFactory& messageFactory)
{
    std::unique_ptr<google::protobuf::MessageLite> message(
            messageFactory.createMessage(readMessageType(messageTypes, messageTypeCount, input)));
    google::protobuf::io::CodedInputStream codedInput(&input);
    const auto limit = codedInput.ReadLengthAndPushLimit();
    if (limit == 0) throw SiodbProtocolError("Protocol error: can't read message size");
    if (!message->ParseFromCodedStream(&codedInput)) reportInputStreamError(input);
    return message;
}

void readMessage(ProtocolMessageType messageType, google::protobuf::MessageLite& message,
        CustomProtobufInputStream& input)
{
    readMessageType(&messageType, 1, input);
    google::protobuf::io::CodedInputStream codedInput(&input);
    const auto limit = codedInput.ReadLengthAndPushLimit();
    if (limit == 0) throw SiodbProtocolError("Protocol error: can't read message size");
    if (!message.ParseFromCodedStream(&codedInput)) reportInputStreamError(input);
}

void writeMessage(ProtocolMessageType messageType, const google::protobuf::MessageLite& message,
        io::IODevice& output, const utils::ErrorCodeChecker& errorCodeChecker)
{
    CustomProtobufOutputStream rawOutput(output, errorCodeChecker);
    writeMessage(messageType, message, rawOutput);
}

void writeMessage(ProtocolMessageType messageType, const google::protobuf::MessageLite& message,
        CustomProtobufOutputStream& output)
{
    google::protobuf::io::CodedOutputStream codedOutput(&output);

    // Write message type
    codedOutput.WriteVarint32(static_cast<std::uint32_t>(messageType));
    checkOutputStreamError(output);

    // Write message size
    const auto messageSize = message.ByteSizeLong();
    codedOutput.WriteVarint32(static_cast<int>(messageSize));
    checkOutputStreamError(output);

    // Write message itself
    message.SerializeToCodedStream(&codedOutput);

    checkOutputStreamError(output);
}

void checkOutputStreamError(const CustomProtobufOutputStream& stream)
{
    const int errorCode = stream.GetErrno();
    if (errorCode != 0) stdext::throw_system_error(errorCode, "Write error");
}

}  // namespace siodb::protobuf
