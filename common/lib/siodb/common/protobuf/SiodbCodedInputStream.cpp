// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SiodbCodedInputStream.h"

// STL headers
#include <stdexcept>

namespace siodb::protobuf {

bool SiodbCodedInputStream::readBool()
{
    std::uint8_t value = 0;
    if (!ReadRaw(&value, sizeof(value))) throw std::runtime_error("Read boolean failed");
    return value != 0;
}

std::int8_t SiodbCodedInputStream::readInt8()
{
    std::int8_t value = 0;
    if (!ReadRaw(&value, sizeof(value))) throw std::runtime_error("Read int8 failed");
    return value;
}

std::int8_t SiodbCodedInputStream::readInt16()
{
    std::int16_t value = 0;
    if (!ReadRaw(&value, sizeof(value))) throw std::runtime_error("Read int16 failed");
    return value;
}

std::int32_t SiodbCodedInputStream::readInt32()
{
    std::int32_t value = 0;
    if (!ReadVarint32(reinterpret_cast<std::uint32_t*>(&value)))
        throw std::runtime_error("Read int32 failed");
    return value;
}

std::int64_t SiodbCodedInputStream::readInt64()
{
    std::int64_t value = 0;
    if (!ReadVarint64(reinterpret_cast<std::uint64_t*>(&value)))
        throw std::runtime_error("Read int64 failed");
    return value;
}

std::uint8_t SiodbCodedInputStream::readUInt8()
{
    std::int8_t value = 0;
    if (!ReadRaw(&value, sizeof(value))) throw std::runtime_error("Read uint8 failed");
    return value;
}

std::uint8_t SiodbCodedInputStream::readUInt16()
{
    std::uint16_t value = 0;
    if (!ReadRaw(&value, sizeof(value))) throw std::runtime_error("Read uint16 failed");
    return value;
}

std::uint32_t SiodbCodedInputStream::readUInt32()
{
    std::uint32_t value = 0;
    if (!ReadVarint32(&value)) throw std::runtime_error("Read uint32 failed");
    return value;
}

std::uint64_t SiodbCodedInputStream::readUInt64()
{
    std::uint64_t value = 0;
    if (!ReadVarint64(&value)) throw std::runtime_error("Read uint64 failed");
    return value;
}

float SiodbCodedInputStream::readFloat()
{
    float value = 0.0f;
    if (!ReadLittleEndian32(reinterpret_cast<std::uint32_t*>(&value)))
        throw std::runtime_error("Read float failed");
    return value;
}

double SiodbCodedInputStream::readDouble()
{
    double value = 0.0;
    if (!ReadLittleEndian64(reinterpret_cast<std::uint64_t*>(&value)))
        throw std::runtime_error("Read double failed");
    return value;
}

std::string SiodbCodedInputStream::readString()
{
    std::uint32_t length = 0;
    if (!ReadVarint32(&length)) throw std::runtime_error("Read string length failed");

    std::string str;
    str.resize(length);
    if (!ReadRaw(str.data(), length)) throw std::runtime_error("Read string data failed");

    return str;
}

BinaryValue SiodbCodedInputStream::readBinary()
{
    std::uint32_t length = 0;
    if (!ReadVarint32(&length)) throw std::runtime_error("Read binary value length failed");

    BinaryValue value;
    value.resize(length);
    if (!ReadRaw(value.data(), length)) throw std::runtime_error("Read binary value data failed");

    return value;
}

}  // namespace siodb::protobuf
