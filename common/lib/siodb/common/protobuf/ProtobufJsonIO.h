// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../utils/HelperMacros.h"

// STL headers
#include <string>

// Protobuf headers
#include <google/protobuf/io/coded_stream.h>

namespace siodb::protobuf {

/** Base class for JSON writers */
class JsonWriter {
public:
    /**
     * Initializes object of class JsonWriter.
     * @param out Output stream.
     */
    JsonWriter(google::protobuf::io::CodedOutputStream& out) noexcept
        : m_out(out)
    {
    }

    /**
     * Writes boolean field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, bool value, bool addCommaBefore = false);

    /**
     * Writes signed integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, int value, bool addCommaBefore = false);

    /**
     * Writes signed long integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, long value, bool addCommaBefore = false);

    /**
     * Writes signed long long integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, long long value, bool addCommaBefore = false);

    /**
     * Writes unsigned integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, unsigned int value, bool addCommaBefore = false);

    /**
     * Writes unsigned long integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, unsigned long value, bool addCommaBefore = false);

    /**
     * Writes unsigned long long integer field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, unsigned long long value, bool addCommaBefore = false);

    /**
     * Writes single precision floating point field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, float value, bool addCommaBefore = false);

    /**
     * Writes double precision floating point field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, double value, bool addCommaBefore = false);

    /**
     * Writes string field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, const char* value, bool addCommaBefore = false);

    /**
     * Writes string field.
     * @param name Field name.
     * @param value Field value.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, const std::string& value, bool addCommaBefore = false);

    /**
     * Writes string field.
     * @param name Field name.
     * @param value Field value.
     * @param length Field length.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeField(const char* name, const char* value, std::size_t length, bool addCommaBefore);

    /**
     * Writes raw string.
     * @param s A string.
     * @param length String length.
     */
    void writeRawString(const char* s, std::size_t length);

    /**
     * Writes field name and delimiter
     * @param name Field name.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    void writeFieldNameAndDelimiter(const char* name, bool addCommaBefore = false);

    /**
     * Writes raw data to the underlying stream.
     * @param buffer Data buffer.
     * @param size Data size.
     */
    void writeRaw(const void* buffer, int size)
    {
        m_out.WriteRaw(buffer, size);
    }

protected:
    /** Output stream */
    google::protobuf::io::CodedOutputStream& m_out;

protected:
    /** Comma string */
    static constexpr const char* kComma = ",";

    /** Opening double quote string */
    static constexpr const char* kOpeningDoubleQuote = "\"";

    /** Closing double quote and delimiter string */
    static constexpr const char* kClosingDoubleQuoteAndDelimiter = "\":";

    /** String output chunk size */
    static constexpr int kStringChunkSize = 4096;

    /** BELL string */
    static constexpr const char* kBell = "\\b";

    /** FORM FEED string */
    static constexpr const char* kFormFeed = "\\f";

    /** LINE FEED string */
    static constexpr const char* kLineFeed = "\\n";

    /** CARRIAGE RETURN string */
    static constexpr const char* kCarriageReturn = "\\r";

    /** HORIZONTAL TAB string */
    static constexpr const char* kHorizintalTab = "\\t";

    /** VERTICAL TAB string */
    static constexpr const char* kVerticalTab = "\\v";

    /** SLASH string */
    static constexpr const char* kSlash = "\\/";

    /** Hex characters */
    static constexpr const char* kHexCharacters = "0123456789ABCDEF";
};

/** Base class for JSON write guards */
class JsonWriteGuardBase {
protected:
    /**
     * Writes opening part of the object.
     * @param jsonWriter JSON writer.
     * @param fieldName Optional field name.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    JsonWriteGuardBase(JsonWriter& jsonWriter, const char* fieldName, bool addCommaBefore);

public:
    DECLARE_NONCOPYABLE(JsonWriteGuardBase);

protected:
    JsonWriter& m_jsonWriter;

protected:
    /** Comma string */
    static constexpr const char* kComma = ",";
};

/** Compound JSON entity write guard */
template<char Opening, char Closing>
class JsonCompoundEntityWriteGuard : public JsonWriteGuardBase {
public:
    /**
     * Writes opening part of the object.
     * @param jsonWriter JSON writer.
     * @param fieldName Optional field name.
     * @param addCommaBefore Indicates that comma needs to be added before the field name.
     */
    JsonCompoundEntityWriteGuard(
            JsonWriter& jsonWriter, const char* fieldName = nullptr, bool addCommaBefore = false)
        : JsonWriteGuardBase(jsonWriter, fieldName, addCommaBefore)
    {
        const char c = Opening;
        m_jsonWriter.writeRaw(&c, 1);
    }

    /** Writes closing part of the object */
    ~JsonCompoundEntityWriteGuard()
    {
        const char c = Closing;
        m_jsonWriter.writeRaw(&c, 1);
    }
};

/** JSON object write guard */
using JsonObjectWriteGuard = JsonCompoundEntityWriteGuard<'{', '}'>;

/** JSON object write guard */
using JsonArrayWriteGuard = JsonCompoundEntityWriteGuard<'[', ']'>;

}  // namespace siodb::protobuf
