// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "JsonWriter.h"

// Common project headers
#include "../crt_ext/compiler_defs.h"
#include "../crt_ext/ct_string.h"
#include "../stl_ext/system_error_ext.h"

// CRT headers
#include <cfloat>
#include <cstdio>
#include <cstring>

namespace siodb::io {

void JsonWriter::writeArrayBegin(const char* name, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    constexpr const char* kArrayBegin = "[";
    if (SIODB_UNLIKELY(
                m_out.write(kArrayBegin, ::ct_strlen(kArrayBegin)) != ::ct_strlen(kArrayBegin)))
        stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeArrayEnd()
{
    constexpr const char* kArrayEnd = "]";
    if (SIODB_UNLIKELY(m_out.write(kArrayEnd, ::ct_strlen(kArrayEnd)) != ::ct_strlen(kArrayEnd)))
        stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeObjectBegin(const char* name, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    constexpr const char* kObjectBegin = "{";
    if (SIODB_UNLIKELY(
                m_out.write(kObjectBegin, ::ct_strlen(kObjectBegin)) != ::ct_strlen(kObjectBegin)))
        stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeObjectEnd()
{
    constexpr const char* kObjectEnd = "}";
    if (SIODB_UNLIKELY(m_out.write(kObjectEnd, ::ct_strlen(kObjectEnd)) != ::ct_strlen(kObjectEnd)))
        stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeField(const char* name, bool value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    constexpr const char* kTrue = "true";
    constexpr const char* kFalse = "false";
    if (SIODB_UNLIKELY(value ? m_out.write(kTrue, ::ct_strlen(kTrue)) != ::ct_strlen(kTrue)
                             : m_out.write(kFalse, ::ct_strlen(kFalse)) != ::ct_strlen(kFalse))) {
        stdext::throw_system_error(kJsonWriteError);
    }
}

void JsonWriter::writeField(const char* name, int value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    char buffer[16];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%d", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeField(const char* name, long value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%ld", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeField(const char* name, long long value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%lld", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeField(const char* name, unsigned int value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    char buffer[16];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%u", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeField(const char* name, unsigned long value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%lu", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeField(const char* name, unsigned long long value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%llu", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeField(const char* name, float value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    char buffer[16 + 3 + DBL_MANT_DIG - DBL_MIN_EXP];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%.7f", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeField(const char* name, double value, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    char buffer[16 + 3 + DBL_MANT_DIG - DBL_MIN_EXP];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%.16f", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) stdext::throw_system_error(kJsonWriteError);
}

void JsonWriter::writeField(
        const char* name, const char* value, std::size_t length, bool addCommaBefore)
{
    writeFieldNameAndDelimiter(name, addCommaBefore);
    if (SIODB_UNLIKELY(m_out.write(kOpeningDoubleQuote, ::ct_strlen(kOpeningDoubleQuote))
                       != ::ct_strlen(kOpeningDoubleQuote)))
        stdext::throw_system_error(kJsonWriteError);
    writeRawString(value, length);
    if (SIODB_UNLIKELY(m_out.write(kClosingDoubleQuoteAndDelimiter,
                               ::ct_strlen(kClosingDoubleQuoteAndDelimiter))
                       != ::ct_strlen(kClosingDoubleQuoteAndDelimiter))) {
        stdext::throw_system_error(kJsonWriteError);
    }
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
            if (SIODB_UNLIKELY(m_out.write(s, kStringChunkSize) != kStringChunkSize))
                stdext::throw_system_error(kJsonWriteError);
            s = p;
        }

        const char c = *p;
        if (static_cast<unsigned char>(c) >= static_cast<unsigned char>(' ') && c != '/') {
            ++p;
            continue;
        }

        if (p != s) {
            const auto n = p - s;
            if (SIODB_UNLIKELY(m_out.write(s, n) != n)) stdext::throw_system_error(kJsonWriteError);
            s = p;
        }

        switch (c) {
            case '\b': {
                constexpr const char* kBell = "\\b";
                if (SIODB_UNLIKELY(m_out.write(kBell, ::ct_strlen(kBell)) != ::ct_strlen(kBell)))
                    stdext::throw_system_error(kJsonWriteError);
                break;
            }
            case '\f': {
                constexpr const char* kFormFeed = "\\f";
                if (SIODB_UNLIKELY(m_out.write(kFormFeed, ::ct_strlen(kFormFeed))
                                   != ::ct_strlen(kFormFeed)))
                    stdext::throw_system_error(kJsonWriteError);
                break;
            }
            case '\n': {
                constexpr const char* kLineFeed = "\\n";
                if (SIODB_UNLIKELY(m_out.write(kLineFeed, ::ct_strlen(kLineFeed))
                                   != ::ct_strlen(kLineFeed)))
                    stdext::throw_system_error(kJsonWriteError);
                break;
            }
            case '\r': {
                constexpr const char* kCarriageReturn = "\\r";
                if (SIODB_UNLIKELY(m_out.write(kCarriageReturn, ::ct_strlen(kCarriageReturn))
                                   != ::ct_strlen(kCarriageReturn)))
                    stdext::throw_system_error(kJsonWriteError);
                break;
            }
            case '\t': {
                constexpr const char* kHorizintalTab = "\\t";
                if (SIODB_UNLIKELY(m_out.write(kHorizintalTab, ::ct_strlen(kHorizintalTab))
                                   != ::ct_strlen(kHorizintalTab)))
                    stdext::throw_system_error(kJsonWriteError);
                break;
            }
            case '\v': {
                constexpr const char* kVerticalTab = "\\v";
                if (SIODB_UNLIKELY(m_out.write(kVerticalTab, ::ct_strlen(kVerticalTab))
                                   != ::ct_strlen(kVerticalTab)))
                    stdext::throw_system_error(kJsonWriteError);
                break;
            }
            case '/': {
                constexpr const char* kSlash = "\\/";
                if (SIODB_UNLIKELY(m_out.write(kSlash, ::ct_strlen(kSlash)) != ::ct_strlen(kSlash)))
                    stdext::throw_system_error(kJsonWriteError);
                break;
            }
            default: {
                constexpr const char* kHexCharacters = "0123456789ABCDEF";
                ubuffer[4] = kHexCharacters[c >> 4];
                ubuffer[5] = kHexCharacters[c & 15];
                if (SIODB_UNLIKELY(m_out.write(ubuffer, sizeof(ubuffer)) != sizeof(ubuffer)))
                    stdext::throw_system_error(kJsonWriteError);
                break;
            }
        }
        s = ++p;
    }  // while

    if (p != s) {
        const auto n = p - s;
        if (SIODB_UNLIKELY(m_out.write(s, n) != n)) stdext::throw_system_error(kJsonWriteError);
    }
}

void JsonWriter::writeFieldNameAndDelimiter(const char* name, bool addCommaBefore)
{
    if (addCommaBefore) {
        constexpr const char* kComma = ",";
        if (SIODB_UNLIKELY(m_out.write(kComma, ::ct_strlen(kComma)) != ::ct_strlen(kComma)))
            stdext::throw_system_error(kJsonWriteError);
    }

    if (name) {
        if (SIODB_UNLIKELY(m_out.write(kOpeningDoubleQuote, ::ct_strlen(kOpeningDoubleQuote))
                           != ::ct_strlen(kOpeningDoubleQuote)))
            stdext::throw_system_error(kJsonWriteError);
        const std::ptrdiff_t n = std::strlen(name);
        if (SIODB_UNLIKELY(m_out.write(name, n) != n)) stdext::throw_system_error(kJsonWriteError);
        if (SIODB_UNLIKELY(m_out.write(kClosingDoubleQuoteAndDelimiter,
                                   ::ct_strlen(kClosingDoubleQuoteAndDelimiter))
                           != ::ct_strlen(kClosingDoubleQuoteAndDelimiter)))
            stdext::throw_system_error(kJsonWriteError);
    }
}

}  // namespace siodb::io
