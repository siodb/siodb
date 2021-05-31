// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "VariantOutput.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "RequestHandlerSharedConstants.h"
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/protobuf/RawDateTimeIO.h>

// STL headers
#include <algorithm>

// Boost headers
#include <boost/algorithm/hex.hpp>

namespace siodb::iomgr::dbengine {

std::size_t getSerializedSize(const Variant& value)
{
    switch (value.getValueType()) {
        case VariantType::kNull: return 0;
        case VariantType::kBool:
        case VariantType::kInt8:
        case VariantType::kUInt8: return 1;
        case VariantType::kInt16:
        case VariantType::kUInt16: return 2;
        case VariantType::kInt32:
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getInt32());
        case VariantType::kUInt32:
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getUInt32());
        case VariantType::kInt64:
            return google::protobuf::io::CodedOutputStream::VarintSize64(value.getInt64());
        case VariantType::kUInt64:
            return google::protobuf::io::CodedOutputStream::VarintSize64(value.getUInt64());
        case VariantType::kFloat: return 4;
        case VariantType::kDouble: return 8;
        case VariantType::kDateTime: return value.getDateTime().getSerializedSize();
        case VariantType::kString: {
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getString().size())
                   + value.getString().size();
        }
        case VariantType::kBinary: {
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getBinary().size())
                   + value.getBinary().size();
        }
        case VariantType::kClob: {
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getClob().getSize())
                   + value.getClob().getSize();
        }
        case VariantType::kBlob: {
            return google::protobuf::io::CodedOutputStream::VarintSize32(value.getBlob().getSize())
                   + value.getBlob().getSize();
        }
        default: {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidValueType,
                    static_cast<int>(value.getValueType()));
        }
    }
}

void writeVariant(const Variant& value, protobuf::ExtendedCodedOutputStream& codedOutput)
{
    switch (value.getValueType()) {
        case VariantType::kNull: break;
        case VariantType::kBool: {
            codedOutput.Write(value.getBool());
            break;
        }
        case VariantType::kInt8: {
            codedOutput.Write(value.getInt8());
            break;
        }
        case VariantType::kUInt8: {
            codedOutput.Write(value.getUInt8());
            break;
        }
        case VariantType::kInt16: {
            codedOutput.Write(value.getInt16());
            break;
        }
        case VariantType::kUInt16: {
            codedOutput.Write(value.getUInt16());
            break;
        }
        case VariantType::kInt32: {
            codedOutput.Write(value.getInt32());
            break;
        }
        case VariantType::kUInt32: {
            codedOutput.Write(value.getUInt32());
            break;
        }
        case VariantType::kInt64: {
            codedOutput.Write(value.getInt64());
            break;
        }
        case VariantType::kUInt64: {
            codedOutput.Write(value.getUInt64());
            break;
        }
        case VariantType::kFloat: {
            codedOutput.Write(value.getFloat());
            break;
        }
        case VariantType::kDouble: {
            codedOutput.Write(value.getDouble());
            break;
        }
        case VariantType::kDateTime: {
            protobuf::writeRawDateTime(codedOutput, value.getDateTime());
            break;
        }
        case VariantType::kString: {
            codedOutput.Write(value.getString());
            break;
        }
        case VariantType::kBinary: {
            codedOutput.Write(value.getBinary());
            break;
        }
        case VariantType::kClob: {
            std::unique_ptr<ClobStream> clob(value.getClob().clone());
            auto size = clob->getRemainingSize();
            stdext::buffer<std::uint8_t> buffer(std::min(size, kLobChunkSize));
            codedOutput.WriteVarint32(size);
            while (size > 0) {
                auto chunkSize = std::min(size, kLobChunkSize);
                chunkSize = clob->read(buffer.data(), chunkSize);
                size -= chunkSize;
                codedOutput.WriteRaw(buffer.data(), chunkSize);
            }
            break;
        }
        case VariantType::kBlob: {
            std::unique_ptr<BlobStream> blob(value.getBlob().clone());
            auto size = blob->getRemainingSize();
            stdext::buffer<std::uint8_t> buffer(std::min(size, kLobChunkSize));
            codedOutput.WriteVarint32(size);
            while (size > 0) {
                auto chunkSize = std::min(size, kLobChunkSize);
                chunkSize = blob->read(buffer.data(), chunkSize);
                size -= chunkSize;
                codedOutput.WriteRaw(buffer.data(), chunkSize);
            }
            break;
        }
        default: {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidValueType,
                    static_cast<int>(value.getValueType()));
        }
    }
}

void writeVariant(const Variant& value, siodb::io::JsonWriter& jsonWriter)
{
    switch (value.getValueType()) {
        case VariantType::kNull: {
            jsonWriter.writeNullValue();
            break;
        }
        case VariantType::kBool: {
            jsonWriter.writeValue(value.getBool());
            break;
        }
        case VariantType::kInt8: {
            jsonWriter.writeValue(value.getInt8());
            break;
        }
        case VariantType::kUInt8: {
            jsonWriter.writeValue(value.getUInt8());
            break;
        }
        case VariantType::kInt16: {
            jsonWriter.writeValue(value.getInt16());
            break;
        }
        case VariantType::kUInt16: {
            jsonWriter.writeValue(value.getUInt16());
            break;
        }
        case VariantType::kInt32: {
            jsonWriter.writeValue(value.getInt32());
            break;
        }
        case VariantType::kUInt32: {
            jsonWriter.writeValue(value.getUInt32());
            break;
        }
        case VariantType::kInt64: {
            jsonWriter.writeValue(value.getInt64());
            break;
        }
        case VariantType::kUInt64: {
            jsonWriter.writeValue(value.getUInt64());
            break;
        }
        case VariantType::kFloat: {
            jsonWriter.writeValue(value.getFloat());
            break;
        }
        case VariantType::kDouble: {
            jsonWriter.writeValue(value.getDouble());
            break;
        }
        case VariantType::kDateTime: {
            // TODO HERE and forward
            jsonWriter.writeValue(value.getDateTime().formatDefault());
            break;
        }
        case VariantType::kString: {
            jsonWriter.writeValue(value.getString());
            break;
        }
        case VariantType::kBinary: {
            jsonWriter.writeValue(*value.asString());
            break;
        }
        case VariantType::kClob: {
            std::unique_ptr<ClobStream> clob(value.getClob().clone());
            auto size = clob->getRemainingSize();
            stdext::buffer<std::uint8_t> buffer(std::min(size, kLobChunkSize));
            jsonWriter.writeDoubleQuote();
            while (size > 0) {
                auto chunkSize = std::min(size, kLobChunkSize);
                chunkSize = clob->read(buffer.data(), chunkSize);
                size -= chunkSize;
                if (SIODB_LIKELY(chunkSize > 0)) {
                    jsonWriter.writeRawString(
                            reinterpret_cast<const char*>(buffer.data()), chunkSize);
                }
            }
            jsonWriter.writeDoubleQuote();
            break;
        }
        case VariantType::kBlob: {
            std::unique_ptr<BlobStream> blob(value.getBlob().clone());
            auto size = blob->getRemainingSize();
            stdext::buffer<std::uint8_t> buffer(std::min(size, kLobChunkSize));
            stdext::buffer<char> hexBuffer(buffer.size() * 2);
            jsonWriter.writeDoubleQuote();
            while (size > 0) {
                auto chunkSize = std::min(size, kLobChunkSize);
                chunkSize = blob->read(buffer.data(), chunkSize);
                size -= chunkSize;
                if (SIODB_LIKELY(chunkSize > 0)) {
                    boost::algorithm::hex_lower(buffer.begin(), buffer.end(), hexBuffer.begin());
                    jsonWriter.writeBytes(hexBuffer.data(), chunkSize * 2);
                }
            }
            jsonWriter.writeDoubleQuote();
            break;
        }
        default: {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidValueType,
                    static_cast<int>(value.getValueType()));
        }
    }
}

}  // namespace siodb::iomgr::dbengine
