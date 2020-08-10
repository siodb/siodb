// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RawDateTime.h"

// Project headers
#include "../config/Config.h"
#include "../stl_ext/chrono_ext.h"
#include "../stl_ext/ostream_ext.h"
#include "../utils/PlainBinaryEncoding.h"

// CRT headers
#include <cstdio>

// STL headers
#include <array>

// System headers
#ifndef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
#include <byteswap.h>
#include <endian.h>
#endif

// Boost headers
#include <boost/date_time.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

// Date headers
#include <date/date.h>

namespace siodb {

namespace {

constexpr std::array<const char*, 7> kDayOfWeekShortNames {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat",
};

constexpr std::array<const char*, 12> kMonthShortName {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec",
};

}  // anonymous namespace

///////////////////// struct RawDate ///////////////////////////////////////

RawDate::RawDate(std::time_t t) noexcept
{
    std::tm tm;
    gmtime_r(&t, &tm);
    m_hasTimePart = false;
    m_dayOfWeek = tm.tm_wday;
    m_dayOfMonth = tm.tm_mday - 1;
    m_month = tm.tm_mon;
    m_year = tm.tm_year + 1900;
}

bool RawDate::operator==(const RawDate& other) const noexcept
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return *reinterpret_cast<const std::uint32_t*>(this)
           == *reinterpret_cast<const std::uint32_t*>(&other);
#else
    return std::memcmp(this, &other, 4) == 0;
#endif
}

bool RawDate::operator!=(const RawDate& other) const noexcept
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return *reinterpret_cast<const std::uint32_t*>(this)
           != *reinterpret_cast<const std::uint32_t*>(&other);
#else
    return std::memcmp(this, &other, 4) != 0;
#endif
}

bool RawDate::operator>(const RawDate& other) const noexcept
{
    if (m_year > other.m_year)
        return true;
    else if (m_year < other.m_year)
        return false;

    if (m_month > other.m_month)
        return true;
    else if (m_month < other.m_month)
        return false;

    return m_dayOfMonth > other.m_dayOfMonth;
}

bool RawDate::operator>=(const RawDate& other) const noexcept
{
    if (m_year > other.m_year)
        return true;
    else if (m_year < other.m_year)
        return false;

    if (m_month > other.m_month)
        return true;
    else if (m_month < other.m_month)
        return false;

    return m_dayOfMonth >= other.m_dayOfMonth;
}

std::time_t RawDate::toEpochTimestamp() const noexcept
{
    std::tm tm {0, 0, 0, static_cast<int>(m_dayOfMonth + 1), static_cast<int>(m_month),
            static_cast<int>(m_year) - 1900, 0, 0, 0, 0, 0};
    return std::mktime(&tm);
}

///////////////////// struct RawTime ///////////////////////////////////////

RawTime::RawTime(std::time_t t) noexcept
{
    std::tm tm;
    gmtime_r(&t, &tm);
    m_reserved1 = 0;
    m_nanos = 0;
    m_seconds = tm.tm_sec;
    m_minutes = tm.tm_min;
    m_hours = tm.tm_hour;
    m_reserved2 = 0;
}

bool RawTime::operator==(const RawTime& other) const noexcept
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return *reinterpret_cast<const std::uint64_t*>(this)
           == *reinterpret_cast<const std::uint64_t*>(&other);
#else
    return std::memcmp(this, &other, 8) == 0;
#endif
}

bool RawTime::operator!=(const RawTime& other) const noexcept
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    return *reinterpret_cast<const std::uint64_t*>(this)
           != *reinterpret_cast<const std::uint64_t*>(&other);
#else
    return std::memcmp(this, &other, 8) != 0;
#endif
}

bool RawTime::operator>(const RawTime& other) const noexcept
{
    if (m_hours > other.m_hours)
        return true;
    else if (m_hours < other.m_hours)
        return false;

    if (m_minutes > other.m_minutes)
        return true;
    else if (m_minutes < other.m_minutes)
        return false;

    if (m_seconds > other.m_seconds)
        return true;
    else if (m_seconds < other.m_seconds)
        return false;

    return m_nanos > other.m_nanos;
}

bool RawTime::operator>=(const RawTime& other) const noexcept
{
    if (m_hours > other.m_hours)
        return true;
    else if (m_hours < other.m_hours)
        return false;

    if (m_minutes > other.m_minutes)
        return true;
    else if (m_minutes < other.m_minutes)
        return false;

    if (m_seconds > other.m_seconds)
        return true;
    else if (m_seconds < other.m_seconds)
        return false;

    return m_nanos >= other.m_nanos;
}

std::time_t RawTime::toEpochTimestamp() const noexcept
{
    return static_cast<std::time_t>(m_seconds) + static_cast<std::time_t>(60) * m_minutes
           + static_cast<std::time_t>(3600) * m_hours;
}

///////////////////// struct RawDateTime ///////////////////////////////////////

RawDateTime::RawDateTime(std::time_t t) noexcept
{
    std::tm tm;
    gmtime_r(&t, &tm);
    m_datePart.m_hasTimePart = true;
    m_datePart.m_dayOfWeek = tm.tm_wday;
    m_datePart.m_dayOfMonth = tm.tm_mday - 1;
    m_datePart.m_month = tm.tm_mon;
    m_datePart.m_year = tm.tm_year + 1900;
    m_timePart.m_reserved1 = 0;
    m_timePart.m_nanos = 0;
    m_timePart.m_seconds = tm.tm_sec;
    m_timePart.m_minutes = tm.tm_min;
    m_timePart.m_hours = tm.tm_hour;
    m_timePart.m_reserved2 = 0;
}

std::uint8_t* RawDateTime::serialize(std::uint8_t* buffer) const noexcept
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS

    // x86 implementation

    std::uint64_t timePart;
    {
        // Make GCC happy: respect strict aliasing rules
        const auto p = reinterpret_cast<const std::uint64_t*>(&m_timePart);
        timePart = *p & 0xFFFFFFFFFFFFULL;
    }

    RawDate rawDatePart = m_datePart;
    rawDatePart.m_hasTimePart = (timePart != 0);
    {
        // Make GCC happy: respect strict aliasing rules
        const auto p = reinterpret_cast<const std::uint32_t*>(&rawDatePart);
        *reinterpret_cast<std::uint32_t*>(buffer) = *p;
    }

    if (timePart == 0) return buffer + 4;

    {
        // Make GCC happy: respect strict aliasing rules
        const auto p = reinterpret_cast<const std::uint32_t*>(&timePart);
        *reinterpret_cast<std::uint32_t*>(buffer + 4) = *p;
    }

    *reinterpret_cast<std::uint16_t*>(buffer + 8) = *reinterpret_cast<const std::uint16_t*>(
            reinterpret_cast<const std::uint8_t*>(&timePart) + 4);
    return buffer + 10;

#else

    // Generic implementation

    std::uint64_t timePart = 0;
    ::pbeDecodeUInt64(reinterpret_cast<const std::uint8_t*>(&m_timePart), &timePart);
#if __BYTE_ORDER == __BIG_ENDIAN
    timePart = bswap64(timePart);
#endif  // __BYTE_ORDER
    timePart &= 0xFFFFFFFFFFFFULL;
    RawDate rawDatePart = m_datePart;
    if (timePart != 0) rawDatePart.m_hasTimePart = 1;
    std::uint32_t datePart = 0;
    ::pbeDecodeUInt32(reinterpret_cast<const std::uint8_t*>(&rawDatePart), &datePart);
#if __BYTE_ORDER == __BIG_ENDIAN
    datePart = bswap32(datePart);
#endif  // __BYTE_ORDER
    ::pbeEncodeUInt32(datePart, buffer);

    if (timePart == 0) return buffer + 4;

    std::uint8_t tempBuffer[8];
    ::pbeEncodeUInt64(timePart, tempBuffer);
    buffer[4] = tempBuffer[0];
    buffer[5] = tempBuffer[1];
    buffer[6] = tempBuffer[2];
    buffer[7] = tempBuffer[3];
    buffer[8] = tempBuffer[4];
    buffer[9] = tempBuffer[5];
    return buffer + 10;

#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
}

int RawDateTime::deserialize(const std::uint8_t* buffer, std::size_t length) noexcept
{
    if (SIODB_UNLIKELY(length < 4)) return 0;

#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS

    // x86 implementation

    m_datePart = *reinterpret_cast<const RawDate*>(buffer);
    if (!m_datePart.m_hasTimePart) {
        // Make GCC happy: respect strict aliasing rules
        const auto p = reinterpret_cast<std::uint64_t*>(&m_timePart);
        *p = 0;
        return 4;
    }

    if (SIODB_UNLIKELY(length < 10)) return 0;

    {
        // Make GCC happy: respect strict aliasing rules
        const auto p = reinterpret_cast<std::uint32_t*>(&m_timePart);
        *p = *reinterpret_cast<const std::uint32_t*>(buffer + 4);
    }

    *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uint8_t*>(&m_timePart) + 4) =
            *reinterpret_cast<const std::uint16_t*>(buffer + 8);
    *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uint8_t*>(&m_timePart) + 6) = 0;

#else

    // Generic implementation

    if (SIODB_UNLIKELY(length < 4)) return 0;

    std::uint32_t datePart = 0;
    ::pbeDecodeUInt32(buffer, &datePart);
#if __BYTE_ORDER == __BIG_ENDIAN
    datePart = bswap32(datePart);
#endif  // __BYTE_ORDER
    ::pbeEncodeUInt32(datePart, reinterpret_cast<std::uint8_t*>(&m_datePart));
    if (!m_datePart.m_hasTimePart) {
        ::pbeEncodeZero64(reinterpret_cast<std::uint8_t*>(&m_timePart));
        return 4;
    }

    if (SIODB_UNLIKELY(length < 10)) return 0;

    std::uint8_t tempBuffer[8];
    tempBuffer[0] = buffer[4];
    tempBuffer[1] = buffer[5];
    tempBuffer[2] = buffer[6];
    tempBuffer[3] = buffer[7];
    tempBuffer[4] = buffer[8];
    tempBuffer[5] = buffer[9];
    // Make GCC happy: initialize entire buffer
    tempBuffer[6] = tempBuffer[7] = 0;
    std::uint64_t timePart = 0;
    ::pbeDecodeUInt64(tempBuffer, &timePart);
#if __BYTE_ORDER == __BIG_ENDIAN
    timePart = bswap64(timePart);
#endif  // __BYTE_ORDER
    ::pbeEncodeUInt64(timePart, reinterpret_cast<std::uint8_t*>(&m_timePart));

#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS

    return 10;
}

void RawDateTime::deserializeDatePart(const std::uint8_t* buffer) noexcept
{
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    // x86 implementation
    m_datePart = *reinterpret_cast<const RawDate*>(buffer);
#else
    // Generic implementation
    {
        std::uint32_t datePart = 0;
        ::pbeDecodeUInt32(buffer, &datePart);
#if __BYTE_ORDER == __BIG_ENDIAN
        datePart = bswap32(datePart);
#endif  // __BYTE_ORDER
        ::pbeEncodeUInt32(datePart, reinterpret_cast<std::uint8_t*>(&m_datePart));
    }
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS

    // Check if time part exists
    if (m_datePart.m_hasTimePart) return;
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
    // x86 implementation
    // Make GCC happy: respect strict alising rules
    const auto p = reinterpret_cast<std::uint64_t*>(&m_timePart);
    *p = 0;
#else
    // Generic implementation
    ::pbeEncodeZero64(reinterpret_cast<std::uint8_t*>(&m_timePart));
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
}

void RawDateTime::parse(const char* s, std::size_t len, const char* format)
{
    // Check for empty string
    if (SIODB_UNLIKELY(len == 0)) throw std::invalid_argument("datetime string is empty");
    if (SIODB_UNLIKELY(s == nullptr)) throw std::invalid_argument("datetime string is nullptr");

    // Parse string
    boost::iostreams::array_source dataSource(s, len);
    boost::iostreams::stream<boost::iostreams::array_source> str(dataSource);
    date::sys_time<stdext::chrono::high_capacity_system_clock::duration> timePoint;
    str >> date::parse(format, timePoint);
    if (str.fail()) throw std::invalid_argument("invalid datetime string");

    // Validate year
    const date::year_month_day ymd(date::floor<date::days>(timePoint));
    const int year = static_cast<int>(ymd.year());
    if (year < RawDate::kMinYear || year > RawDate::kMaxYear)
        throw std::out_of_range("datetime year is out of range");

    // Fill date part
    m_datePart.m_year = year;
    m_datePart.m_month = static_cast<unsigned int>(ymd.month()) - 1;
    m_datePart.m_dayOfMonth = static_cast<unsigned int>(ymd.day()) - 1;
    m_datePart.m_dayOfWeek = date::weekday(ymd).c_encoding();

    // Extract time
    const std::int64_t t0 = timePoint.time_since_epoch().count() % kNanosecondsPerDay;
    std::uint64_t t = (t0 < 0) ? t0 + kNanosecondsPerDay : t0;

    // Fill time part
    m_timePart.m_nanos = t % std::nano::den;
    t /= std::nano::den;
    m_timePart.m_seconds = t % 60;
    t /= 60;
    m_timePart.m_minutes = t % 60;
    t /= 60;
    m_timePart.m_hours = t % 24;

    m_timePart.m_reserved1 = 0;
    m_timePart.m_reserved2 = 0;

    m_datePart.m_hasTimePart = m_timePart != kZeroRawTime;
}

std::string RawDateTime::formatDefault() const
{
    char buffer[kMaxDateTimeStringLength];
    int size;
    if (m_datePart.m_hasTimePart) {
        // String date month and day starts from 1. Internal format starts from 0
        size = std::snprintf(buffer, kMaxDateTimeStringLength, kDefaultDateTimePrintString,
                m_datePart.m_year, m_datePart.m_month + 1, m_datePart.m_dayOfMonth + 1,
                m_timePart.m_hours, m_timePart.m_minutes, m_timePart.m_seconds, m_timePart.m_nanos);
    } else {
        size = std::snprintf(buffer, kMaxDateStringLength, kDefaultDatePrintString,
                m_datePart.m_year, m_datePart.m_month + 1, m_datePart.m_dayOfMonth + 1);
    }
    return std::string(buffer, size);
}

std::string RawDateTime::format(const char* format) const
{
    constexpr stdext::int128_t kNanosecondsPerDay128 = kNanosecondsPerDay;
    const auto y = static_cast<int>(m_datePart.m_year) - (m_datePart.m_month <= 1);
    const auto era = (y >= 0 ? y : y - 399) / 400;
    const auto yoe = static_cast<unsigned>(y - era * 400);  // [0, 399]
    const auto doy =
            (153 * (m_datePart.m_month > 1 ? m_datePart.m_month - 2 : m_datePart.m_month + 10) + 2)
                    / 5
            + m_datePart.m_dayOfMonth;  // [0, 365]
    const auto doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;  // [0, 146096]
    const auto days = era * 146097 + static_cast<int>(doe) - 719468;
    const date::sys_time<stdext::chrono::high_capacity_system_clock::duration> timePoint(
            stdext::chrono::high_capacity_system_clock::duration(
                    days * kNanosecondsPerDay128
                    + (m_timePart.m_hours * 3600 + m_timePart.m_minutes * 60 + m_timePart.m_seconds)
                              * std::nano::den
                    + m_timePart.m_nanos));

    return date::format(format, timePoint);
}

bool RawDateTime::operator==(const RawDateTime& other) const noexcept
{
    return m_datePart == other.m_datePart
           && (!m_datePart.m_hasTimePart || m_timePart == other.m_timePart);
}

bool RawDateTime::operator!=(const RawDateTime& other) const noexcept
{
    return m_datePart != other.m_datePart
           || (m_datePart.m_hasTimePart && m_timePart != other.m_timePart);
}

bool RawDateTime::operator>(const RawDateTime& other) const noexcept
{
    if (m_datePart > other.m_datePart)
        return true;
    else if (m_datePart < other.m_datePart)
        return false;

    if (m_datePart.m_hasTimePart && other.m_datePart.m_hasTimePart)
        return m_timePart > other.m_timePart;
    else
        return m_datePart.m_hasTimePart;
}

bool RawDateTime::operator>=(const RawDateTime& other) const noexcept
{
    if (m_datePart > other.m_datePart)
        return true;
    else if (m_datePart < other.m_datePart)
        return false;

    if (m_datePart.m_hasTimePart && other.m_datePart.m_hasTimePart)
        return m_timePart >= other.m_timePart;
    else
        return !other.m_datePart.m_hasTimePart;
}

std::time_t RawDateTime::toEpochTimestamp() const noexcept
{
    int hh = 0, mm = 0, ss = 0;
    if (m_datePart.m_hasTimePart) {
        hh = static_cast<int>(m_timePart.m_hours);
        mm = static_cast<int>(m_timePart.m_minutes);
        ss = static_cast<int>(m_timePart.m_seconds);
    }
    std::tm tm {ss, mm, hh, static_cast<int>(m_datePart.m_dayOfMonth + 1),
            static_cast<int>(m_datePart.m_month), static_cast<int>(m_datePart.m_year) - 1900, 0, 0,
            0, 0, 0};
    return std::mktime(&tm);
}

///////////////////// Utility functions ///////////////////////////////////////

const char* getDayOfWeekShortName(unsigned dayOfWeek)
{
    if (SIODB_LIKELY(dayOfWeek < kDayOfWeekShortNames.size()))
        return kDayOfWeekShortNames[dayOfWeek];
    return nullptr;
}

const char* getDayMonthShortName(unsigned month)
{
    if (SIODB_LIKELY(month < kMonthShortName.size())) return kMonthShortName[month];
    return nullptr;
}

bool convertHours24To12(unsigned hours, std::pair<unsigned, bool>& result)
{
    if (SIODB_UNLIKELY(hours > 23)) return false;
    result.second = hours > 11;
    if (result.second) hours -= 12;
    result.first = (hours == 0) ? 12 : hours;
    return true;
}

}  // namespace siodb
