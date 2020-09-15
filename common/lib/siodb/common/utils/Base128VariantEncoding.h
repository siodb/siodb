// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../crt_ext/compiler_defs.h"

// CRT headers
#include <stddef.h>
#include <stdint.h>

/** Maximum serialized 16-bit integer size in bytes */
#define kMaxSerializedInt16Size 3

/** Maximum serialized 32-bit integer size in bytes */
#define kMaxSerializedInt32Size 5

/** Maximum serialized 64-bit integer size in bytes */
#define kMaxSerializedInt64Size 10

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Encodes 16-bit unsigned integer into Base128 variant.
 * Assumes destination buffer is long enough.
 * @param value A value to encode.
 * @param dest Target buffer.
 * @return Address after last written byte.
 */
uint8_t* encodeVarUInt16(uint16_t value, uint8_t* dest);

/**
 * Encodes 16-bit signed integer into Base128 variant.
 * Assumes destination buffer is long enough.
 * @param value A value to encode.
 * @param dest Target buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* encodeVarInt16(int16_t value, uint8_t* dest)
{
    return encodeVarUInt16((value << 1) ^ (value >> 15), dest);
}

/**
 * Returns encoded length of the 16-bit unsigned integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value length in bytes.
 */
unsigned getVarUInt16Size(uint16_t value);

/**
 * Returns encoded length of the 16-bit signed integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value length in bytes.
 */
SIODB_ALWAYS_INLINE static inline unsigned getVarInt16Size(int16_t value)
{
    return getVarUInt16Size((value << 1) ^ (value >> 15));
}

/**
 * Encodes 32-bit unsigned integer into Base128 variant.
 * Assumes destination buffer is long enough.
 * @param value A value to encode.
 * @param dest Target buffer.
 * @return Address after last written byte.
 */
uint8_t* encodeVarUInt32(uint32_t value, uint8_t* dest);

/**
 * Encodes 32-bit signed integer into Base128 variant.
 * Assumes destination buffer is long enough.
 * @param value A value to encode.
 * @param dest Target buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* encodeVarInt32(int32_t value, uint8_t* dest)
{
    return encodeVarUInt32((value << 1) ^ (value >> 31), dest);
}

/**
 * Returns encoded length of the 32-bit unsigned integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value length in bytes.
 */
unsigned getVarUInt32Size(uint32_t value);

/**
 * Returns encoded length of the 32-bit signed integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value length in bytes.
 */
SIODB_ALWAYS_INLINE static inline unsigned getVarInt32Size(int32_t value)
{
    return getVarUInt32Size((value << 1) ^ (value >> 31));
}

/**
 * Encodes 64-bit unsigned integer into Base128 variant.
 * Assumes destination buffer is long enough.
 * @param value Output value.
 * @param dest Target buffer.
 * @return Address after the last written byte.
 */
uint8_t* encodeVarUInt64(uint64_t value, uint8_t* dest);

/**
 * Encodes 64-bit signed integer into Base128 variant.
 * Assumes destination buffer is long enough.
 * @param value A value to encode.
 * @param dest Target buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* encodeVarInt64(int64_t value, uint8_t* dest)
{
    return encodeVarUInt64((value << 1) ^ (value >> 63), dest);
}

/**
 * Returns encoded length of the 64-bit unsigned integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value length in bytes.
 */
unsigned getVarUInt64Size(uint64_t value);

/**
 * Returns encoded length of the 64-bit signed integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value length in bytes.
 */
SIODB_ALWAYS_INLINE static inline uint32_t getVarInt64Size(int64_t value)
{
    return getVarUInt64Size((value << 1) ^ (value >> 63));
}

/**
 * Decodes 16-bit unsigned integer from Base128 variant.
 * @param src Buffer to decode.
 * @param size Data size in the buffer.
 * @param value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
int decodeVarUInt16(const uint8_t* src, size_t size, uint16_t* value);

/**
 * Decodes 16-bit signed integer from Base128 variant.
 * @param src Source buffer.
 * @param size Data size in the buffer.
 * @param value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
static inline int decodeVarInt16(const uint8_t* src, size_t size, int16_t* value)
{
    uint16_t v = 0;
    const int result = decodeVarUInt16(src, size, &v);
    if (SIODB_LIKELY(result > 0)) *value = (v >> 1) ^ -(v & 1);
    return result;
}

/**
 * Decodes 32-bit unsigned integer from Base128 variant.
 * @param src Source buffer.
 * @param size Data size in the buffer.
 * @param value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
int decodeVarUInt32(const uint8_t* src, size_t size, uint32_t* value);

/**
 * Decodes 32-bit signed integer from Base128 variant.
 * @param src Source buffer.
 * @param size Data size in the buffer.
 * @param value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
static inline int decodeVarInt32(const uint8_t* src, size_t size, int32_t* value)
{
    uint32_t v = 0;
    const int result = decodeVarUInt32(src, size, &v);
    if (SIODB_LIKELY(result > 0)) *value = (v >> 1) ^ -(v & 1);
    return result;
}

/**
 * Decodes 64-bit unsigned integer from Base128 variant.
 * @param src Source buffer.
 * @param size Data size in the buffer.
 * @param value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
int decodeVarUInt64(const uint8_t* src, size_t size, uint64_t* value);

/**
 * Decodes 64-bit signed integer from Base128 variant.
 * @param src Source buffer.
 * @param size Data size in the buffer.
 * @param value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
static inline int decodeVarInt64(const uint8_t* src, size_t size, int64_t* value)
{
    uint64_t v = 0;
    const int result = decodeVarUInt64(src, size, &v);
    if (SIODB_LIKELY(result > 0)) *value = (v >> 1) ^ -(v & 1);
    return result;
}

#ifdef __cplusplus

}  // extern "C"

// Project headers (C++)
#include "BinaryValue.h"
#include "DeserializationError.h"
#include "Uuid.h"

// STL headers
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

/**
 * Encodes 16-bit signed integer as Base128 variant.
 * @param value Value to encode.
 * @param dest Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* encodeVarInt(int16_t value, uint8_t* dest) noexcept
{
    return ::encodeVarInt16(value, dest);
}

/**
 * Encodes 16-bit unsigned integer as Base128 variant.
 * @param value Value to encode.
 * @param dest Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* encodeVarInt(uint16_t value, uint8_t* dest) noexcept
{
    return ::encodeVarUInt16(value, dest);
}

/**
 * Encodes 32-bit signed integer as Base128 variant.
 * @param value Value to encode.
 * @param dest Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* encodeVarInt(int32_t value, uint8_t* dest) noexcept
{
    return ::encodeVarInt32(value, dest);
}

/**
 * Encodes 32-bit unsigned integer as Base128 variant.
 * @param value Value to encode.
 * @param dest Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* encodeVarInt(uint32_t value, uint8_t* dest) noexcept
{
    return ::encodeVarUInt32(value, dest);
}

/**
 * Encodes 64-bit signed integer as Base128 variant.
 * @param value Value to encode.
 * @param dest Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* encodeVarInt(int64_t value, uint8_t* dest) noexcept
{
    return ::encodeVarInt64(value, dest);
}

/**
 * Encodes 64-bit unsigned integer as Base128 variant.
 * @param value Value to encode.
 * @param dest Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* encodeVarInt(uint64_t value, uint8_t* dest) noexcept
{
    return ::encodeVarUInt64(value, dest);
}

/**
 * Decodes 16-bit signed integer from Base128 variant.
 * @param src Source buffer.
 * @param size Size of data in the buffer.
 * @param[out] value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
SIODB_ALWAYS_INLINE inline int decodeVarInt(
        const uint8_t* src, size_t size, int16_t& value) noexcept
{
    return ::decodeVarInt16(src, size, &value);
}

/**
 * Decodes 16-bit unsigned integer from Base128 variant.
 * @param src Source buffer.
 * @param size Size of data in the buffer.
 * @param[out] value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
SIODB_ALWAYS_INLINE inline int decodeVarInt(
        const uint8_t* src, size_t size, uint16_t& value) noexcept
{
    return ::decodeVarUInt16(src, size, &value);
}

/**
 * Decodes 32-bit signed integer from Base128 variant.
 * @param src Source buffer.
 * @param size Size of data in the buffer.
 * @param[out] value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
SIODB_ALWAYS_INLINE inline int decodeVarInt(
        const uint8_t* src, size_t size, int32_t& value) noexcept
{
    return ::decodeVarInt32(src, size, &value);
}

/**
 * Decodes 32-bit unsigned integer from Base128 variant.
 * @param src Source buffer.
 * @param size Size of data in the buffer.
 * @param[out] value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
SIODB_ALWAYS_INLINE inline int decodeVarInt(
        const uint8_t* src, size_t size, uint32_t& value) noexcept
{
    return ::decodeVarUInt32(src, size, &value);
}

/**
 * Decodes 64-bit signed integer from Base128 variant.
 * @param src Source buffer.
 * @param size Size of data in the buffer.
 * @param[out] value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
SIODB_ALWAYS_INLINE inline int decodeVarInt(
        const uint8_t* src, size_t size, int64_t& value) noexcept
{
    return ::decodeVarInt64(src, size, &value);
}

/**
 * Decodes 64-bit unsigned integer from Base128 variant.
 * @param src Source buffer.
 * @param size Size of data in the buffer.
 * @param[out] value Output value.
 * @return On success returns number of bytes consumed from buffer.
 *         If there is not enough data in buffer, returns 0.
 *         If data is corrupted, returns -1.
 */
SIODB_ALWAYS_INLINE inline int decodeVarInt(
        const uint8_t* src, size_t size, uint64_t& value) noexcept
{
    return ::decodeVarUInt64(src, size, &value);
}

/**
 * Returns encoded size of the 16-bit signed integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value size in bytes.
 */
SIODB_ALWAYS_INLINE inline unsigned getVarIntSize(int16_t value) noexcept
{
    return ::getVarInt16Size(value);
}

/**
 * Returns encoded size of the 16-bit unsigned integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value size in bytes.
 */
SIODB_ALWAYS_INLINE inline unsigned getVarIntSize(uint16_t value) noexcept
{
    return ::getVarUInt16Size(value);
}

/**
 * Returns encoded size of the 32-bit signed integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value size in bytes.
 */
SIODB_ALWAYS_INLINE inline unsigned getVarIntSize(int32_t value) noexcept
{
    return ::getVarInt32Size(value);
}

/**
 * Returns encoded size of the 32-bit unsigned integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value size in bytes.
 */
SIODB_ALWAYS_INLINE inline unsigned getVarIntSize(uint32_t value) noexcept
{
    return ::getVarUInt32Size(value);
}

/**
 * Returns encoded size of the 64-bit signed integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value size in bytes.
 */
SIODB_ALWAYS_INLINE inline unsigned getVarIntSize(int64_t value) noexcept
{
    return ::getVarInt64Size(value);
}

/**
 * Returns encoded length of the 64-bit unsigned integer as a Base128 variant.
 * @param value A value to encode.
 * @return Encoded value length in bytes.
 */
SIODB_ALWAYS_INLINE inline unsigned getVarIntSize(uint64_t value) noexcept
{
    return ::getVarUInt64Size(value);
}

/**
 * Returns memory size in bytes required to serialize given string.
 * @param s A string.
 * @return Memory size in bytes.
 */
SIODB_ALWAYS_INLINE inline std::size_t getSerializedSize(const std::string& s) noexcept
{
    const auto len = s.length();
    return ::getVarIntSize(len) + len;
}

/**
 * Serializes string into a buffer, doesn't check buffer size.
 * @param s A string to serialize.
 * @param length Length of the string.
 * @param buffer Destination buffer.
 */
std::uint8_t* serializeUnchecked(const char* s, std::size_t length, std::uint8_t* buffer) noexcept;

/**
 * Serializes string into a buffer, doesn't check buffer size.
 * @param s A string to serialize.
 * @param buffer Destination buffer.
 */
std::uint8_t* serializeUnchecked(const char* s, std::uint8_t* buffer) noexcept;

/**
 * Serializes string into a buffer, doesn't check buffer size.
 * @param s A string to serialize.
 * @param buffer Destination buffer.
 */
SIODB_ALWAYS_INLINE inline std::uint8_t* serializeUnchecked(
        const std::string& s, std::uint8_t* buffer) noexcept
{
    return serializeUnchecked(s.c_str(), s.length(), buffer);
}

/**
 * Deserializes string from a buffer.
 * @param buffer Source buffer.
 * @param length Data length in the buffer.
 * @param[out] s Destination string object.
 * @return Number of consumed bytes.
 * @throw VariantDeserializationError if deserialization failed.
 */
std::size_t deserializeObject(const std::uint8_t* buffer, std::size_t length, std::string& s);

/**
 * Returns memory size in bytes required to serialize given binary value.
 * @param bv A binary value.
 * @return Memory size in bytes.
 */
SIODB_ALWAYS_INLINE inline std::size_t getSerializedSize(const siodb::BinaryValue& bv) noexcept
{
    const auto size = bv.size();
    return ::getVarIntSize(size) + size;
}

/**
 * Serializes binary value into a buffer, doesn't check buffer size.
 * @param bv A binary value to serialize.
 * @param size Binary value size.
 * @param buffer Destination buffer.
 */
std::uint8_t* serializeUnchecked(
        const std::uint8_t* bv, std::size_t size, std::uint8_t* buffer) noexcept;

/**
 * Serializes binary value into a buffer, doesn't check buffer size.
 * @param bv A binary value to serialize.
 * @param buffer Destination buffer.
 */
SIODB_ALWAYS_INLINE inline std::uint8_t* serializeUnchecked(
        const siodb::BinaryValue& bv, std::uint8_t* buffer) noexcept
{
    return serializeUnchecked(bv.data(), bv.size(), buffer);
}

/**
 * Deserializes binary value from a buffer.
 * @param buffer Source buffer.
 * @param length Data length in the buffer.
 * @param[out] bv Destination binary value object.
 * @return Number of consumed bytes.
 * @throw VariantDeserializationError if deserialization failed.
 */
std::size_t deserializeObject(
        const std::uint8_t* buffer, std::size_t length, siodb::BinaryValue& bv);

/**
 * Returns memory size in bytes required to serialize given optional object.
 * @param opt An optional object to serialize.
 * @return Memory size in bytes.
 */
template<class T>
inline std::size_t getSerializedSize(const std::optional<T>& opt) noexcept
{
    if constexpr (std::is_fundamental_v<T>)
        return opt ? ::getVarIntSize(*opt) + 1U : 1U;
    else
        return opt ? ::getSerializedSize(*opt) + 1U : 1U;
}

/**
 * Serializes optional object into a buffer, doesn't check buffer size.
 * @param opt An optional object to serialize.
 * @param buffer Destination buffer.
 */
template<class T>
std::uint8_t* serializeUnchecked(const std::optional<T>& opt, std::uint8_t* buffer) noexcept
{
    *buffer++ = opt ? 1 : 0;
    if (opt) {
        const auto& obj = *opt;
        if constexpr (std::is_fundamental_v<T>)
            buffer = encodeVarInt(*opt, buffer);
        else
            buffer = serializeUnchecked(obj, buffer);
    }
    return buffer;
}

/**
 * Deserializes optional object from a buffer, doesn't check buffer size.
 * @tparam Object type.
 * @param buffer Source buffer.
 * @param length Data length in the buffer.
 * @param[out] opt Destination object.
 * @return Number of consumed bytes.
 * @throw VariantDeserializationError if deserialization failed.
 */
template<class T>
std::size_t deserializeObject(const std::uint8_t* buffer, std::size_t length, std::optional<T>& opt)
{
    if (SIODB_UNLIKELY(length < 1))
        throw siodb::utils::DeserializationError("Not enough data for the optional object");

    if (*buffer > 1) throw siodb::utils::DeserializationError("Invalid optional value flag");

    if (!*buffer++) {
        opt.reset();
        return 1;
    }

    if (opt) {
        if constexpr (std::is_fundamental_v<T>)
            return decodeVarInt(buffer, length - 1, *opt);
        else
            return deserializeObject(buffer, length - 1, *opt) + 1;
    } else {
        T tmp = T();
        std::size_t consumed;
        if constexpr (std::is_fundamental_v<T>) {
            consumed = decodeVarInt(buffer, length - 1, tmp);
            opt = tmp;
        } else {
            consumed = deserializeObject(buffer, length - 1, tmp);
            opt = std::move(tmp);
        }
        return consumed + 1;
    }
}

#endif  // __cplusplus
