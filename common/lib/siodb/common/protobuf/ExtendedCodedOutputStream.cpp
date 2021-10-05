// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ExtendedCodedOutputStream.h"

// Boost headers
#include <boost/endian/conversion.hpp>

namespace siodb::protobuf {

void ExtendedCodedOutputStream::Write(std::int16_t value)
{
    boost::endian::native_to_little_inplace(value);
    WriteRaw(&value, sizeof(value));
}

void ExtendedCodedOutputStream::Write(std::uint16_t value)
{
    boost::endian::native_to_little_inplace(value);
    WriteRaw(&value, sizeof(value));
}

void ExtendedCodedOutputStream::Write(float value)
{
    // Make GCC strict aliasing rules happy
    union {
        float m_floatValue;
        std::uint32_t m_uint32Value;
    } v;
    v.m_floatValue = value;
    WriteLittleEndian32(v.m_uint32Value);
}

void ExtendedCodedOutputStream::Write(double value)
{
    // Make GCC strict aliasing rules happy
    union {
        double m_doubleValue;
        std::uint64_t m_uint64Value;
    } v;
    v.m_doubleValue = value;
    WriteLittleEndian64(v.m_uint64Value);
}

void ExtendedCodedOutputStream::Write(const std::string& value)
{
    Write(value.c_str(), value.length());
}

void ExtendedCodedOutputStream::Write(const char* value)
{
    Write(value, std::strlen(value));
}

void ExtendedCodedOutputStream::Write(const char* value, std::size_t length)
{
    if (length > std::numeric_limits<std::uint32_t>::max())
        throw std::invalid_argument("String is too long");
    WriteVarint32(static_cast<std::uint32_t>(length));
    if (!HadError() && length > 0) WriteRaw(value, length);
}

void ExtendedCodedOutputStream::Write(const BinaryValue& value)
{
    if (value.size() > std::numeric_limits<std::uint32_t>::max())
        throw std::invalid_argument("Binary value is too long");
    WriteVarint32(static_cast<std::uint32_t>(value.size()));
    if (!HadError() && !value.empty()) WriteRaw(value.data(), value.size());
}

}  // namespace siodb::protobuf
