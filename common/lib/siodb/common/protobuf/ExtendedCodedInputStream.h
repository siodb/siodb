// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
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
 * Custom coded input stream to read protocol data types.
 * As long as this is extension of the Google Protobuf class,
 * it follows lots of Google's design and coding conventions.
 */
class ExtendedCodedInputStream : public google::protobuf::io::CodedInputStream {
public:
    /**
     * Initializes object of class ExtendedCodedInputStream.
     * @param input Underlying input stream.
     */
    explicit ExtendedCodedInputStream(google::protobuf::io::ZeroCopyInputStream* input)
        : google::protobuf::io::CodedInputStream(input)
    {
    }

    /**
     * Initializes object of class ExtendedCodedInputStream.
     * @param buffer Input buffer.
     * @param size Buffer size.
     */
    ExtendedCodedInputStream(const std::uint8_t* buffer, int size)
        : google::protobuf::io::CodedInputStream(buffer, size)
    {
    }

    /**
     * Reads boolean value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadBool(bool* value);

    /**
     * Reads signed 8-bit integer value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadInt(std::int8_t* value);

    /**
     * Reads signed 16-bit integer value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadInt(std::int16_t* value);

    /**
     * Reads signed 32-bit integer value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadInt(std::int32_t* value);

    /**
     * Reads signed 64-bit integer value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadInt(std::int64_t* value);

    /**
     * Reads unsigned 8-bit integer value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadInt(std::uint8_t* value);

    /**
     * Reads unsigned 16-bit integer value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadInt(std::uint16_t* value);

    /**
     * Reads unsigned 32-bit integer value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadInt(std::uint32_t* value);

    /**
     * Reads unsigned 64-bit integer value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadInt(std::uint64_t* value);

    /**
     * Reads single precision floating-point value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadFloat(float* value);

    /**
     * Reads double precision floating-point value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadDouble(double* value);

    /**
     * Reads string value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadString(std::string* value);

    /**
     * Reads binary value.
     * @param value Address of varible to read value into.
     * @return true on success, false on read error.
     */
    bool ReadBinary(BinaryValue* value);

private:
    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ExtendedCodedInputStream);
};

}  // namespace siodb::protobuf
