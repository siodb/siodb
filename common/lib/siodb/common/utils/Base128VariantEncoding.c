// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Base128VariantEncoding.h"

// TODO: Consider "Compact Varint"
// https://habr.com/ru/post/350796/

// NOTE:
// Code of the encodeXXX() functions is based
// on the original protobuf function WriteVarint64ToArray().

// NOTE:
// Following code of this function is based
// on the original protobuf function ReadVarint32FromArray().

unsigned getVarUInt16Size(uint16_t value)
{
    unsigned length = 1;
    while (value >= 0x80) {
        ++length;
        value >>= 7;
    }
    return length;
}

unsigned getVarUInt32Size(uint32_t value)
{
    unsigned length = 1;
    while (value >= 0x80) {
        ++length;
        value >>= 7;
    }
    return length;
}

unsigned getVarUInt64Size(uint64_t value)
{
    unsigned length = 1;
    while (value >= 0x80) {
        ++length;
        value >>= 7;
    }
    return length;
}

uint8_t* encodeVarUInt16(uint16_t value, uint8_t* dest)
{
    while (value >= 0x80) {
        *dest++ = (uint8_t)(value | 0x80);
        value >>= 7;
    }
    *dest = (uint8_t)(value);
    return dest + 1;
}

uint8_t* encodeVarUInt32(uint32_t value, uint8_t* dest)
{
    while (value >= 0x80) {
        *dest++ = (uint8_t)(value | 0x80);
        value >>= 7;
    }
    *dest = (uint8_t)(value);
    return dest + 1;
}

uint8_t* encodeVarUInt64(uint64_t value, uint8_t* dest)
{
    while (value >= 0x80) {
        *dest++ = (uint8_t)(value | 0x80);
        value >>= 7;
    }
    *dest = (uint8_t)(value);
    return dest + 1;
}

int decodeVarUInt16(const uint8_t* src, size_t size, uint16_t* value)
{
    if (SIODB_LIKELY(size > 0)) {
        if (*src < 0x80) {
            *value = *src;
            return 1;
        }
        if (size >= kMaxSerializedInt16Size || !(src[size - 1] & 0x80)) {
            const uint8_t* p = src;
            uint16_t result = (*p++) - 0x80;

            uint16_t b = *p++;
            result += b << 7;
            if (!(b & 0x80)) goto done;
            result -= 0x80 << 7;

            b = *p++;
            result += b << 14;
            if (!(b & 0x80)) goto done;

            // Data is corrupt
            return -1;

        done:
            *value = result;
            return (int) (p - src);
        }
    }
    return 0;  // Not enough data
}

int decodeVarUInt32(const uint8_t* src, size_t size, uint32_t* value)
{
    if (SIODB_LIKELY(size > 0)) {
        if (*src < 0x80) {
            *value = *src;
            return 1;
        }
        if (size >= kMaxSerializedInt32Size || !(src[size - 1] & 0x80)) {
            const uint8_t* p = src;
            uint32_t result = (*p++) - 0x80;

            uint32_t b = *p++;
            result += b << 7;
            if (!(b & 0x80)) goto done;
            result -= 0x80 << 7;

            b = *p++;
            result += b << 14;
            if (!(b & 0x80)) goto done;
            result -= 0x80 << 14;

            b = *p++;
            result += b << 21;
            if (!(b & 0x80)) goto done;
            result -= 0x80 << 21;

            b = *p++;
            result += b << 28;
            if (!(b & 0x80)) goto done;

            // Data is corrupt
            return -1;

        done:
            *value = result;
            return (int) (p - src);
        }
    }
    return 0;  // Not enough data
}

int decodeVarUInt64(const uint8_t* src, size_t size, uint64_t* value)
{
    if (SIODB_LIKELY(size > 0)) {
        if (*src < 0x80) {
            *value = *src;
            return 1;
        }
        if (size >= kMaxSerializedInt64Size || !(src[size - 1] & 0x80)) {
            const uint8_t* p = src;

            // Splitting into 32-bit pieces gives better performance
            // on 32-bit processors.
            uint32_t part0 = 0;
            uint32_t part1 = 0;
            uint32_t part2 = 0;

            uint32_t b = *(p++);
            part0 = b;
            if (!(b & 0x80)) goto done;
            part0 -= 0x80;

            b = *(p++);
            part0 += b << 7;
            if (!(b & 0x80)) goto done;
            part0 -= 0x80 << 7;

            b = *(p++);
            part0 += b << 14;
            if (!(b & 0x80)) goto done;
            part0 -= 0x80 << 14;

            b = *(p++);
            part0 += b << 21;
            if (!(b & 0x80)) goto done;
            part0 -= 0x80 << 21;

            b = *(p++);
            part1 = b;
            if (!(b & 0x80)) goto done;
            part1 -= 0x80;

            b = *(p++);
            part1 += b << 7;
            if (!(b & 0x80)) goto done;
            part1 -= 0x80 << 7;

            b = *(p++);
            part1 += b << 14;
            if (!(b & 0x80)) goto done;
            part1 -= 0x80 << 14;

            b = *(p++);
            part1 += b << 21;
            if (!(b & 0x80)) goto done;
            part1 -= 0x80 << 21;

            b = *(p++);
            part2 = b;
            if (!(b & 0x80)) goto done;
            part2 -= 0x80;

            b = *(p++);
            part2 += b << 7;
            if (!(b & 0x80)) goto done;

            // Data is corrupt
            return -1;

        done:
            // Decoding success
            *value = ((uint64_t) part0) | (((uint64_t) part1) << 28) | (((uint64_t) part2) << 56);
            return (int) (p - src);
        }
    }
    return 0;  // not enough data
}
