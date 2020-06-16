// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Variant.h"

// Project headers
#include "lob/BinaryValueBlobStream.h"
#include "lob/BlobWrapperClobStream.h"
#include "lob/ClobWrapperBlobStream.h"
#include "lob/StringClobStream.h"

// Common project headers
#include <siodb/common/utils/PlainBinaryEncoding.h>

namespace siodb::iomgr::dbengine {

namespace {

// Error messages for exceptions.

constexpr const char* kInvalidStringValue = "invalid string value";
constexpr const char* kInvalidBinaryValue = "invalid binary value";
constexpr const char* kInvalidTimestamp = "invalid timestamp";
constexpr const char* kUnexpectedError = "unexpected error";
constexpr const char* kConvertedValueOutOfRange = "converted value is out of range";
constexpr const char* kBinaryValueIsTooLong = "binary value is too long";
constexpr const char* kClobIsTooLong = "CLOB value is too long";
constexpr const char* kBlobIsTooLong = "BLOB value is too long";

}  // anonymous namespace

bool Variant::asBool() const
{
    static constexpr VariantType kDestValueType = VariantType::kBool;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool;
        case VariantType::kInt8: return m_value.m_i8 != 0;
        case VariantType::kUInt8: return m_value.m_ui8 != 0;
        case VariantType::kInt16: return m_value.m_i16 != 0;
        case VariantType::kUInt16: return m_value.m_ui16 != 0;
        case VariantType::kInt32: return m_value.m_i32 != 0;
        case VariantType::kUInt32: return m_value.m_ui32 != 0;
        case VariantType::kInt64: return m_value.m_i64 != 0;
        case VariantType::kUInt64: return m_value.m_ui64 != 0;
        case VariantType::kFloat: return m_value.m_float != 0.0f;
        case VariantType::kDouble: return m_value.m_double != 0.0;
        case VariantType::kString: return stringToBool(*m_value.m_string);
        case VariantType::kBinary: return binaryToBool(*m_value.m_binary);
        case VariantType::kClob: return stringToBool(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToBool(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::int8_t Variant::asInt8() const
{
    static constexpr VariantType kDestValueType = VariantType::kInt8;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool ? 1 : 0;
        case VariantType::kInt8: return m_value.m_i8;
        case VariantType::kUInt8: return static_cast<std::int8_t>(m_value.m_ui8);
        case VariantType::kInt16: return static_cast<std::int8_t>(m_value.m_i16);
        case VariantType::kUInt16: return static_cast<std::int8_t>(m_value.m_ui16);
        case VariantType::kInt32: return static_cast<std::int8_t>(m_value.m_i32);
        case VariantType::kUInt32: return static_cast<std::int8_t>(m_value.m_ui32);
        case VariantType::kInt64: return static_cast<std::int8_t>(m_value.m_i64);
        case VariantType::kUInt64: return static_cast<std::int8_t>(m_value.m_ui64);
        case VariantType::kFloat: return static_cast<std::int8_t>(m_value.m_float);
        case VariantType::kDouble: return static_cast<std::int8_t>(m_value.m_double);
        case VariantType::kString: return stringToInt8(*m_value.m_string);
        case VariantType::kBinary: return binaryToInt8(*m_value.m_binary);
        case VariantType::kClob: return stringToInt8(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToInt8(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::int8_t Variant::asUInt8() const
{
    static constexpr VariantType kDestValueType = VariantType::kUInt8;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool ? 1 : 0;
        case VariantType::kInt8: return static_cast<std::uint8_t>(m_value.m_ui8);
        case VariantType::kUInt8: return m_value.m_ui8;
        case VariantType::kInt16: return static_cast<std::uint8_t>(m_value.m_i16);
        case VariantType::kUInt16: return static_cast<std::uint8_t>(m_value.m_ui16);
        case VariantType::kInt32: return static_cast<std::uint8_t>(m_value.m_i32);
        case VariantType::kUInt32: return static_cast<std::uint8_t>(m_value.m_ui32);
        case VariantType::kInt64: return static_cast<std::uint8_t>(m_value.m_i64);
        case VariantType::kUInt64: return static_cast<std::uint8_t>(m_value.m_ui64);
        case VariantType::kFloat: return static_cast<std::uint8_t>(m_value.m_float);
        case VariantType::kDouble: return static_cast<std::uint8_t>(m_value.m_double);
        case VariantType::kString: return stringToUInt8(*m_value.m_string);
        case VariantType::kBinary: return binaryToUInt8(*m_value.m_binary);
        case VariantType::kClob: return stringToUInt8(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToUInt8(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::int16_t Variant::asInt16() const
{
    static constexpr VariantType kDestValueType = VariantType::kInt16;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool ? 1 : 0;
        case VariantType::kInt8: return static_cast<std::int16_t>(m_value.m_i8);
        case VariantType::kUInt8: return static_cast<std::int16_t>(m_value.m_ui8);
        case VariantType::kInt16: return m_value.m_i16;
        case VariantType::kUInt16: return static_cast<std::int16_t>(m_value.m_ui16);
        case VariantType::kInt32: return static_cast<std::int16_t>(m_value.m_i32);
        case VariantType::kUInt32: return static_cast<std::int16_t>(m_value.m_ui32);
        case VariantType::kInt64: return static_cast<std::int16_t>(m_value.m_i64);
        case VariantType::kUInt64: return static_cast<std::int16_t>(m_value.m_ui64);
        case VariantType::kFloat: return static_cast<std::int16_t>(m_value.m_float);
        case VariantType::kDouble: return static_cast<std::int16_t>(m_value.m_double);
        case VariantType::kString: return stringToInt16(*m_value.m_string);
        case VariantType::kBinary: return binaryToInt16(*m_value.m_binary);
        case VariantType::kClob: return stringToInt16(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToInt16(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::uint16_t Variant::asUInt16() const
{
    static constexpr VariantType kDestValueType = VariantType::kUInt16;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool ? 1 : 0;
        case VariantType::kInt8: return static_cast<std::uint16_t>(m_value.m_ui8);
        case VariantType::kUInt8: return static_cast<std::uint16_t>(m_value.m_ui8);
        case VariantType::kInt16: return static_cast<std::uint16_t>(m_value.m_i16);
        case VariantType::kUInt16: return static_cast<std::uint16_t>(m_value.m_ui16);
        case VariantType::kInt32: return static_cast<std::uint16_t>(m_value.m_i32);
        case VariantType::kUInt32: return static_cast<std::uint16_t>(m_value.m_ui32);
        case VariantType::kInt64: return static_cast<std::uint16_t>(m_value.m_i64);
        case VariantType::kUInt64: return static_cast<std::uint16_t>(m_value.m_ui64);
        case VariantType::kFloat: return static_cast<std::uint16_t>(m_value.m_float);
        case VariantType::kDouble: return static_cast<std::uint16_t>(m_value.m_double);
        case VariantType::kString: return stringToUInt16(*m_value.m_string);
        case VariantType::kBinary: return binaryToUInt16(*m_value.m_binary);
        case VariantType::kClob: return stringToUInt16(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToUInt16(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::int32_t Variant::asInt32() const
{
    static constexpr VariantType kDestValueType = VariantType::kInt32;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool ? 1 : 0;
        case VariantType::kInt8: return static_cast<std::int32_t>(m_value.m_ui8);
        case VariantType::kUInt8: return static_cast<std::int32_t>(m_value.m_ui8);
        case VariantType::kInt16: return static_cast<std::int32_t>(m_value.m_i16);
        case VariantType::kUInt16: return static_cast<std::int32_t>(m_value.m_ui16);
        case VariantType::kInt32: return m_value.m_i32;
        case VariantType::kUInt32: return static_cast<std::int32_t>(m_value.m_ui32);
        case VariantType::kInt64: return static_cast<std::int32_t>(m_value.m_i64);
        case VariantType::kUInt64: return static_cast<std::int32_t>(m_value.m_ui64);
        case VariantType::kFloat: return static_cast<std::int32_t>(m_value.m_float);
        case VariantType::kDouble: return static_cast<std::int32_t>(m_value.m_double);
        case VariantType::kString: return stringToInt32(*m_value.m_string);
        case VariantType::kBinary: return binaryToInt32(*m_value.m_binary);
        case VariantType::kClob: return stringToInt32(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToInt32(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::uint32_t Variant::asUInt32() const
{
    static constexpr VariantType kDestValueType = VariantType::kUInt32;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool ? 1 : 0;
        case VariantType::kInt8: return static_cast<std::uint32_t>(m_value.m_ui8);
        case VariantType::kUInt8: return static_cast<std::uint32_t>(m_value.m_ui8);
        case VariantType::kInt16: return static_cast<std::uint32_t>(m_value.m_i16);
        case VariantType::kUInt16: return static_cast<std::uint32_t>(m_value.m_ui16);
        case VariantType::kInt32: return static_cast<std::uint32_t>(m_value.m_i32);
        case VariantType::kUInt32: return m_value.m_ui32;
        case VariantType::kInt64: return static_cast<std::uint32_t>(m_value.m_i64);
        case VariantType::kUInt64: return static_cast<std::uint32_t>(m_value.m_ui64);
        case VariantType::kFloat: return static_cast<std::uint32_t>(m_value.m_float);
        case VariantType::kDouble: return static_cast<std::uint32_t>(m_value.m_double);
        case VariantType::kString: return stringToUInt32(*m_value.m_string);
        case VariantType::kBinary: return binaryToUInt32(*m_value.m_binary);
        case VariantType::kClob: return stringToUInt32(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToUInt32(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::int64_t Variant::asInt64() const
{
    static constexpr VariantType kDestValueType = VariantType::kInt64;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool ? 1 : 0;
        case VariantType::kInt8: return static_cast<std::int64_t>(m_value.m_ui8);
        case VariantType::kUInt8: return static_cast<std::int64_t>(m_value.m_ui8);
        case VariantType::kInt16: return static_cast<std::int64_t>(m_value.m_i16);
        case VariantType::kUInt16: return static_cast<std::int64_t>(m_value.m_ui16);
        case VariantType::kInt32: return static_cast<std::int64_t>(m_value.m_i32);
        case VariantType::kUInt32: return static_cast<std::int64_t>(m_value.m_ui32);
        case VariantType::kInt64: return m_value.m_i64;
        case VariantType::kUInt64: return static_cast<std::int64_t>(m_value.m_ui64);
        case VariantType::kFloat: return static_cast<std::int64_t>(m_value.m_float);
        case VariantType::kDouble: return static_cast<std::int64_t>(m_value.m_double);
        case VariantType::kString: return stringToInt64(*m_value.m_string);
        case VariantType::kBinary: return binaryToInt64(*m_value.m_binary);
        case VariantType::kClob: return stringToInt64(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToInt64(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::uint64_t Variant::asUInt64() const
{
    static constexpr VariantType kDestValueType = VariantType::kUInt64;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool ? 1 : 0;
        case VariantType::kInt8: return static_cast<std::uint64_t>(m_value.m_ui8);
        case VariantType::kUInt8: return static_cast<std::uint64_t>(m_value.m_ui8);
        case VariantType::kInt16: return static_cast<std::uint64_t>(m_value.m_i16);
        case VariantType::kUInt16: return static_cast<std::uint64_t>(m_value.m_ui16);
        case VariantType::kInt32: return static_cast<std::uint64_t>(m_value.m_i32);
        case VariantType::kUInt32: return static_cast<std::uint64_t>(m_value.m_ui32);
        case VariantType::kInt64: return static_cast<std::uint64_t>(m_value.m_i64);
        case VariantType::kUInt64: return m_value.m_ui64;
        case VariantType::kFloat: return static_cast<std::uint64_t>(m_value.m_float);
        case VariantType::kDouble: return static_cast<std::uint64_t>(m_value.m_double);
        case VariantType::kString: return stringToUInt64(*m_value.m_string);
        case VariantType::kBinary: return binaryToUInt64(*m_value.m_binary);
        case VariantType::kClob: return stringToUInt64(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToUInt64(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

float Variant::asFloat() const
{
    static constexpr VariantType kDestValueType = VariantType::kFloat;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool ? 1.0f : 0.0f;
        case VariantType::kInt8: return static_cast<float>(m_value.m_ui8);
        case VariantType::kUInt8: return static_cast<float>(m_value.m_ui8);
        case VariantType::kInt16: return static_cast<float>(m_value.m_i16);
        case VariantType::kUInt16: return static_cast<float>(m_value.m_ui16);
        case VariantType::kInt32: return static_cast<float>(m_value.m_i32);
        case VariantType::kUInt32: return static_cast<float>(m_value.m_ui32);
        case VariantType::kInt64: return static_cast<float>(m_value.m_i64);
        case VariantType::kUInt64: return static_cast<float>(m_value.m_ui64);
        case VariantType::kFloat: return m_value.m_float;
        case VariantType::kDouble: return static_cast<float>(m_value.m_double);
        case VariantType::kString: return stringToFloat(*m_value.m_string);
        case VariantType::kBinary: return binaryToFloat(*m_value.m_binary);
        case VariantType::kClob: return stringToFloat(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToFloat(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

double Variant::asDouble() const
{
    static constexpr VariantType kDestValueType = VariantType::kDouble;
    switch (m_valueType) {
        case VariantType::kBool: return m_value.m_bool ? 1.0 : 0.0;
        case VariantType::kInt8: return static_cast<double>(m_value.m_ui8);
        case VariantType::kUInt8: return static_cast<double>(m_value.m_ui8);
        case VariantType::kInt16: return static_cast<double>(m_value.m_i16);
        case VariantType::kUInt16: return static_cast<double>(m_value.m_ui16);
        case VariantType::kInt32: return static_cast<double>(m_value.m_i32);
        case VariantType::kUInt32: return static_cast<double>(m_value.m_ui32);
        case VariantType::kInt64: return static_cast<double>(m_value.m_i64);
        case VariantType::kUInt64: return static_cast<double>(m_value.m_ui64);
        case VariantType::kFloat: return static_cast<double>(m_value.m_float);
        case VariantType::kDouble: return m_value.m_double;
        case VariantType::kString: return stringToDouble(*m_value.m_string);
        case VariantType::kBinary: return binaryToDouble(*m_value.m_binary);
        case VariantType::kClob: return stringToDouble(readClobAsString(kDestValueType));
        case VariantType::kBlob: return binaryToDouble(readBlobAsBinary(kDestValueType));
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

RawDateTime Variant::asDateTime(const char* format) const
{
    static constexpr VariantType kDestValueType = VariantType::kDateTime;
    switch (m_valueType) {
        case VariantType::kBool: return timestampToDateTime(m_value.m_bool ? 1 : 0);
        case VariantType::kInt8: return timestampToDateTime(m_value.m_i8);
        case VariantType::kUInt8: return timestampToDateTime(m_value.m_ui8);
        case VariantType::kInt16: return timestampToDateTime(m_value.m_i16);
        case VariantType::kUInt16: return timestampToDateTime(m_value.m_ui16);
        case VariantType::kInt32: return timestampToDateTime(m_value.m_i32);
        case VariantType::kUInt32: return timestampToDateTime(m_value.m_ui32);
        case VariantType::kInt64: return timestampToDateTime(m_value.m_i64);
        case VariantType::kUInt64: return timestampToDateTime(m_value.m_ui64);
        case VariantType::kFloat: {
            return timestampToDateTime(static_cast<std::time_t>(m_value.m_float));
        }
        case VariantType::kDouble: {
            return timestampToDateTime(static_cast<std::time_t>(m_value.m_double));
        }
        case VariantType::kDateTime: return *m_value.m_dt;
        case VariantType::kString: {
            return stringToDateTime(
                    *m_value.m_string, format ? format : getDateTimeFormat(*m_value.m_string));
        }
        case VariantType::kBinary: return binaryToDateTime(*m_value.m_binary);
        case VariantType::kClob: {
            auto str = readClobAsString(kDestValueType);
            return stringToDateTime(str, format ? format : getDateTimeFormat(str));
        }
        case VariantType::kBlob: {
            return binaryToDateTime(readBlobAsBinary(kDestValueType));
        }
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::pair<const std::string*, bool> Variant::asStringInternal(const char* format) const
{
    static constexpr VariantType kDestValueType = VariantType::kString;
    switch (m_valueType) {
        case VariantType::kBool: {
            return std::make_pair(
                    new std::string(m_value.m_bool ? kTrueString : kFalseString), false);
        }
        case VariantType::kInt8: {
            return std::make_pair(
                    new std::string(std::to_string(static_cast<std::int32_t>(m_value.m_i8))),
                    false);
        }
        case VariantType::kUInt8: {
            return std::make_pair(
                    new std::string(std::to_string(static_cast<std::uint32_t>(m_value.m_ui8))),
                    false);
        }
        case VariantType::kInt16: {
            return std::make_pair(
                    new std::string(std::to_string(static_cast<std::int32_t>(m_value.m_i16))),
                    false);
        }
        case VariantType::kUInt16: {
            return std::make_pair(
                    new std::string(std::to_string(static_cast<std::uint32_t>(m_value.m_ui16))),
                    false);
        }
        case VariantType::kInt32: {
            return std::make_pair(new std::string(std::to_string(m_value.m_i32)), false);
        }
        case VariantType::kUInt32: {
            return std::make_pair(new std::string(std::to_string(m_value.m_ui32)), false);
        }
        case VariantType::kInt64: {
            return std::make_pair(new std::string(std::to_string(m_value.m_i64)), false);
        }
        case VariantType::kUInt64: {
            return std::make_pair(new std::string(std::to_string(m_value.m_ui64)), false);
        }
        case VariantType::kFloat: {
            char buffer[128];
            std::snprintf(buffer, sizeof(buffer), "%.8f", m_value.m_float);
            return std::make_pair(new std::string(buffer), false);
        }
        case VariantType::kDouble: {
            char buffer[128];
            std::snprintf(buffer, sizeof(buffer), "%.8f", m_value.m_double);
            return std::make_pair(new std::string(buffer), false);
        }
        case VariantType::kDateTime: {
            auto s = m_value.m_dt->format(format ? format : kDefaultDateTimeFormat);
            return std::make_pair(new std::string(std::move(s)), false);
        }
        case VariantType::kString: return std::make_pair(m_value.m_string, true);
        case VariantType::kBinary: {
            return std::make_pair(
                    binaryToString(*m_value.m_binary, kDestValueType, kMaxStringValueLength),
                    false);
        }
        case VariantType::kClob: {
            return std::make_pair(new std::string(readClobAsString(kDestValueType)), false);
        }
        case VariantType::kBlob: {
            return std::make_pair(binaryToString(readBlobAsBinary(kDestValueType), kDestValueType,
                                          kMaxStringValueLength),
                    false);
        }
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::pair<const BinaryValue*, bool> Variant::asBinaryInternal() const
{
    static constexpr VariantType kDestValueType = VariantType::kBinary;
    switch (m_valueType) {
        case VariantType::kBool: {
            auto binaryData = new BinaryValue(1);
            binaryData->front() = m_value.m_bool ? 1 : 0;
            return std::make_pair(binaryData, false);
        }
        case VariantType::kInt8: {
            auto binaryData = new BinaryValue(1);
            binaryData->front() = m_value.m_i8;
            return std::make_pair(binaryData, false);
        }
        case VariantType::kUInt8: {
            auto binaryData = new BinaryValue(1);
            binaryData->front() = m_value.m_ui8;
            return std::make_pair(binaryData, false);
        }
        case VariantType::kInt16: {
            auto binaryData = new BinaryValue(2);
            ::pbeEncodeInt16(m_value.m_i16, binaryData->data());
            return std::make_pair(binaryData, false);
        }
        case VariantType::kUInt16: {
            auto binaryData = new BinaryValue(2);
            ::pbeEncodeUInt16(m_value.m_ui16, binaryData->data());
            return std::make_pair(binaryData, false);
        }
        case VariantType::kInt32: {
            auto binaryData = new BinaryValue(4);
            ::pbeEncodeInt32(m_value.m_i32, binaryData->data());
            return std::make_pair(binaryData, false);
        }
        case VariantType::kUInt32: {
            auto binaryData = new BinaryValue(4);
            ::pbeEncodeUInt32(m_value.m_ui32, binaryData->data());
            return std::make_pair(binaryData, false);
        }
        case VariantType::kInt64: {
            auto binaryData = new BinaryValue(8);
            ::pbeEncodeUInt64(m_value.m_i64, binaryData->data());
            return std::make_pair(binaryData, false);
        }
        case VariantType::kUInt64: {
            auto binaryData = new BinaryValue(8);
            ::pbeEncodeUInt64(m_value.m_ui64, binaryData->data());
            return std::make_pair(binaryData, false);
        }
        case VariantType::kFloat: {
            auto binaryData = new BinaryValue(4);
            ::pbeEncodeFloat(m_value.m_float, binaryData->data());
            return std::make_pair(binaryData, false);
        }
        case VariantType::kDouble: {
            auto binaryData = new BinaryValue(8);
            ::pbeEncodeDouble(m_value.m_float, binaryData->data());
            return std::make_pair(binaryData, false);
        }
        case VariantType::kDateTime: {
            std::uint8_t buffer[RawDateTime::kMaxSerializedSize];
            const auto p = m_value.m_dt->serialize(buffer);
            auto binaryData = new BinaryValue(buffer, p);
            return std::make_pair(binaryData, false);
        }
        case VariantType::kString: {
            return std::make_pair(stringToBinary(*m_value.m_string), false);
        }
        case VariantType::kBinary: return std::make_pair(m_value.m_binary, true);
        case VariantType::kClob: {
            return std::make_pair(stringToBinary(readClobAsString(kDestValueType)), false);
        }
        case VariantType::kBlob: {
            return std::make_pair(new BinaryValue(readBlobAsBinary(kDestValueType)), true);
        }
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::pair<const ClobStream*, bool> Variant::asClobInternal(const char* format) const
{
    static constexpr VariantType kDestValueType = VariantType::kClob;
    switch (m_valueType) {
        case VariantType::kBool:
        case VariantType::kInt8:
        case VariantType::kUInt8:
        case VariantType::kInt16:
        case VariantType::kUInt16:
        case VariantType::kInt32:
        case VariantType::kUInt32:
        case VariantType::kInt64:
        case VariantType::kUInt64:
        case VariantType::kFloat:
        case VariantType::kDouble:
        case VariantType::kDateTime:
        case VariantType::kString:
        case VariantType::kBinary: {
            const auto p = asStringInternal(format);
            if (p.second) {
                return std::make_pair(new StringClobStream(*p.first), false);
            } else {
                std::unique_ptr<std::string> s(stdext::as_mutable_ptr(p.first));
                return std::make_pair(new StringClobStream(std::move(*s)), false);
            }
        }
        case VariantType::kClob: return std::make_pair(m_value.m_clob, true);
        case VariantType::kBlob: {
            return std::make_pair(new BlobWrapperClobStream(m_value.m_blob), false);
        }
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

std::pair<const BlobStream*, bool> Variant::asBlobInternal() const
{
    static constexpr VariantType kDestValueType = VariantType::kBlob;
    switch (m_valueType) {
        case VariantType::kBool:
        case VariantType::kInt8:
        case VariantType::kUInt8:
        case VariantType::kInt16:
        case VariantType::kUInt16:
        case VariantType::kInt32:
        case VariantType::kUInt32:
        case VariantType::kInt64:
        case VariantType::kUInt64:
        case VariantType::kFloat:
        case VariantType::kDouble:
        case VariantType::kDateTime:
        case VariantType::kString:
        case VariantType::kBinary: {
            const auto p = asBinaryInternal();
            if (p.second) {
                return std::make_pair(new BinaryValueBlobStream(*p.first), false);
            } else {
                std::unique_ptr<BinaryValue> v(stdext::as_mutable_ptr(p.first));
                return std::make_pair(new BinaryValueBlobStream(std::move(*v)), false);
            }
        }
        case VariantType::kClob: {
            return std::make_pair(new ClobWrapperBlobStream(m_value.m_clob), false);
        }
        case VariantType::kBlob: return std::make_pair(m_value.m_blob, true);
        default: throw VariantTypeCastError(m_valueType, kDestValueType);
    }
}

bool Variant::stringToBool(const std::string& s) const
{
    if (s == kTrueString) return true;
    if (s == kFalseString) return false;
    throw VariantTypeCastError(m_valueType, VariantType::kBool, kInvalidStringValue);
}

std::int8_t Variant::stringToInt8(const std::string& s) const
{
    try {
        std::size_t pos = 0;
        const auto v = std::stoll(s, &pos, 0);
        if (pos != s.length()) throw std::invalid_argument(kInvalidStringValue);
        if (v < std::numeric_limits<std::int8_t>::min()
                || v < std::numeric_limits<std::int8_t>::max()) {
            throw std::out_of_range(kConvertedValueOutOfRange);
        }
        return static_cast<std::int8_t>(v);
    } catch (std::logic_error& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kInt8, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kInt8, kUnexpectedError);
    }
}

std::uint8_t Variant::stringToUInt8(const std::string& s) const
{
    try {
        std::size_t pos = 0;
        const auto v = std::stoull(s, &pos, 0);
        if (pos != s.length()) throw std::invalid_argument(kInvalidStringValue);
        if (v < std::numeric_limits<std::uint8_t>::min()
                || v < std::numeric_limits<std::uint8_t>::max()) {
            throw std::out_of_range(kConvertedValueOutOfRange);
        }
        return static_cast<std::uint8_t>(v);
    } catch (std::logic_error& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kUInt8, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kUInt8, kUnexpectedError);
    }
}

std::int16_t Variant::stringToInt16(const std::string& s) const
{
    try {
        std::size_t pos = 0;
        const auto v = std::stoll(s, &pos, 0);
        if (pos != s.length()) throw std::invalid_argument(kInvalidStringValue);
        if (v < std::numeric_limits<std::int16_t>::min()
                || v < std::numeric_limits<std::int16_t>::max()) {
            throw std::out_of_range(kConvertedValueOutOfRange);
        }
        return static_cast<std::int16_t>(v);
    } catch (std::logic_error& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kInt16, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kInt16, kUnexpectedError);
    }
}

std::uint16_t Variant::stringToUInt16(const std::string& s) const
{
    try {
        std::size_t pos = 0;
        const auto v = std::stoull(s, &pos, 0);
        if (pos != s.length()) throw std::invalid_argument(kInvalidStringValue);
        if (v < std::numeric_limits<std::uint16_t>::min()
                || v < std::numeric_limits<std::uint16_t>::max()) {
            throw std::out_of_range(kConvertedValueOutOfRange);
        }
        return static_cast<std::uint16_t>(v);
    } catch (std::logic_error& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kUInt16, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kUInt16, kUnexpectedError);
    }
}

std::int32_t Variant::stringToInt32(const std::string& s) const
{
    try {
        std::size_t pos = 0;
        const auto v = std::stoll(s, &pos, 0);
        if (pos != s.length()) throw std::invalid_argument(kInvalidStringValue);
        if (v < std::numeric_limits<std::int32_t>::min()
                || v < std::numeric_limits<std::int32_t>::max()) {
            throw std::out_of_range(kConvertedValueOutOfRange);
        }
        return static_cast<std::int32_t>(v);
    } catch (std::logic_error& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kInt32, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kInt32, kUnexpectedError);
    }
}

std::uint32_t Variant::stringToUInt32(const std::string& s) const
{
    try {
        std::size_t pos = 0;
        if (pos != s.length()) throw std::invalid_argument(kInvalidStringValue);
        const auto v = std::stoull(s, &pos, 0);
        if (v < std::numeric_limits<std::uint32_t>::min()
                || v < std::numeric_limits<std::uint32_t>::max()) {
            throw std::out_of_range(kConvertedValueOutOfRange);
        }
        return static_cast<std::uint32_t>(v);
    } catch (std::logic_error& ex) {  // 1x
        throw VariantTypeCastError(m_valueType, VariantType::kUInt32, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kUInt32, kUnexpectedError);
    }
}

std::int64_t Variant::stringToInt64(const std::string& s) const
{
    try {
        std::size_t pos = 0;
        const auto v = std::stoll(s, &pos, 0);
        if (pos != s.length()) throw std::invalid_argument(kInvalidStringValue);
        return v;
    } catch (std::logic_error& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kInt64, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kInt64, kUnexpectedError);
    }
}

std::uint64_t Variant::stringToUInt64(const std::string& s) const
{
    try {
        std::size_t pos = 0;
        const auto v = std::stoull(s, &pos, 0);
        if (pos != s.length()) throw std::invalid_argument(kInvalidStringValue);
        return v;
    } catch (std::logic_error& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kUInt64, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kUInt64, kUnexpectedError);
    }
}

float Variant::stringToFloat(const std::string& s) const
{
    try {
        std::size_t pos = 0;
        const auto v = std::stof(s, &pos);
        if (pos != s.length()) throw std::invalid_argument(kInvalidStringValue);
        return v;
    } catch (std::logic_error& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kFloat, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kFloat, kUnexpectedError);
    }
}

double Variant::stringToDouble(const std::string& s) const
{
    try {
        std::size_t pos = 0;
        const auto v = std::stod(s, &pos);
        if (pos != s.length()) throw std::invalid_argument(kInvalidStringValue);
        return v;
    } catch (std::logic_error& ex) {  // 1x
        throw VariantTypeCastError(m_valueType, VariantType::kDouble, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kDouble, kUnexpectedError);
    }
}

RawDateTime Variant::stringToDateTime(const char* s, std::size_t len, const char* format) const
{
    RawDateTime dateTime;
    try {
        dateTime.parse(s, len, format);
    } catch (std::logic_error& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kDateTime, ex.what());
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, VariantType::kDateTime, kUnexpectedError);
    }
    return dateTime;
}

bool Variant::binaryToBool(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 1)
        throw VariantTypeCastError(m_valueType, VariantType::kBool, kInvalidBinaryValue);
    return binaryValue.front() != 0;
}

std::int8_t Variant::binaryToInt8(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 1)
        throw VariantTypeCastError(m_valueType, VariantType::kInt8, kInvalidBinaryValue);
    return static_cast<std::int8_t>(binaryValue.front());
}

std::uint8_t Variant::binaryToUInt8(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 1)
        throw VariantTypeCastError(m_valueType, VariantType::kUInt8, kInvalidBinaryValue);
    return binaryValue.front();
}

std::int16_t Variant::binaryToInt16(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 2)
        throw VariantTypeCastError(m_valueType, VariantType::kInt16, kInvalidBinaryValue);
    std::int16_t result = 0;
    ::pbeDecodeInt16(binaryValue.data(), &result);
    return result;
}

std::uint16_t Variant::binaryToUInt16(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 2) {
        throw VariantTypeCastError(m_valueType, VariantType::kUInt16, kInvalidBinaryValue);
    }
    std::uint16_t result = 0;
    ::pbeDecodeUInt16(binaryValue.data(), &result);
    return result;
}

std::int32_t Variant::binaryToInt32(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 4)
        throw VariantTypeCastError(m_valueType, VariantType::kInt32, kInvalidBinaryValue);
    std::int32_t result = 0;
    ::pbeDecodeInt32(binaryValue.data(), &result);
    return result;
}

std::uint32_t Variant::binaryToUInt32(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 4) {
        throw VariantTypeCastError(m_valueType, VariantType::kUInt32, kInvalidBinaryValue);
    }
    std::uint32_t result = 0;
    ::pbeDecodeUInt32(binaryValue.data(), &result);
    return result;
}

std::int64_t Variant::binaryToInt64(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 8)
        throw VariantTypeCastError(m_valueType, VariantType::kInt64, kInvalidBinaryValue);
    std::int64_t result = 0;
    ::pbeDecodeInt64(binaryValue.data(), &result);
    return result;
}

std::uint64_t Variant::binaryToUInt64(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 8) {
        throw VariantTypeCastError(m_valueType, VariantType::kUInt64, kInvalidBinaryValue);
    }
    std::uint64_t result = 0;
    ::pbeDecodeUInt64(binaryValue.data(), &result);
    return result;
}

float Variant::binaryToFloat(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 4)
        throw VariantTypeCastError(m_valueType, VariantType::kFloat, kInvalidBinaryValue);
    float result = 0.0f;
    ::pbeDecodeFloat(binaryValue.data(), &result);
    return result;
}

double Variant::binaryToDouble(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < 8)
        throw VariantTypeCastError(m_valueType, VariantType::kFloat, kInvalidBinaryValue);
    double result = 0.0;
    ::pbeDecodeDouble(binaryValue.data(), &result);
    return result;
}

std::string* Variant::binaryToString(const BinaryValue& binaryValue, VariantType destValueType,
        std::size_t maxOutputLength) const
{
    if (binaryValue.empty()) return new std::string();

    if (binaryValue.size() * 2 > maxOutputLength)
        throw VariantTypeCastError(m_valueType, destValueType, kBinaryValueIsTooLong);

    std::vector<char> buffer(binaryValue.size() * 2 + 1);
    auto pb = buffer.data();
    for (auto p = binaryValue.data(), e = p + binaryValue.size(); p != e; ++p) {
        const unsigned char v = *p;
        *pb++ = m_hexConversionTable[v >> 4];
        *pb++ = m_hexConversionTable[v & 15];
    }
    *pb = '\0';
    return new std::string(buffer.data());
}

RawDateTime Variant::binaryToDateTime(const BinaryValue& binaryValue) const
{
    if (binaryValue.size() < RawDateTime::kDatePartSerializedSize) {
        throw VariantTypeCastError(m_valueType, VariantType::kDateTime, kInvalidBinaryValue);
    }

    RawDateTime result;
    result.deserializeDatePart(binaryValue.data());
    if (result.m_datePart.m_hasTimePart) {
        if (binaryValue.size() < RawDateTime::kMaxSerializedSize) {
            throw VariantTypeCastError(m_valueType, VariantType::kDateTime, kInvalidBinaryValue);
        }
        result.deserialize(binaryValue.data(), binaryValue.size());
    }
    return result;
}

RawDateTime Variant::timestampToDateTime(std::time_t timestamp) const
{
    struct tm tm;
    if (!gmtime_r(&timestamp, &tm)) {
        throw VariantTypeCastError(m_valueType, VariantType::kDateTime, kInvalidTimestamp);
    }
    return tmToDateTime(tm);
}

RawDateTime Variant::tmToDateTime(const std::tm& tm) const
{
    const int year = tm.tm_year + 1900;
    if (year < RawDate::kMinYear || year > RawDate::kMaxYear) {
        throw VariantTypeCastError(m_valueType, VariantType::kDateTime, kInvalidTimestamp);
    }
    RawDateTime result;
    result.m_timePart.m_nanos = 0;
    result.m_timePart.m_seconds = tm.tm_sec;
    result.m_timePart.m_minutes = tm.tm_min;
    result.m_timePart.m_hours = tm.tm_hour;
    result.m_datePart.m_dayOfMonth = tm.tm_mday;
    result.m_datePart.m_month = tm.tm_mon;
    result.m_datePart.m_year = year;
    result.m_datePart.m_dayOfWeek = tm.tm_wday;
    return result;
}

std::string Variant::readClobAsString(VariantType destValueType) const
{
    const auto clobSize = m_value.m_clob->getRemainingSize();
    if (clobSize > kMaxStringValueLength)
        throw VariantTypeCastError(m_valueType, destValueType, kClobIsTooLong);

    try {
        return m_value.m_clob->readAsString(clobSize);
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, destValueType, ex.what());
    }
}

BinaryValue Variant::readBlobAsBinary(VariantType destValueType) const
{
    const auto blobSize = m_value.m_blob->getRemainingSize();
    if (blobSize > kMaxBinaryValueLength)
        throw VariantTypeCastError(m_valueType, destValueType, kBlobIsTooLong);
    try {
        return m_value.m_blob->readAsBinary(blobSize);
    } catch (std::exception& ex) {
        throw VariantTypeCastError(m_valueType, destValueType, ex.what());
    }
}

}  // namespace siodb::iomgr::dbengine
