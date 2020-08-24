// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "OutputStream.h"

// Common project headers
#include "../utils/HelperMacros.h"

// CRT headers
#include <cstring>

// STL headers
#include <string>

namespace siodb::io {

/** Base class for JSON writers */
class JsonWriter {
public:
    /**
     * Initializes object of class JsonWriter.
     * @param out Output stream.
     */
    JsonWriter(OutputStream& out) noexcept
        : m_out(out)
    {
    }

    /**
     * Writes field name and delimiter.
     * @param name Field name.
     * @throw std::system_erorr on write error.
     */
    void writeFieldName(const char* name)
    {
        writeFieldName(name, std::strlen(name));
    }

    /**
     * Writes field name and delimiter.
     * @param name Field name.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeFieldName(const std::string& name)
    {
        writeFieldName(name.c_str(), name.length());
    }

    /**
     * Writes field name and delimiter.
     * @param name Field name.
     * @param length Field name length.
     * @throw std::system_erorr on write error.
     */
    void writeFieldName(const char* name, std::size_t length);

    /**
     * Writes begin of array.
     * @param name Field name, can be nullptr.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeArrayBegin();

    /**
     * Writes end of array.
     * @throw std::system_erorr on write error.
     */
    void writeArrayEnd();

    /**
     * Writes begin of array.
     * @param name Field name, can be nullptr.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeObjectBegin();

    /**
     * Writes end of object.
     * @throw std::system_erorr on write error.
     */
    void writeObjectEnd();

    /**
     * Writes double quote.
     * @throw std::system_erorr on write error.
     */
    void writeDoubleQuote();

    /**
     * Writes comma.
     * @throw std::system_erorr on write error.
     */
    void writeComma();

    /**
     * Writes field with null value.
     * @throw std::system_erorr on write error.
     */
    void writeNullValue();

    /**
     * Writes boolean field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(bool value);

    /**
     * Writes signed integer field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(int value);

    /**
     * Writes signed long integer field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(long value);

    /**
     * Writes signed long long integer field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(long long value);

    /**
     * Writes unsigned integer field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(unsigned int value);

    /**
     * Writes unsigned long integer field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(unsigned long value);

    /**
     * Writes unsigned long long integer field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(unsigned long long value);

    /**
     * Writes single precision floating point field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(float value);

    /**
     * Writes double precision floating point field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(double value);

    /**
     * Writes string field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(const char* value);

    /**
     * Writes string field.
     * @param value Field value.
     * @throw std::system_erorr on write error.
     */
    void writeValue(const std::string& value)
    {
        writeValue(value.c_str(), value.length());
    }

    /**
     * Writes string field.
     * @param value Field value.
     * @param length Field length.
     * @throw std::system_erorr on write error.
     */
    void writeValue(const char* value, std::size_t length);

    /**
     * Writes raw string.
     * @param s A string.
     * @param length String length.
     * @throw std::system_erorr on write error.
     */
    void writeRawString(const char* s, std::size_t length);

    /**
     * Writes raw bytes to the underlying stream.
     * @param buffer Data buffer.
     * @param size Data size.
     * @throw std::system_erorr on write error.
     */
    void writeBytes(const void* buffer, std::size_t size);

private:
    /** Reports JSON write error. */
    [[noreturn]] static void reportJsonWriteError();

private:
    /** Output stream */
    OutputStream& m_out;

    /** Opening double quote string */
    static constexpr const char* kDoubleQuote = "\"";

    /** String output chunk size */
    static constexpr int kStringChunkSize = 4096;

    /** Json write error string */
    static constexpr const char* kJsonWriteError = "JSON write error";
};

}  // namespace siodb::io
