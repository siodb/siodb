// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "OutputStream.h"

// Common project headers
#include "../utils/HelperMacros.h"

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
     * Writes begin of array.
     * @param name Field name, can be nullptr.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeArrayBegin(const char* name = nullptr, bool addCommaBefore = false);

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
    void writeObjectBegin(const char* name = nullptr, bool addCommaBefore = false);

    /**
     * Writes end of object.
     * @throw std::system_erorr on write error.
     */
    void writeObjectEnd();

    /**
     * Writes boolean field.
     * @param name Field name, can be nullptr.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, bool value, bool addCommaBefore = false);

    /**
     * Writes signed integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, int value, bool addCommaBefore = false);

    /**
     * Writes signed long integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, long value, bool addCommaBefore = false);

    /**
     * Writes signed long long integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, long long value, bool addCommaBefore = false);

    /**
     * Writes unsigned integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, unsigned int value, bool addCommaBefore = false);

    /**
     * Writes unsigned long integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, unsigned long value, bool addCommaBefore = false);

    /**
     * Writes unsigned long long integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, unsigned long long value, bool addCommaBefore = false);

    /**
     * Writes single precision floating point field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, float value, bool addCommaBefore = false);

    /**
     * Writes double precision floating point field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, double value, bool addCommaBefore = false);

    /**
     * Writes string field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, const char* value, bool addCommaBefore = false);

    /**
     * Writes string field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(const char* name, const std::string& value, bool addCommaBefore = false)
    {
        writeField(name, value.c_str(), value.length(), addCommaBefore);
    }

    /**
     * Writes string field.
     * @param name Field name.
     * @param value Field value.
     * @param length Field length.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeField(
            const char* name, const char* value, std::size_t length, bool addCommaBefore = false);

    /**
     * Writes raw string.
     * @param s A string.
     * @param length String length.
     * @throw std::system_erorr on write error.
     */
    void writeRawString(const char* s, std::size_t length);

    /**
     * Writes field name and delimiter.
     * @param name Field name.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     * @throw std::system_erorr on write error.
     */
    void writeFieldNameAndDelimiter(const char* name, bool addCommaBefore = false);

    /**
     * Writes raw data to the underlying stream.
     * @param buffer Data buffer.
     * @param size Data size.
     * @return Number of bytes written or negative value on error.
     */
    std::ptrdiff_t writeRaw(const void* buffer, std::size_t size)
    {
        return m_out.write(buffer, size);
    }

protected:
    /** Output stream */
    OutputStream& m_out;

protected:
    /** Opening double quote string */
    static constexpr const char* kOpeningDoubleQuote = "\"";

    /** Closing double quote and delimiter string */
    static constexpr const char* kClosingDoubleQuoteAndDelimiter = "\":";

    /** String output chunk size */
    static constexpr int kStringChunkSize = 4096;

    /** Json write error string */
    static constexpr const char* kJsonWriteError = "JSON write error";
};

}  // namespace siodb::io
