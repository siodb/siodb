// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ExtendedCodedInputStream.h"

// STL headers
#include <stdexcept>

// System headers
#include <byteswap.h>
#include <endian.h>

namespace siodb::protobuf {

bool ExtendedCodedInputStream::ReadBool(bool* value)
{
    std::uint8_t v = 0;
    if (!ReadRaw(&v, sizeof(v))) return false;
    *value = (v != 0);
    return true;
}

bool ExtendedCodedInputStream::ReadInt(std::int8_t* value)
{
    std::int8_t v = 0;
    if (!ReadRaw(&v, sizeof(v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::ReadInt(std::int16_t* value)
{
    std::int16_t v = 0;
    if (!ReadRaw(&v, sizeof(v))) return false;
#if __BYTE_ORDER == __BIG_ENDIAN
    *value = bswap_16(v);
#else
    *value = v;
#endif
    return true;
}

bool ExtendedCodedInputStream::ReadInt(std::int32_t* value)
{
    std::int32_t v = 0;
    if (!ReadVarint32(reinterpret_cast<std::uint32_t*>(&v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::ReadInt(std::int64_t* value)
{
    std::int64_t v = 0;
    if (!ReadVarint64(reinterpret_cast<std::uint64_t*>(&v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::ReadInt(std::uint8_t* value)
{
    std::uint8_t v = 0;
    if (!ReadRaw(&value, sizeof(v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::ReadInt(std::uint16_t* value)
{
    std::uint16_t v = 0;
    if (!ReadRaw(&value, sizeof(v))) return false;
#if __BYTE_ORDER == __BIG_ENDIAN
    *value = bswap_16(v);
#else
    *value = v;
#endif
    return true;
}

bool ExtendedCodedInputStream::ReadInt(std::uint32_t* value)
{
    std::uint32_t v = 0;
    if (!ReadVarint32(&v)) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::ReadInt(std::uint64_t* value)
{
    std::uint64_t v = 0;
    if (!ReadVarint64(&v)) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::ReadFloat(float* value)
{
    float v = 0.0f;
    if (!ReadLittleEndian32(reinterpret_cast<std::uint32_t*>(&v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::ReadDouble(double* value)
{
    double v = 0.0;
    if (!ReadLittleEndian64(reinterpret_cast<std::uint64_t*>(&v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::ReadString(std::string* value)
{
    std::uint32_t length = 0;
    if (!ReadVarint32(&length)) return false;

    std::string str;
    str.resize(length);
    if (!ReadRaw(str.data(), length)) return false;

    *value = std::move(str);
    return true;
}

bool ExtendedCodedInputStream::ReadBinary(BinaryValue* value)
{
    std::uint32_t length = 0;
    if (!ReadVarint32(&length)) return false;

    BinaryValue bv;
    bv.resize(length);
    if (!ReadRaw(bv.data(), length)) return false;

    *value = std::move(bv);
    return true;
}

}  // namespace siodb::protobuf
