// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include "../data/RawDateTime.h"
#include "../utils/BinaryValue.h"

// Protobuf headers
#include <google/protobuf/io/coded_stream.h>

namespace siodb::protobuf {

/**
 * Custom coded output stream to write protocol data types.
 * As long as this is extension of the Google Protobuf class,
 * it follows lots of Google's design and coding conventions.
 */
class ExtendedCodedOutputStream : public google::protobuf::io::CodedOutputStream {
public:
    /**
     * Initializes object of class ExtendedCodedOutputStream.
     * @param output Underlying input stream.
     * @param do_eager_refresh Indicates that eager refresh should be done, on by default.
     */
    explicit ExtendedCodedOutputStream(
            google::protobuf::io::ZeroCopyOutputStream* input, bool do_eager_refresh = true)
        : google::protobuf::io::CodedOutputStream(input, do_eager_refresh)
    {
    }

    /**
     * Writes boolean value.
     * @param value A value to write.
     */
    void Write(bool value)
    {
        char v = value ? 1 : 0;
        WriteRaw(&v, 1);
    }

    /**
     * Writes signed 8-bit integer value.
     * @param value A value to write.
     */
    void Write(std::int8_t value)
    {
        WriteRaw(&value, 1);
    }

    /**
     * Writes unsigned 8-bit integer value.
     * @param value A value to write.
     */
    void Write(std::uint8_t value)
    {
        WriteRaw(&value, 1);
    }

    /**
     * Writes signed 16-bit integer value.
     * @param value A value to write.
     */
    void Write(std::int16_t value);

    /**
     * Writes unsigned 16-bit integer value.
     * @param value A value to write.
     */
    void Write(std::uint16_t value);

    /**
     * Writes signed 32-bit integer value.
     * @param value A value to write.
     */
    void Write(std::int32_t value)
    {
        WriteVarint32(static_cast<std::uint32_t>(value));
    }

    /**
     * Writes unsigned 32-bit integer value.
     * @param value A value to write.
     */
    void Write(std::uint32_t value)
    {
        WriteVarint32(value);
    }

    /**
     * Writes signed 64-bit integer value.
     * @param value A value to write.
     */
    void Write(std::int64_t value)
    {
        WriteVarint64(static_cast<std::uint64_t>(value));
    }

    /**
     * Writes unsigned 64-bit integer value.
     * @param value A value to write.
     */
    void Write(std::uint64_t value)
    {
        WriteVarint64(value);
    }

    /**
     * Writes single precision floating-point value.
     * @param value A value to write.
     */
    void Write(float value);

    /**
     * Writes double precision floating-point value.
     * @param value A value to write.
     */
    void Write(double value);

    /**
     * Writes string value.
     * @param value A value to write.
     */
    void Write(const std::string& value);

    /**
     * Writes string value. Length is measured via std::strlen().
     * @param value A value to write.
     */
    void Write(const char* value);

    /**
     * Writes string value.
     * @param value A value to write.
     * @param length Value length in characters.
     */
    void Write(const char* value, std::size_t length);

    /**
     * Writes binary value.
     * @param value A value to write.
     */
    void Write(const BinaryValue& value);

private:
    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ExtendedCodedOutputStream);
};

}  // namespace siodb::protobuf
