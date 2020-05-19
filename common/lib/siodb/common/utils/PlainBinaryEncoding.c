// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "PlainBinaryEncoding.h"

uint8_t* pbeEncodeTinyString(const char* value, uint8_t* buffer)
{
    const size_t length = strlen(value);
    if (SIODB_UNLIKELY(length > 0xFF)) return NULL;
    *buffer++ = (uint8_t) length;
    if (SIODB_LIKELY(length > 0)) {
        memcpy(buffer, value, length);
        buffer += length;
    }
    return buffer;
}

uint8_t* pbeEncodeShortString(const char* value, uint8_t* buffer)
{
    const size_t length = strlen(value);
    if (SIODB_UNLIKELY(length > 0xFFFF)) return NULL;
    buffer = pbeEncodeUInt16((uint16_t) length, buffer);
    if (SIODB_LIKELY(length > 0)) {
        memcpy(buffer, value, length);
        buffer += length;
    }
    return buffer;
}

uint8_t* pbeEncodeLongString(const char* value, uint8_t* buffer)
{
    const size_t length = strlen(value);
    if (SIODB_UNLIKELY(length > 0xFFFFFFFFULL)) return NULL;
    buffer = pbeEncodeUInt32((uint32_t) length, buffer);
    if (SIODB_LIKELY(length > 0)) {
        memcpy(buffer, value, length);
        buffer += length;
    }
    return buffer;
}

const uint8_t* pbeDecodeTinyString(const uint8_t* buffer, char* value, size_t size)
{
    const uint8_t length = *buffer++;
    if (SIODB_UNLIKELY(size == 0)) return NULL;
    if (SIODB_UNLIKELY(length > size - 1)) return NULL;
    memcpy(value, buffer, length);
    value[length] = '\0';
    return buffer + length;
}

const uint8_t* pbeDecodeShortString(const uint8_t* buffer, char* value, size_t size)
{
    uint16_t length = 0;
    buffer = pbeDecodeUInt16(buffer, &length);
    if (SIODB_UNLIKELY(size == 0)) return NULL;
    if (SIODB_UNLIKELY(length > size - 1)) return NULL;
    memcpy(value, buffer, length);
    value[length] = '\0';
    return buffer + length;
}

const uint8_t* pbeDecodeLongString(const uint8_t* buffer, char* value, size_t size)
{
    uint32_t length = 0;
    buffer = pbeDecodeUInt32(buffer, &length);
    if (SIODB_UNLIKELY(size == 0)) return NULL;
    if (SIODB_UNLIKELY(length > size - 1)) return NULL;
    memcpy(value, buffer, length);
    value[length] = '\0';
    return buffer + length;
}
