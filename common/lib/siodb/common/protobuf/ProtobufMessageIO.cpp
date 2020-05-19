// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ProtobufMessageIO.h"

// Project headers
#include "CustomProtobufOutputStream.h"
#include "../stl_ext/utility_ext.h"
#include "../utils/SystemError.h"

// CRT headers
#include <cstring>

// STL headers
#include <sstream>

namespace siodb::protobuf {

void readMessage(ProtocolMessageType messageType, google::protobuf::MessageLite& message,
        io::IoBase& io, const utils::ErrorCodeChecker& errorCodeChecker)
{
    // Create input streams
    CustomProtobufInputStream rawInput(io, errorCodeChecker);
    readMessage(messageType, message, rawInput);
}

void readMessage(ProtocolMessageType messageType, google::protobuf::MessageLite& message,
        CustomProtobufInputStream& inputStream)
{
    // Read metadata
    {
        std::uint32_t messageTypeId = 0;
        google::protobuf::io::CodedInputStream codedInput(&inputStream);

        // Read message type
        if (!codedInput.ReadVarint32(&messageTypeId)) {
            checkInputStreamError(inputStream);
        }
        if (messageTypeId >= stdext::underlying_value(ProtocolMessageType::kMax)) {
            std::ostringstream err;
            err << "Protocol error: Unsupported message type " << messageTypeId;
            throw SiodbProtocolError(err.str());
        }
        if (messageTypeId != stdext::underlying_value(messageType)) {
            std::ostringstream err;
            err << "Protocol error: Unexpected message type " << messageTypeId
                << " while waiting for " << stdext::underlying_value(messageType);
            throw SiodbProtocolError(err.str());
        }
    }

    // Read message
    {
        google::protobuf::io::CodedInputStream codedInput(&inputStream);
        const auto limit = codedInput.ReadLengthAndPushLimit();
        if (limit == 0) {
            throw SiodbProtocolError("Protocol error: can't read message size");
        }
        if (!message.ParseFromCodedStream(&codedInput)) {
            checkInputStreamError(inputStream);
        }
    }
}

void writeMessage(ProtocolMessageType messageType, const google::protobuf::MessageLite& message,
        io::IoBase& io, const utils::ErrorCodeChecker& errorCodeChecker)
{
    // Create output streams
    CustomProtobufOutputStream rawOutput(io, errorCodeChecker);
    writeMessage(messageType, message, rawOutput);
}

void writeMessage(ProtocolMessageType messageType, const google::protobuf::MessageLite& message,
        CustomProtobufOutputStream& rawOutput)
{
    google::protobuf::io::CodedOutputStream codedOutput(&rawOutput);

    // Write message type
    const auto messageTypeId = stdext::underlying_value(messageType);
    codedOutput.WriteVarint32(messageTypeId);
    checkOutputStreamError(rawOutput);

    // Write message size
    const auto messageSize = message.ByteSizeLong();
    codedOutput.WriteVarint32(static_cast<int>(messageSize));
    checkOutputStreamError(rawOutput);

    // Write message itself
    message.SerializeToCodedStream(&codedOutput);

    checkOutputStreamError(rawOutput);
}

void reportStreamReadError(int errorCode, const char* message)
{
    if (message == nullptr) {
        message = "Read error";
    }
    utils::throwSystemError(errorCode, message);
}

void reportStreamWriteError(int errorCode, const char* message)
{
    if (message == nullptr) {
        message = "Write error";
    }
    utils::throwSystemError(errorCode, message);
}

void checkInputStreamError(const CustomProtobufInputStream& rawStream)
{
    const int errorCode = rawStream.GetErrno();
    if (errorCode != 0) {
        reportStreamReadError(errorCode);
    } else {
        throw SiodbProtocolError("Protocol error: Protobuf input error");
    }
}

void checkOutputStreamError(const CustomProtobufOutputStream& rawStream)
{
    const int errorCode = rawStream.GetErrno();
    if (errorCode != 0) reportStreamWriteError(errorCode);
}

}  // namespace siodb::protobuf
