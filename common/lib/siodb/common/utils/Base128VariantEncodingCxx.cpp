// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Base128VariantEncoding.h"

// CRT headers
#include <cstring>

using siodb::utils::DeserializationError;

std::uint8_t* serializeUnchecked(const char* s, std::uint8_t* buffer) noexcept
{
    return serializeUnchecked(s, std::strlen(s), buffer);
}

std::uint8_t* serializeUnchecked(const char* s, std::size_t length, std::uint8_t* buffer) noexcept
{
    buffer = ::encodeVarInt(length, buffer);
    if (SIODB_UNLIKELY(length == 0)) return buffer;
    std::memcpy(buffer, s, length);
    return buffer + length;
}

std::size_t deserializeObject(const std::uint8_t* buffer, std::size_t length, std::string& s)
{
    std::uint64_t len = 0;
    const int consumed = ::decodeVarInt(buffer, length, len);
    if (SIODB_UNLIKELY(consumed < 0)) throw DeserializationError("Corrupt string length");
    if (SIODB_UNLIKELY(consumed == 0)) {
        throw DeserializationError(
                "Not enough data for the string length: " + std::to_string(length));
    }
    length -= consumed;
    if (SIODB_UNLIKELY(length < len)) {
        throw DeserializationError(
                "Invalid string length or not enough data for the string: required "
                + std::to_string(len) + ", but there is only " + std::to_string(length));
    }
    buffer += consumed;
    std::string newStr;
    if (len > 0) newStr.assign(reinterpret_cast<const char*>(buffer), len);
    s.swap(newStr);
    return len + consumed;
}

std::uint8_t* serializeUnchecked(
        const std::uint8_t* bv, std::size_t size, std::uint8_t* buffer) noexcept
{
    buffer = ::encodeVarInt(size, buffer);
    if (SIODB_UNLIKELY(size == 0)) return buffer;
    std::memcpy(buffer, bv, size);
    return buffer + size;
}

std::size_t deserializeObject(
        const std::uint8_t* buffer, std::size_t length, siodb::BinaryValue& bv)
{
    std::uint64_t len = 0;
    const int consumed = ::decodeVarInt(buffer, length, len);
    if (SIODB_UNLIKELY(consumed < 0)) throw DeserializationError("Corrupt binary value length");
    if (SIODB_UNLIKELY(consumed == 0)) {
        throw DeserializationError(
                "Not enough data for the binary value length: " + std::to_string(length));
    }
    length -= consumed;
    if (SIODB_UNLIKELY(length < len)) {
        throw DeserializationError(
                "Invalid binary value length or not enough data for the binary value: required "
                + std::to_string(len) + ", but there is only " + std::to_string(length));
    }
    buffer += consumed;
    siodb::BinaryValue newBv;
    if (len > 0) {
        newBv.resize(len);
        std::memcpy(newBv.data(), buffer, len);
    }
    bv.swap(newBv);
    return len + consumed;
}
