// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
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
    if (ReadRaw(&v, sizeof(v))) {
        *value = (v != 0);
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(std::int8_t* value)
{
    std::int8_t v = 0;
    if (ReadRaw(&v, sizeof(v))) {
        *value = v;
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(std::uint8_t* value)
{
    std::uint8_t v = 0;
    if (ReadRaw(&v, sizeof(v))) {
        *value = v;
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(std::int16_t* value)
{
    std::int16_t v = 0;
    if (ReadRaw(&v, sizeof(v))) {
        *value = boost::endian::little_to_native(v);
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(std::uint16_t* value)
{
    std::uint16_t v = 0;
    if (ReadRaw(&v, sizeof(v))) {
        *value = boost::endian::little_to_native(v);
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(std::int32_t* value)
{
    std::int32_t v = 0;
    if (ReadVarint32(reinterpret_cast<std::uint32_t*>(&v))) {
        *value = v;
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(std::uint32_t* value)
{
    std::uint32_t v = 0;
    if (ReadVarint32(&v)) {
        *value = v;
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(std::int64_t* value)
{
    std::int64_t v = 0;
    if (ReadVarint64(reinterpret_cast<std::uint64_t*>(&v))) {
        *value = v;
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(std::uint64_t* value)
{
    std::uint64_t v = 0;
    if (ReadVarint64(&v)) {
        *value = v;
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(float* value)
{
    float v = 0.0f;
    if (ReadLittleEndian32(reinterpret_cast<std::uint32_t*>(&v))) {
        *value = v;
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(double* value)
{
    double v = 0.0;
    if (ReadLittleEndian64(reinterpret_cast<std::uint64_t*>(&v))) {
        *value = v;
        return true;
    }
    return false;
}

bool ExtendedCodedInputStream::Read(std::string* value)
{
    std::uint32_t length = 0;
    if (ReadVarint32(&length)) {
        std::string str;
        if (length > 0) {
            str.resize(length);
            if (ReadRaw(str.data(), length)) {
                *value = std::move(str);
                return true;
            }
        } else {
            *value = std::move(str);
            return true;
        }
    }
    return false;
}

bool ExtendedCodedInputStream::Read(BinaryValue* value)
{
    std::uint32_t length = 0;
    if (ReadVarint32(&length)) {
        BinaryValue bv;
        if (length > 0) {
            bv.resize(length);
            if (ReadRaw(bv.data(), length)) {
                *value = std::move(bv);
                return true;
            }
        } else {
            *value = std::move(bv);
            return true;
        }
    }
    return false;
}

}  // namespace siodb::protobuf
