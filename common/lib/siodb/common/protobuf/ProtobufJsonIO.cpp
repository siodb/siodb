// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ProtobufJsonIO.h"

// Common project headers
#include "../crt_ext/compiler_defs.h"
#include "../crt_ext/ct_string.h"

// CRT headers
#include <cfloat>
#include <cstdio>
#include <cstring>

namespace siodb::protobuf {

///////////////// class JsonWriter ////////////////////////////////////////////

void JsonWriter::writeField(const char* name, bool value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    if (value)
        m_out.WriteRaw("true", 4);
    else
        m_out.WriteRaw("false", 5);
}

void JsonWriter::writeField(const char* name, int value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    char buffer[16];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%d", value);
    m_out.WriteRaw(buffer, n);
}

void JsonWriter::writeField(const char* name, long value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%ld", value);
    m_out.WriteRaw(buffer, n);
}

void JsonWriter::writeField(const char* name, long long value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%lld", value);
    m_out.WriteRaw(buffer, n);
}

void JsonWriter::writeField(const char* name, unsigned int value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    char buffer[16];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%u", value);
    m_out.WriteRaw(buffer, n);
}

void JsonWriter::writeField(const char* name, unsigned long value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%lu", value);
    m_out.WriteRaw(buffer, n);
}

void JsonWriter::writeField(const char* name, unsigned long long value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%llu", value);
    m_out.WriteRaw(buffer, n);
}

void JsonWriter::writeField(const char* name, float value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    char buffer[16 + 3 + DBL_MANT_DIG - DBL_MIN_EXP];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%.7f", value);
    m_out.WriteRaw(buffer, n);
}

void JsonWriter::writeField(const char* name, double value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    char buffer[16 + 3 + DBL_MANT_DIG - DBL_MIN_EXP];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%.16f", value);
    m_out.WriteRaw(buffer, n);
}

void JsonWriter::writeField(
        const char* name, const char* value, std::size_t length, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.HadError())) return;

    m_out.WriteRaw(kOpeningDoubleQuote, ::ct_strlen(kOpeningDoubleQuote));
    if (SIODB_UNLIKELY(m_out.HadError())) return;

    writeRawString(value, length);
    if (SIODB_UNLIKELY(m_out.HadError())) return;

    m_out.WriteRaw(kClosingDoubleQuoteAndDelimiter, ::ct_strlen(kClosingDoubleQuoteAndDelimiter));
}

void JsonWriter::writeRawString(const char* s, std::size_t length)
{
    const auto end = s + length;
    auto p = s;

    char ubuffer[6];
    ubuffer[0] = '\\';
    ubuffer[1] = 'u';
    ubuffer[2] = '0';
    ubuffer[3] = '0';

    while (p != end) {
        if (p - s == kStringChunkSize) {
            m_out.WriteRaw(s, p - s);
            if (SIODB_UNLIKELY(m_out.HadError())) return;
            s = p;
        }

        const char c = *p;
        if (static_cast<unsigned char>(c) >= static_cast<unsigned char>(' ') && c != '/') {
            ++p;
            continue;
        }

        if (p != s) {
            m_out.WriteRaw(s, p - s);
            if (SIODB_UNLIKELY(m_out.HadError())) return;
            s = p;
        }

        switch (c) {
            case '\b': {
                m_out.WriteRaw(kBell, ::ct_strlen(kBell));
                break;
            }
            case '\f': {
                m_out.WriteRaw(kFormFeed, ::ct_strlen(kFormFeed));
                break;
            }
            case '\n': {
                m_out.WriteRaw(kLineFeed, ::ct_strlen(kLineFeed));
                break;
            }
            case '\r': {
                m_out.WriteRaw(kCarriageReturn, ::ct_strlen(kCarriageReturn));
                break;
            }
            case '\t': {
                m_out.WriteRaw(kHorizintalTab, ::ct_strlen(kHorizintalTab));
                break;
            }
            case '\v': {
                m_out.WriteRaw(kVerticalTab, ::ct_strlen(kVerticalTab));
                break;
            }
            case '/': {
                m_out.WriteRaw(kSlash, ::ct_strlen(kSlash));
                break;
            }
            default: {
                ubuffer[4] = kHexCharacters[c >> 4];
                ubuffer[5] = kHexCharacters[c & 15];
                m_out.WriteRaw(ubuffer, sizeof(ubuffer));
                break;
            }
        }
        if (SIODB_UNLIKELY(m_out.HadError())) return;
        s = ++p;
    }  // while

    if (p != s) m_out.WriteRaw(s, p - s);
}

void JsonWriter::writeFieldNameAndDelimiter(const char* name, bool addCommaBefore)
{
    if (addCommaBefore) {
        m_out.WriteRaw(kComma, ::ct_strlen(kComma));
        if (SIODB_UNLIKELY(m_out.HadError())) return;
    }
    m_out.WriteRaw(kOpeningDoubleQuote, ::ct_strlen(kOpeningDoubleQuote));
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    m_out.WriteRaw(name, std::strlen(name));
    if (SIODB_UNLIKELY(m_out.HadError())) return;
    m_out.WriteRaw(kClosingDoubleQuoteAndDelimiter, ::ct_strlen(kClosingDoubleQuoteAndDelimiter));
}

///////////////// class JsonWriteGuardBase ////////////////////////////////////

JsonWriteGuardBase::JsonWriteGuardBase(
        JsonWriter& jsonWriter, const char* fieldName, bool addCommaBefore)
    : m_jsonWriter(jsonWriter)
{
    if (fieldName)
        m_jsonWriter.writeFieldNameAndDelimiter(fieldName, addCommaBefore);
    else if (addCommaBefore)
        m_jsonWriter.writeRaw(kComma, ::ct_strlen(kComma));
}

}  // namespace siodb::protobuf
