// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

// STL headers
#include <string>
#include <utility>

namespace siodb {

#pragma pack(push, 1)
/** Date value. */
struct RawDate {
    /** Used to indicate that time part present in serialize form . Otherwise must be zero. */
    unsigned m_hasTimePart : 1;

    /** Day of week: 0...6 Sun-Sat */
    unsigned m_dayOfWeek : 3;

    /** Day of month: 0...30 -> 1 ... 31 */
    unsigned m_dayOfMonth : 5;

    /** Month: 0...11 -> Jan ... Dec */
    unsigned m_month : 4;

    /** Year -262144 ... +262143 */
    int m_year : 19;

    static constexpr int kMinYear = -262144;
    static constexpr int kMaxYear = 262143;
    static constexpr int kMinMonth = 0;
    static constexpr int kMaxMonth = 11;
    static constexpr int kMinDay = 0;
    static constexpr int kMaxDay = 30;
    static constexpr int kMinDayOfWeek = 0;
    static constexpr int kMaxDayOfWeek = 6;

    /**
     * Compares date with other RawDate
     * @param other Other RawDate
     * @return true if objects are equal, false otherwise.
     */
    bool operator==(const RawDate& other) const noexcept;

    /**
     * Compares date with other RawDate
     * @param other Other RawDate
     * @return true if objects aren't equal, false otherwise.
     */
    bool operator!=(const RawDate& other) const noexcept;

    /**
     * Checks if this date greater than another one
     * @param other Other RawDate
     * @return true if this date is greater than another one, false otherwise
     */
    bool operator>(const RawDate& other) const noexcept;

    /**
     * Checks if this date greater than or equal to another one
     * @param other Other RawDate
     * @return true if this date is greater than or equal to another one, false otherwise
     */
    bool operator>=(const RawDate& other) const noexcept;

    /**
     * Checks if this date less than another one
     * @param other Other RawDate
     * @return true if this date is less than another one, false otherwise
     */
    bool operator<(const RawDate& other) const noexcept
    {
        return !(*this >= other);
    }

    /**
     * Checks if this date less than or equal to another one
     * @param other Other RawDate
     * @return true if this date is less than or equal to another one, false otherwise
     */
    bool operator<=(const RawDate& other) const noexcept
    {
        return !(*this > other);
    }
};

/** Time value. */
struct RawTime {
    /** Reserved, must be 0, likely will be TZ presence flag. */
    unsigned m_reserved1 : 1;

    /** Number of nanoseconds */
    unsigned m_nanos : 30;

    /** Number of seconds */
    unsigned m_seconds : 6;

    /** Number of minutes */
    unsigned m_minutes : 6;

    /** Number of hours */
    unsigned m_hours : 5;

    /** Fill up to 64 bits */
    unsigned m_reserved2 : 16;

    static constexpr int kMinHours = 0;
    static constexpr int kMaxHours = 23;
    static constexpr int kMinMinutes = 0;
    static constexpr int kMaxMinutes = 59;
    static constexpr int kMinSeconds = 0;
    static constexpr int kMaxSeconds = 59;
    static constexpr int kMinNanoseconds = 0;
    static constexpr int kMaxNanoseconds = 999999999;

    static const RawTime kZeroTime;

    /**
     * Compares this object with other RawTime
     * @param other Other RawTime
     * @return true if objects are equal, false otherwise.
     */
    bool operator==(const RawTime& other) const noexcept;

    /**
     * Compares this object with other RawTime
     * @param other Other RawTime
     * @return true if objects aren't equal, false otherwise.
     */
    bool operator!=(const RawTime& other) const noexcept;

    /**
     * Checks if this object greater than another one
     * @param other Other RawTime
     * @return true if this object is greater than another, false otherwise
     */
    bool operator>(const RawTime& other) const noexcept;

    /**
     * Checks if this object greater than or equal to another one
     * @param other Other RawTime
     * @return true if this object is greater than or equal to another, false otherwise
     */
    bool operator>=(const RawTime& other) const noexcept;

    /**
     * Checks if this object less than another one
     * @param other Other RawTime
     * @return true if this object is less than another, false otherwise
     */
    bool operator<(const RawTime& other) const noexcept
    {
        return !(*this >= other);
    }

    /**
     * Checks if this object less than or equal to another one
     * @param other Other RawTime
     * @return true if this object is less than or equal to another, false otherwise
     */
    bool operator<=(const RawTime& other) const noexcept
    {
        return !(*this > other);
    }
};

#pragma pack(pop)

constexpr const RawTime RawTime::kZeroTime {0, 0, 0, 0, 0, 0};

/** Date time value. */
struct RawDateTime {
    /** Maximum serialized size of a RawDateTime object */
    static constexpr std::size_t kMaxSerializedSize = 10;

    /** Serialized size of date part */
    static constexpr std::size_t kDatePartSerializedSize = 4;

    /**
     * Maximum string length for default datetime string
     * "-262144:12:31 23:59:59.999999999\0"
     */
    static constexpr std::size_t kMaxDateTimeStringLength = 33;

    /**
     * Maximum string length for default datetime string
     * "-262144:12:31\0"
     */
    static constexpr std::size_t kMaxDateStringLength = 14;

    /** Number of nanoseconds per day */
    static constexpr std::int64_t kNanosecondsPerDay = 86400000000000LL;

    /** Default Datetime scan string format */
    static constexpr const char* kDefaultDateTimeScanString = "%6d-%02d-%02d %02d:%02d:%02d.%d";

    /** Default Datetime scan string format */
    static constexpr const char* kDefaultDateScanString = "%6d-%02d-%02d";

    /* Initializes structure RawDateTime. */
    RawDateTime() noexcept
    {
    }

    /**
     * Initializes structure RawDateTime from a string.
     * @param s A string.
     * @param format Datetime string format.
     * @throw out_of_range if date range is invalid
     * @throw invalid_argument if date string can't be parsed
     */
    RawDateTime(const std::string& s, const char* format)
    {
        parse(s, format);
    }

    /**
     * Initializes structure RawDateTime from a string.
     * @param s A string.
     * @param len String length.
     * @param format Datetime string format.
     * @throw out_of_range if date range is invalid
     * @throw invalid_argument if date string can't be parsed
     */
    RawDateTime(const char* s, std::size_t len, const char* format)
    {
        parse(s, len, format);
    }

    /**
     * Serializes object into a buffer.
     * @param buffer A buffer.
     * @return Address of byte after last written byte.
     */
    std::uint8_t* serialize(std::uint8_t* buffer) const noexcept;

    /**
     * De-serializes object from a buffer.
     * @param buffer A buffer.
     * @param length Data length in the buffer.
     * @return Positive number of consumed bytes on success, zero if there were not enough data,
     *         -1 if deserialization error occurred.
     */
    int deserialize(const std::uint8_t* buffer, std::size_t length) noexcept;

    /**
     * Returns serialized buffer size for raw date time.
     * @return Serialized buffer size for raw date time.
     */
    std::size_t getSerializedSize() const noexcept
    {
        return m_datePart.m_hasTimePart ? 10 : 4;
    }

    /**
     * De-serializes date part from a buffer. If time part doesn't exist,
     * sets it to 0.
     * @param buffer A buffer.
     * @return Address of byte after last written byte.
     */
    void deserializeDatePart(const std::uint8_t* buffer) noexcept;

    /**
     * Parses date and time from string.
     * @param s A string.
     * @param len String length.
     * @param format Datetime string format.
     * @throw out_of_range If date range is invalid.
     * @throw invalid_argument if date string can't be parsed.
     */
    void parse(const char* s, std::size_t len, const char* format);

    /**
     * Parses date and time from string.
     * @param s A string.
     * @param format Datetime string format.
     * @throw out_of_range If date range is invalid.
     * @throw invalid_argument if date string can't be parsed.
     */
    void parse(const std::string& s, const char* format)
    {
        parse(s.c_str(), s.length(), format);
    }

    /**
     * Formats date/time as string.
     * @return Datetime formatted as string.
     */
    std::string formatDefault() const;

    /**
     * Formats date/time as string.
     * @param fmt Datetime format.
     * @return Datetime formatted as string.
     */
    std::string format(const char* fmt) const;

    /**
     * Compares datetime with other RawDateTime
     * @param other Other RawDateTime
     * @return true if objects are equal, false otherwise.
     */
    bool operator==(const RawDateTime& other) const noexcept;

    /**
     * Compares datetime with other RawDateTime
     * @param other Other RawDateTime
     * @return true if objects aren't equal, false otherwise.
     */
    bool operator!=(const RawDateTime& other) const noexcept;

    /**
     * Checks if this object greater than another one
     * @param other Other RawDateTime
     * @return true if this object is greater than another, false otherwise
     */
    bool operator>(const RawDateTime& other) const noexcept;

    /**
     * Checks if this object greater than or equal to another one
     * @param other Other RawDateTime
     * @return true if this object is greater than or equal to another, false otherwise
     */
    bool operator>=(const RawDateTime& other) const noexcept;

    /**
     * Checks if this object less than another one
     * @param other Other RawDateTime
     * @return true if this object is less than another, false otherwise
     */
    bool operator<(const RawDateTime& other) const noexcept
    {
        return !(*this >= other);
    }

    /**
     * Checks if this object less than or equal to another one
     * @param other Other RawDateTime
     * @return true if this object is less than or equal to another, false otherwise
     */
    bool operator<=(const RawDateTime& other) const noexcept
    {
        return !(*this > other);
    }

    /** Time part */
    RawTime m_timePart;

    /** Date part */
    RawDate m_datePart;
};

/**
 * Returns short name of day of week.
 * @param dayOfWeek Number of day of week, 0..6 Sunday...Saturday
 * @return Short 3-letter name of day of week or nullptr if dayOfWeek is out of range.
 */
const char* getDayOfWeekShortName(unsigned dayOfWeek);

/**
 * Returns short name of month.
 * @param month Number of day of month, 0..11 Jan...Dec.
 * @return Short 3-letter name of day of week or nullptr if month is out of range.
 */
const char* getDayMonthShortName(unsigned month);

/**
 * Converts hours from 24 to 12 hour format.
 * @param hours 24-hours value 0...23
 * @param result conversion result, first contain hours value,
 *               second contain false for AM and true for PM
 * @return true if converted successfully, false if hours is out of range.
 */
bool convertHours24To12(unsigned hours, std::pair<unsigned, bool>& result);

}  // namespace siodb
