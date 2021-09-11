// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../config/Config.h"
#include "../crt_ext/compiler_defs.h"

// CRT headers
#include <stdint.h>
#include <string.h>

// System headers
#include <byteswap.h>
#include <endian.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Encodes 16-bit signed integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeInt16(const int16_t value, uint8_t* buffer)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *(int16_t*) buffer = value;
    return buffer + 2;
#else
    *buffer++ = value & 0xFF;
    *buffer++ = (value >> 8) & 0xFF;
    return buffer;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
}

/**
 * Encodes 32-bit signed integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeInt32(const int32_t value, uint8_t* buffer)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *(int32_t*) buffer = value;
    return buffer + 4;
#else
    *buffer++ = value & 0xFF;
    *buffer++ = (value >> 8) & 0xFF;
    *buffer++ = (value >> 16) & 0xFF;
    *buffer++ = (value >> 24) & 0xFF;
    return buffer;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
}

/**
 * Encodes 64-bit signed integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeInt64(const int64_t value, uint8_t* buffer)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *(int64_t*) buffer = value;
    return buffer + 8;
#else
    int32_t v = value & 0xFFFFFFFF;
    *buffer++ = v & 0xFF;
    *buffer++ = (v >> 8) & 0xFF;
    *buffer++ = (v >> 16) & 0xFF;
    *buffer++ = (v >> 24) & 0xFF;
    v = (value >> 32) & 0xFFFFFFFF;
    *buffer++ = v & 0xFF;
    *buffer++ = (v >> 8) & 0xFF;
    *buffer++ = (v >> 16) & 0xFF;
    *buffer++ = (v >> 24) & 0xFF;
    return buffer;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
}

/**
 * Encodes 16-bit unsigned integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeUInt16(const uint16_t value, uint8_t* buffer)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *(uint16_t*) buffer = value;
    return buffer + 2;
#else
    *buffer++ = value & 0xFF;
    *buffer++ = (value >> 8) & 0xFF;
    return buffer;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
}

/**
 * Encodes 32-bit unsigned integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeUInt32(const uint32_t value, uint8_t* buffer)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *(uint32_t*) buffer = value;
    return buffer + 4;
#else
    *buffer++ = value & 0xFF;
    *buffer++ = (value >> 8) & 0xFF;
    *buffer++ = (value >> 16) & 0xFF;
    *buffer++ = (value >> 24) & 0xFF;
    return buffer;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
}

/**
 * Encodes 64-bit unsigned integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeUInt64(const uint64_t value, uint8_t* buffer)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *(uint64_t*) buffer = value;
    return buffer + 8;
#else
    int32_t v = value & 0xFFFFFFFF;
    *buffer++ = v & 0xFF;
    *buffer++ = (v >> 8) & 0xFF;
    *buffer++ = (v >> 16) & 0xFF;
    *buffer++ = (v >> 24) & 0xFF;
    v = (value >> 32) & 0xFFFFFFFF;
    *buffer++ = v & 0xFF;
    *buffer++ = (v >> 8) & 0xFF;
    *buffer++ = (v >> 16) & 0xFF;
    *buffer++ = (v >> 24) & 0xFF;
    return buffer;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
}

/**
 * Encodes 32-bit floating point number into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeFloat(const float value, uint8_t* buffer)
{
    const uint32_t* pv = (const uint32_t*) &value;
    return pbeEncodeUInt32(*pv, buffer);
}

/**
 * Encodes 64-bit floating point number into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeDouble(const double value, uint8_t* buffer)
{
    const uint64_t* pv = (const uint64_t*) &value;
    return pbeEncodeUInt64(*pv, buffer);
}

/**
 * Encodes 16-bit zero into buffer.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeZero16(uint8_t* buffer)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *(uint16_t*) buffer = 0;
#else
    buffer[0] = 0;
    buffer[1] = 0;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 2;
}

/**
 * Encodes 32-bit zero into buffer.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeZero32(uint8_t* buffer)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *(uint32_t*) buffer = 0;
#else
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[3] = 0;
    buffer[4] = 0;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 4;
}

/**
 * Encodes 64-bit zero into buffer.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeZero64(uint8_t* buffer)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *(uint64_t*) buffer = 0;
#else
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[3] = 0;
    buffer[4] = 0;
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = 0;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 8;
}

/**
 * Encodes tiny string (up to 255 bytes long) into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last read byte on success, NULL if string is longer than 255 bytes.
 */
uint8_t* pbeEncodeTinyString(const char* value, uint8_t* buffer);

/**
 * Encodes short string (up to 65535 bytes long) into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last read byte on success, NULL if string is longer than 255 bytes.
 */
uint8_t* pbeEncodeShortString(const char* value, uint8_t* buffer);

/**
 * Encodes long string (up to 2^32-1 bytes long) into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last read byte on success, NULL if string is longer than 255 bytes.
 */
uint8_t* pbeEncodeLongString(const char* value, uint8_t* buffer);

/**
 * Decodes binary data from buffer.
 * @param value Address of source buffer.
 * @param size Data size.
 * @param buffer Destination buffer.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline uint8_t* pbeEncodeBinary(
        const void* value, size_t size, uint8_t* buffer)
{
    memcpy(buffer, value, size);
    return buffer + size;
}

/**
 * Decodes 16-bit signed integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeInt16(
        const uint8_t* buffer, int16_t* value)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *value = *(const int16_t*) buffer;
#else
    *value = buffer[0] | buffer[1] << 8;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 2;
}

/**
 * Decodes 32-bit signed integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeInt32(
        const uint8_t* buffer, int32_t* value)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *value = *(const int32_t*) buffer;
#else
    *value = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 4;
}

/**
 * Decodes 64-bit signed integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeInt64(
        const uint8_t* buffer, int64_t* value)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *value = *(const int64_t*) buffer;
#else
    *((uint64_t*) value) =
            (((uint64_t)((uint32_t)(
                     buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24))))
                    << 32)
            | ((uint32_t)(buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 8;
}

/**
 * Decodes 16-bit unsigned integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeUInt16(
        const uint8_t* buffer, uint16_t* value)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *value = *(const uint16_t*) buffer;
#else
    *value = buffer[0] | buffer[1] << 8;
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 2;
}

/**
 * Decodes 32-bit unsigned integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeUInt32(
        const uint8_t* buffer, uint32_t* value)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *value = *(const uint32_t*) buffer;
#else
    *value = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 4;
}

/**
 * Decodes 32-bit unsigned integer from buffer in the LE order.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeUInt32LE(
        const uint8_t* buffer, uint32_t* value)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
#if __BYTE_ORDER == __BIG_ENDIAN
    *value = __bswap_32(*(const uint32_t*) buffer);
#else
    *value = *(const uint32_t*) buffer;
#endif
#else
    *value = (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 4;
}

/**
 * Decodes 64-bit unsigned integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeUInt64(
        const uint8_t* buffer, uint64_t* value)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    *value = *(const uint64_t*) buffer;
#else
    *((uint64_t*) value) =
            (((uint64_t)((uint32_t)(
                     buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24))))
                    << 32)
            | ((uint32_t)(buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 8;
}

/**
 * Decodes 64-bit unsigned integer from buffer in the LE order.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeUInt64LE(
        const uint8_t* buffer, uint64_t* value)
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
#if __BYTE_ORDER == __BIG_ENDIAN
    *value = __bswap_64(*(const uint64_t*) buffer);
#else
    *value = *(const uint64_t*) buffer;
#endif
#else
    *((uint64_t*) value) =
            (((uint64_t)((uint32_t)(
                     buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24))))
                    << 32)
            | ((uint32_t)(buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)));
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return buffer + 8;
}

/**
 * Decodes 32-bit floating point number into buffer.
 * @param buffer Source buffer.
 * @param value Address of destination floating point value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeFloat(const uint8_t* buffer, float* value)
{
    return pbeDecodeUInt32(buffer, (uint32_t*) value);
}

/**
 * Decodes 64-bit floating point number into buffer.
 * @param buffer Source buffer.
 * @param value Address of destination floating point value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeDouble(
        const uint8_t* buffer, double* value)
{
    return pbeDecodeUInt64(buffer, (uint64_t*) value);
}

/**
 * Decodes tiny string (up to 255 bytes) from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination string buffer.
 * @param size Destination buffer size.
 * @return Address after last read byte on success, NULL of source buffer is too short.
 */
const uint8_t* pbeDecodeTinyString(const uint8_t* buffer, char* value, size_t size);

/**
 * Decodes short string (up to 65535 bytes) from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination string buffer.
 * @param size Destination buffer size.
 * @return Address after last read byte.
 */
const uint8_t* pbeDecodeShortString(const uint8_t* buffer, char* value, size_t size);

/**
 * Decodes long string (up to 2^32-1 bytes) from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination string buffer.
 * @param size Destination buffer size.
 * @return Address after last read byte.
 */
const uint8_t* pbeDecodeLongString(const uint8_t* buffer, char* value, size_t size);

/**
 * Decodes binary data from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination buffer.
 * @param size Data size.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE static inline const uint8_t* pbeDecodeBinary(
        const uint8_t* buffer, void* value, size_t size)
{
    memcpy(value, buffer, size);
    return buffer + size;
}

#ifdef __cplusplus

}  // extern "C"

/**
 * Encodes 64-bit signed integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* pbeEncodeInt(int16_t value, uint8_t* buffer)
{
    return ::pbeEncodeInt16(value, buffer);
}

/**
 * Encodes 16-bit unsigned integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* pbeEncodeInt(uint16_t value, uint8_t* buffer)
{
    return ::pbeEncodeUInt16(value, buffer);
}

/**
 * Encodes 32-bit signed integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* pbeEncodeInt(int32_t value, uint8_t* buffer)
{
    return ::pbeEncodeInt32(value, buffer);
}

/**
 * Encodes 32-bit unsigned integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* pbeEncodeInt(uint32_t value, uint8_t* buffer)
{
    return ::pbeEncodeUInt32(value, buffer);
}

/**
 * Encodes 64-bit signed integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* pbeEncodeInt(int64_t value, uint8_t* buffer)
{
    return ::pbeEncodeInt64(value, buffer);
}

/**
 * Encodes 64-bit unsigned integer into buffer.
 * @param value A value to encode.
 * @param buffer Destination buffer.
 * @return Address after last written byte.
 */
SIODB_ALWAYS_INLINE inline uint8_t* pbeEncodeInt(uint64_t value, uint8_t* buffer)
{
    return ::pbeEncodeUInt64(value, buffer);
}

/**
 * Decodes 16-bit signed integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE inline const uint8_t* pbeDecodeInt(const uint8_t* buffer, int16_t& value)
{
    return ::pbeDecodeInt16(buffer, &value);
}

/**
 * Decodes 16-bit unsigned integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE inline const uint8_t* pbeDecodeInt(const uint8_t* buffer, uint16_t& value)
{
    return ::pbeDecodeUInt16(buffer, &value);
}

/**
 * Decodes 32-bit signed integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE inline const uint8_t* pbeDecodeInt(const uint8_t* buffer, int32_t& value)
{
    return ::pbeDecodeInt32(buffer, &value);
}

/**
 * Decodes 32-bit unsigned integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE inline const uint8_t* pbeDecodeInt(const uint8_t* buffer, uint32_t& value)
{
    return ::pbeDecodeUInt32(buffer, &value);
}

/**
 * Decodes 64-bit signed integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE inline const uint8_t* pbeDecodeInt(const uint8_t* buffer, int64_t& value)
{
    return ::pbeDecodeInt64(buffer, &value);
}

/**
 * Decodes 64-bit unsigned integer from buffer.
 * @param buffer Source buffer.
 * @param value Address of destination integer value.
 * @return Address after last read byte.
 */
SIODB_ALWAYS_INLINE inline const uint8_t* pbeDecodeInt(const uint8_t* buffer, uint64_t& value)
{
    return ::pbeDecodeUInt64(buffer, &value);
}

#endif  // __cplusplus
