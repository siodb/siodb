// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ExtendedCodedInputStream.h"

// STL headers
#include <stdexcept>

// Boost headers
#include <boost/endian/conversion.hpp>

namespace siodb::protobuf {

bool ExtendedCodedInputStream::Read(bool* value)
{
    std::uint8_t v = 0;
    if (!ReadRaw(&v, sizeof(v))) return false;
    *value = (v != 0);
    return true;
}

bool ExtendedCodedInputStream::Read(std::int8_t* value)
{
    std::int8_t v = 0;
    if (!ReadRaw(&v, sizeof(v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::Read(std::uint8_t* value)
{
    std::uint8_t v = 0;
    if (!ReadRaw(&v, sizeof(v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::Read(std::int16_t* value)
{
    std::int16_t v = 0;
    if (!ReadRaw(&v, sizeof(v))) return false;
    *value = boost::endian::little_to_native(v);
    return true;
}

bool ExtendedCodedInputStream::Read(std::uint16_t* value)
{
    std::uint16_t v = 0;
    if (!ReadRaw(&v, sizeof(v))) return false;
    *value = boost::endian::little_to_native(v);
    return true;
}

bool ExtendedCodedInputStream::Read(std::int32_t* value)
{
    std::int32_t v = 0;
    if (!ReadVarint32(reinterpret_cast<std::uint32_t*>(&v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::Read(std::uint32_t* value)
{
    std::uint32_t v = 0;
    if (!ReadVarint32(&v)) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::Read(std::int64_t* value)
{
    std::int64_t v = 0;
    if (!ReadVarint64(reinterpret_cast<std::uint64_t*>(&v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::Read(std::uint64_t* value)
{
    std::uint64_t v = 0;
    if (!ReadVarint64(&v)) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::Read(float* value)
{
    float v = 0.0f;
    if (!ReadLittleEndian32(reinterpret_cast<std::uint32_t*>(&v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::Read(double* value)
{
    double v = 0.0;
    if (!ReadLittleEndian64(reinterpret_cast<std::uint64_t*>(&v))) return false;
    *value = v;
    return true;
}

bool ExtendedCodedInputStream::Read(std::string* value)
{
    std::uint32_t length = 0;
    if (!ReadVarint32(&length)) return false;

    std::string str;
    if (length > 0) {
        str.resize(length);
        if (!ReadRaw(str.data(), length)) return false;
    }

    *value = std::move(str);
    return true;
}

bool ExtendedCodedInputStream::Read(BinaryValue* value)
{
    std::uint32_t length = 0;
    if (!ReadVarint32(&length)) return false;

    BinaryValue bv;
    if (length > 0) {
        bv.resize(length);
        if (!ReadRaw(bv.data(), length)) return false;
    }

    *value = std::move(bv);
    return true;
}

}  // namespace siodb::protobuf
