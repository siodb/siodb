// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/data/RawDateTime.h>
#include <siodb/common/utils/BinaryValue.h>

// Protobuf headers
#include <google/protobuf/io/coded_stream.h>

namespace siodb::protobuf {

/**
 * Custom coded input stream to read protocol data types
 */
class CustomCodedInputStream : public google::protobuf::io::CodedInputStream {
public:
    /**
     * Initializes object of class CustomCodedInputStream
     * @param input Input stream.
     */
    explicit CustomCodedInputStream(google::protobuf::io::ZeroCopyInputStream* input)
        : google::protobuf::io::CodedInputStream(input)
    {
    }

    /**
     * Initializes object of class CustomCodedInputStream
     * @param buffer Input buffer.
     * @param size Buffer size
     */
    explicit CustomCodedInputStream(const std::uint8_t* buffer, int size)
        : google::protobuf::io::CodedInputStream(buffer, size)
    {
    }

    /** 
     * Reads binary value.
     * @return Binary value.
     * @throw runtime_error in case of error.
     */
    BinaryValue readBinary();

    /** 
     * Reads string value.
     * @return String value.
     * @throw runtime_error in case of error.
     */
    std::string readString();

    /** 
     * Reads binary value
     * @return Binary value
     * @throw runtime_error in case of error
     */
    std::int8_t readInt8();

    /** 
     * Reads std::int16_t value.
     * @return std::int16_t value.
     * @throw runtime_error in case of error.
     */
    std::int8_t readInt16();

    /** 
     * Reads std::int32_t value.
     * @return std::int32_t value.
     * @throw runtime_error in case of error.
     */
    std::int32_t readInt32();

    /** 
     * Reads std::int64_t value.
     * @return std::int64_t value.
     * @throw runtime_error in case of error.
     */
    std::int64_t readInt64();

    /** 
     * Reads std::uint8_t value.
     * @return std::uint8_t value.
     * @throw runtime_error in case of error.
     */
    std::uint8_t readUInt8();

    /** 
     * Reads std::uint16_t value.
     * @return std::uint16_t value.
     * @throw runtime_error in case of error.
     */
    std::uint8_t readUInt16();

    /** 
     * Reads std::uint32_t value.
     * @return std::uint32_t value.
     * @throw runtime_error in case of error.
     */
    std::uint32_t readUInt32();

    /** 
     * Reads std::uint64_t value.
     * @return std::uint64_t value.
     * @throw runtime_error in case of error.
     */
    std::uint64_t readUInt64();

    /** 
     * Reads float value.
     * @return Float value.
     * @throw runtime_error in case of error.
     */
    float readFloat();

    /** 
     * Reads double value.
     * @return Double value.
     * @throw runtime_error in case of error.
     */
    double readDouble();

    /** 
     * Reads bool value.
     * @return Bool value.
     * @throw runtime_error in case of error.
     */
    bool readBool();

private:
    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(CustomCodedInputStream);
};

}  // namespace siodb::protobuf
