// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "CustomCodedInputStream.h"

// STL headers
#include <stdexcept>

namespace siodb::protobuf {

BinaryValue CustomCodedInputStream::readBinary()
{
    std::uint32_t blobLength = 0;
    // Read length
    if (!ReadVarint32(&blobLength)) throw std::runtime_error("Read binary value length failed");

    BinaryValue value;
    value.resize(blobLength);

    if (!ReadRaw(value.data(), blobLength))
        throw std::runtime_error("Read binary value data length failed");

    return value;
}

std::string CustomCodedInputStream::readString()
{
    std::uint32_t clobLength = 0;
    // Read length
    if (!ReadVarint32(&clobLength)) throw std::runtime_error("Read string length failed");

    // Read sample
    std::string str;
    str.resize(clobLength);

    if (!ReadRaw(str.data(), clobLength)) throw std::runtime_error("Read string data failed");

    return str;
}

std::int8_t CustomCodedInputStream::readInt8()
{
    std::int8_t data = 0;
    if (!ReadRaw(&data, sizeof(data))) throw std::runtime_error("Read int8 failed");
    return data;
}

std::int8_t CustomCodedInputStream::readInt16()
{
    std::int16_t data = 0;
    if (!ReadRaw(&data, sizeof(data))) throw std::runtime_error("Read int16 failed");
    return data;
}

std::int32_t CustomCodedInputStream::readInt32()
{
    std::int32_t data = 0;
    if (!ReadVarint32(reinterpret_cast<std::uint32_t*>(&data)))
        throw std::runtime_error("Read int32 failed");
    return data;
}

std::int64_t CustomCodedInputStream::readInt64()
{
    std::int64_t data = 0;
    if (!ReadVarint64(reinterpret_cast<std::uint64_t*>(&data)))
        throw std::runtime_error("Read int64 failed");
    return data;
}

std::uint8_t CustomCodedInputStream::readUInt8()
{
    std::int8_t data = 0;
    if (!ReadRaw(&data, sizeof(data))) throw std::runtime_error("Read uint8 failed");
    return data;
}

std::uint8_t CustomCodedInputStream::readUInt16()
{
    std::uint16_t data = 0;
    if (!ReadRaw(&data, sizeof(data))) throw std::runtime_error("Read uint16 failed");
    return data;
}

std::uint32_t CustomCodedInputStream::readUInt32()
{
    std::uint32_t data = 0;
    if (!ReadVarint32(&data)) throw std::runtime_error("Read uint32 failed");
    return data;
}

std::uint64_t CustomCodedInputStream::readUInt64()
{
    std::uint64_t data = 0;
    if (!ReadVarint64(&data)) throw std::runtime_error("Read uint64 failed");
    return data;
}

float CustomCodedInputStream::readFloat()
{
    float data = 0;
    if (!ReadLittleEndian32(reinterpret_cast<std::uint32_t*>(&data)))
        throw std::runtime_error("Read float failed");
    return data;
}

double CustomCodedInputStream::readDouble()
{
    double data = 0;
    if (!ReadLittleEndian64(reinterpret_cast<std::uint64_t*>(&data)))
        throw std::runtime_error("Read double failed");
    return data;
}

bool CustomCodedInputStream::readBool()
{
    std::uint8_t data = 0;
    if (!ReadRaw(&data, sizeof(data))) throw std::runtime_error("Read bool failed");
    return data != 0;
}

}  // namespace siodb::protobuf
