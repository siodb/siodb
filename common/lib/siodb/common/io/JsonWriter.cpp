// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "JsonWriter.h"

// Common project headers
#include "../crt_ext/compiler_defs.h"
#include "../crt_ext/ct_string.h"
#include "../stl_ext/system_error_ext.h"

// STL headers
#include <iostream>

// CRT headers
#include <cfloat>
#include <cstdio>

namespace siodb::io {

void JsonWriter::writeFieldName(const char* name, std::size_t length)
{
    constexpr auto kDoubleQuoteLength = ::ct_strlen(kDoubleQuote);
    if (SIODB_UNLIKELY(m_out.write(kDoubleQuote, kDoubleQuoteLength) != kDoubleQuoteLength))
        reportJsonWriteError();
    writeRawString(name, length);
    constexpr const char* kDoubleQuoteAndDelimiter = "\":";
    constexpr auto kDoubleQuoteAndDelimiterLength = ::ct_strlen(kDoubleQuoteAndDelimiter);
    if (SIODB_UNLIKELY(m_out.write(kDoubleQuoteAndDelimiter, kDoubleQuoteAndDelimiterLength)
                       != kDoubleQuoteAndDelimiterLength)) {
        reportJsonWriteError();
    }
}

void JsonWriter::writeArrayBegin()
{
    constexpr const char* kArrayBegin = "[";
    constexpr auto kArrayBeginLength = ::ct_strlen(kArrayBegin);
    if (SIODB_UNLIKELY(m_out.write(kArrayBegin, kArrayBeginLength) != kArrayBeginLength))
        reportJsonWriteError();
}

void JsonWriter::writeArrayEnd()
{
    constexpr const char* kArrayEnd = "]";
    constexpr auto kArrayEndLength = ::ct_strlen(kArrayEnd);
    if (SIODB_UNLIKELY(m_out.write(kArrayEnd, kArrayEndLength) != kArrayEndLength))
        reportJsonWriteError();
}

void JsonWriter::writeObjectBegin()
{
    constexpr const char* kObjectBegin = "{";
    constexpr auto kObjectBeginLength = ::ct_strlen(kObjectBegin);
    if (SIODB_UNLIKELY(m_out.write(kObjectBegin, kObjectBeginLength) != kObjectBeginLength))
        reportJsonWriteError();
}

void JsonWriter::writeObjectEnd()
{
    constexpr const char* kObjectEnd = "}";
    constexpr auto kObjectEndLength = ::ct_strlen(kObjectEnd);
    if (SIODB_UNLIKELY(m_out.write(kObjectEnd, kObjectEndLength) != kObjectEndLength))
        reportJsonWriteError();
}

void JsonWriter::writeDoubleQuote()
{
    constexpr auto kDoubleQuoteLength = ::ct_strlen(kDoubleQuote);
    if (SIODB_UNLIKELY(m_out.write(kDoubleQuote, kDoubleQuoteLength) != kDoubleQuoteLength))
        reportJsonWriteError();
}

void JsonWriter::writeComma()
{
    constexpr const char* kComma = ",";
    constexpr const auto kCommaLength = ::ct_strlen(kComma);
    if (SIODB_UNLIKELY(m_out.write(kComma, kCommaLength) != kCommaLength)) reportJsonWriteError();
}

void JsonWriter::writeNullValue()
{
    constexpr const char* kNull = "null";
    constexpr auto kNullLength = ::ct_strlen(kNull);
    if (SIODB_UNLIKELY(m_out.write(kNull, kNullLength) != kNullLength)) reportJsonWriteError();
}

void JsonWriter::writeValue(bool value)
{
    constexpr const char* kTrue = "true";
    constexpr const char* kFalse = "false";
    constexpr auto kTrueLength = ::ct_strlen(kTrue);
    constexpr auto kFalseLength = ::ct_strlen(kFalse);
    if (SIODB_UNLIKELY(value ? m_out.write(kTrue, kTrueLength) != kTrueLength
                             : m_out.write(kFalse, kFalseLength) != kFalseLength)) {
        reportJsonWriteError();
    }
}

void JsonWriter::writeValue(int value)
{
    char buffer[16];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%d", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) reportJsonWriteError();
}

void JsonWriter::writeValue(long value)
{
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%ld", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) reportJsonWriteError();
}

void JsonWriter::writeValue(long long value)
{
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%lld", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) reportJsonWriteError();
}

void JsonWriter::writeValue(unsigned int value)
{
    char buffer[16];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%u", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) reportJsonWriteError();
}

void JsonWriter::writeValue(unsigned long value)
{
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%lu", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) reportJsonWriteError();
}

void JsonWriter::writeValue(unsigned long long value)
{
    char buffer[32];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%llu", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) reportJsonWriteError();
}

void JsonWriter::writeValue(float value)
{
    char buffer[DBL_MANT_DIG - DBL_MIN_EXP + 20];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%.7f", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) reportJsonWriteError();
}

void JsonWriter::writeValue(double value)
{
    char buffer[DBL_MANT_DIG - DBL_MIN_EXP + 20];
    const auto n = std::snprintf(buffer, sizeof(buffer), "%.16f", value);
    if (SIODB_UNLIKELY(m_out.write(buffer, n) != n)) reportJsonWriteError();
}

void JsonWriter::writeValue(const char* value, std::size_t length)
{
    constexpr auto kDoubleQuoteLength = ::ct_strlen(kDoubleQuote);
    if (SIODB_UNLIKELY(m_out.write(kDoubleQuote, kDoubleQuoteLength) != kDoubleQuoteLength))
        reportJsonWriteError();
    writeRawString(value, length);
    if (SIODB_UNLIKELY(m_out.write(kDoubleQuote, kDoubleQuoteLength) != kDoubleQuoteLength))
        reportJsonWriteError();
}

void JsonWriter::writeRawString(const char* s, std::size_t length)
{
    const auto end = s + length;
    std::cout << "s=" << s << '\n';
    std::cout << "length=" << length << '\n';
    std::cout << "end=" << end << '\n';
    auto p = s;
    std::cout << "p=" << p << '\n';

    char ubuffer[6];
    ubuffer[0] = '\\';
    ubuffer[1] = 'u';
    ubuffer[2] = '0';
    ubuffer[3] = '0';

    bool previousCharIsEscaped = false;
    while (p != end) {
        std::cout << "enter while p=" << p << '\n';
        std::cout << "enter while s=" << s << '\n';
        if (p - s == kStringChunkSize) {
            if (SIODB_UNLIKELY(m_out.write(s, kStringChunkSize) != kStringChunkSize))
                reportJsonWriteError();
            s = p;
        }

        const char c = *p;
        std::cout << "previousCharIsEscaped=" << previousCharIsEscaped << ">" << c << '\n';
        if (static_cast<unsigned char>(c) >= static_cast<unsigned char>(' ') && c != '"'
                && c != '\\' && c != 'b' && c != 'f' && c != 'n' && c != 'r' && c != 't') {
            if (previousCharIsEscaped) {
                constexpr const char* kBackSlash = "\\";
                constexpr auto kBackSlashLength = ::ct_strlen(kBackSlash);
                if (SIODB_UNLIKELY(m_out.write(kBackSlash, kBackSlashLength) != kBackSlashLength))
                    reportJsonWriteError();
                previousCharIsEscaped = false;
            }
            ++p;
            continue;
        }

        std::cout << "BEFORE1" << '\n';
        if (p != s) {
            const auto n = p - s;
            if (SIODB_UNLIKELY(m_out.write(s, n) != n)) reportJsonWriteError();
            s = p;
        }

        std::cout << "BEFORE2" << '\n';
        if (!previousCharIsEscaped && c == '\\') {
            constexpr const char* kBackSlash = "\\";
            constexpr auto kBackSlashLength = ::ct_strlen(kBackSlash);
            if (SIODB_UNLIKELY(m_out.write(kBackSlash, kBackSlashLength) != kBackSlashLength))
                reportJsonWriteError();
            previousCharIsEscaped = true;
            s = ++p;

            // Escape last '\' if not yet escaped
            if (p == end) {
                constexpr const char* kBackSlash = "\\";
                constexpr auto kBackSlashLength = ::ct_strlen(kBackSlash);
                if (SIODB_UNLIKELY(m_out.write(kBackSlash, kBackSlashLength) != kBackSlashLength))
                    reportJsonWriteError();
                previousCharIsEscaped = true;
            }
            continue;
        }
        if (previousCharIsEscaped && c != '"' && c != '\\' && c != 'b' && c != 'f' && c != 'n'
                && c != 'r' && c != 't') {
            constexpr const char* kBackSlash = "\\";
            constexpr auto kBackSlashLength = ::ct_strlen(kBackSlash);
            if (SIODB_UNLIKELY(m_out.write(kBackSlash, kBackSlashLength) != kBackSlashLength))
                reportJsonWriteError();
        }
        std::cout << "AFTER" << '\n';

        switch (c) {
            case '\b': {
                constexpr const char* kBell = "\\b";
                constexpr auto kBellLength = ::ct_strlen(kBell);
                if (SIODB_UNLIKELY(m_out.write(kBell, kBellLength) != kBellLength))
                    reportJsonWriteError();
                break;
            }

            case 'b': {
                constexpr const char* kLetterB = "b";
                constexpr auto kLetterBLength = ::ct_strlen(kLetterB);
                if (SIODB_UNLIKELY(m_out.write(kLetterB, kLetterBLength) != kLetterBLength))
                    reportJsonWriteError();
                break;
            }

            case '\f': {
                constexpr const char* kFormFeed = "\\f";
                constexpr auto kFormFeedLength = ::ct_strlen(kFormFeed);
                if (SIODB_UNLIKELY(m_out.write(kFormFeed, kFormFeedLength) != kFormFeedLength))
                    reportJsonWriteError();
                break;
            }

            case 'f': {
                constexpr const char* kLetterF = "f";
                constexpr auto kLetterFLength = ::ct_strlen(kLetterF);
                if (SIODB_UNLIKELY(m_out.write(kLetterF, kLetterFLength) != kLetterFLength))
                    reportJsonWriteError();
                break;
            }

            case '\n': {
                constexpr const char* kLineFeed = "\\n";
                constexpr auto kLineFeedLength = ::ct_strlen(kLineFeed);
                if (SIODB_UNLIKELY(m_out.write(kLineFeed, kLineFeedLength) != kLineFeedLength))
                    reportJsonWriteError();
                break;
            }

            case 'n': {
                constexpr const char* kLetterN = "n";
                constexpr auto kLetterNLength = ::ct_strlen(kLetterN);
                if (SIODB_UNLIKELY(m_out.write(kLetterN, kLetterNLength) != kLetterNLength))
                    reportJsonWriteError();
                break;
            }

            case '\r': {
                constexpr const char* kCarriageReturn = "\\r";
                constexpr auto kCarriageReturnLength = ::ct_strlen(kCarriageReturn);
                if (SIODB_UNLIKELY(m_out.write(kCarriageReturn, kCarriageReturnLength)
                                   != kCarriageReturnLength))
                    reportJsonWriteError();
                break;
            }

            case 'r': {
                constexpr const char* kLetterR = "r";
                constexpr auto kLetterRLength = ::ct_strlen(kLetterR);
                if (SIODB_UNLIKELY(m_out.write(kLetterR, kLetterRLength) != kLetterRLength))
                    reportJsonWriteError();
                break;
            }

            case '\t': {
                constexpr const char* kHorizintalTab = "\\t";
                constexpr auto kHorizintalTabLength = ::ct_strlen(kHorizintalTab);
                if (SIODB_UNLIKELY(m_out.write(kHorizintalTab, kHorizintalTabLength)
                                   != kHorizintalTabLength))
                    reportJsonWriteError();
                break;
            }

            case 't': {
                constexpr const char* kLetterT = "t";
                constexpr auto kLetterTLength = ::ct_strlen(kLetterT);
                if (SIODB_UNLIKELY(m_out.write(kLetterT, kLetterTLength) != kLetterTLength))
                    reportJsonWriteError();
                break;
            }

            case '\v': {
                constexpr const char* kVerticalTab = "\\v";
                constexpr auto kVerticalTabLength = ::ct_strlen(kVerticalTab);
                if (SIODB_UNLIKELY(
                            m_out.write(kVerticalTab, kVerticalTabLength) != kVerticalTabLength))
                    reportJsonWriteError();
                break;
            }

            case '"': {
                constexpr const char* kDoubleQuoteEscaped = "\\\"";
                constexpr auto kDoubleQuoteEscapedLength = ::ct_strlen(kDoubleQuoteEscaped);
                if (previousCharIsEscaped) {
                    constexpr auto kDoubleQuoteLength = ::ct_strlen(kDoubleQuote);
                    if (SIODB_UNLIKELY(m_out.write(kDoubleQuote, kDoubleQuoteLength)
                                       != kDoubleQuoteEscapedLength))
                        reportJsonWriteError();
                } else {
                    if (SIODB_UNLIKELY(m_out.write(kDoubleQuoteEscaped, kDoubleQuoteEscapedLength)
                                       != kDoubleQuoteEscapedLength))
                        reportJsonWriteError();
                }
                break;
            }

            default: {
                constexpr const char* kHexCharacters = "0123456789ABCDEF";
                ubuffer[4] = kHexCharacters[c >> 4];
                ubuffer[5] = kHexCharacters[c & 15];
                if (SIODB_UNLIKELY(m_out.write(ubuffer, sizeof(ubuffer)) != sizeof(ubuffer)))
                    reportJsonWriteError();
                break;
            }
        }
        s = ++p;
        previousCharIsEscaped = false;
    }  // while

    if (p != s) {
        const auto n = p - s;
        if (SIODB_UNLIKELY(m_out.write(s, n) != n)) reportJsonWriteError();
    }
}

void JsonWriter::writeBytes(const void* buffer, std::size_t size)
{
    if (SIODB_UNLIKELY(static_cast<std::size_t>(m_out.write(buffer, size)) != size))
        reportJsonWriteError();
}

// ----- internals -----

[[noreturn]] void JsonWriter::reportJsonWriteError()
{
    stdext::throw_system_error(kJsonWriteError);
}

}  // namespace siodb::io
