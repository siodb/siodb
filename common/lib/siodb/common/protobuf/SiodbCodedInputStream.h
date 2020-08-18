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
 * Custom coded input stream to read protocol data types
 */
class SiodbCodedInputStream : public google::protobuf::io::CodedInputStream {
public:
    /**
     * Initializes object of class SiodbCodedInputStream
     * @param input Input stream.
     */
    explicit SiodbCodedInputStream(google::protobuf::io::ZeroCopyInputStream* input)
        : google::protobuf::io::CodedInputStream(input)
    {
    }

    /**
     * Initializes object of class SiodbCodedInputStream
     * @param buffer Input buffer.
     * @param size Buffer size
     */
    SiodbCodedInputStream(const std::uint8_t* buffer, int size)
        : google::protobuf::io::CodedInputStream(buffer, size)
    {
    }

    /** 
     * Reads boolean value.
     * @return Boolean value.
     * @throw std::runtime_error if IO error happens.
     */
    bool readBool();

    /** 
     * Reads binary value
     * @return Binary value
     * @throw std::runtime_error if IO error happens.
     */
    std::int8_t readInt8();

    /** 
     * Reads std::int16_t value.
     * @return std::int16_t value.
     * @throw std::runtime_error if IO error happens.
     */
    std::int8_t readInt16();

    /** 
     * Reads std::int32_t value.
     * @return std::int32_t value.
     * @throw std::runtime_error if IO error happens.
     */
    std::int32_t readInt32();

    /** 
     * Reads std::int64_t value.
     * @return std::int64_t value.
     * @throw std::runtime_error if IO error happens.
     */
    std::int64_t readInt64();

    /** 
     * Reads std::uint8_t value.
     * @return std::uint8_t value.
     * @throw std::runtime_error if IO error happens.
     */
    std::uint8_t readUInt8();

    /** 
     * Reads std::uint16_t value.
     * @return std::uint16_t value.
     * @throw std::runtime_error if IO error happens.
     */
    std::uint8_t readUInt16();

    /** 
     * Reads std::uint32_t value.
     * @return std::uint32_t value.
     * @throw std::runtime_error if IO error happens.
     */
    std::uint32_t readUInt32();

    /** 
     * Reads std::uint64_t value.
     * @return std::uint64_t value.
     * @throw std::runtime_error if IO error happens.
     */
    std::uint64_t readUInt64();

    /** 
     * Reads float value.
     * @return Float value.
     * @throw std::runtime_error if IO error happens.
     */
    float readFloat();

    /** 
     * Reads double value.
     * @return Double value.
     * @throw std::runtime_error if IO error happens.
     */
    double readDouble();

    /** 
     * Reads string value.
     * @return String value.
     * @throw std::runtime_error if IO error happens.
     */
    std::string readString();

    /** 
     * Reads binary value.
     * @return Binary value.
     * @throw std::runtime_error if IO error happens.
     */
    BinaryValue readBinary();

private:
    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(SiodbCodedInputStream);
};

}  // namespace siodb::protobuf
