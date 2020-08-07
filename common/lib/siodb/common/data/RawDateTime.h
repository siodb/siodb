// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>
#include <cstring>
#include <ctime>

// STL headers
#include <string>
#include <utility>

namespace siodb {

#pragma pack(push, 1)

/** Date value. */
struct RawDate {
    /** Used to indicate that time part present in serialize form . Otherwise must be zero. */
    bool m_hasTimePart : 1;

    /** Day of week: 0...6 Sun-Sat */
    unsigned m_dayOfWeek : 3;

    /** Day of month: 0...30 -> 1 ... 31 */
    unsigned m_dayOfMonth : 5;

    /** Month: 0...11 -> Jan ... Dec */
    unsigned m_month : 4;

    /** Year: -262144 ... +262143 */
    int m_year : 19;

    /** Minimum year */
    static constexpr int kMinYear = -262144;

    /** Maximum year */
    static constexpr int kMaxYear = 262143;

    /** Minimum month */
    static constexpr int kMinMonth = 0;

    /** Maximum month */
    static constexpr int kMaxMonth = 11;

    /** Minimum day */
    static constexpr int kMinDay = 0;

    /** Maximum day */
    static constexpr int kMaxDay = 30;

    /** Minimum day of week */
    static constexpr int kMinDayOfWeek = 0;

    /** Maximum day of week */
    static constexpr int kMaxDayOfWeek = 6;

    /** Initializes structure RawDate. */
    RawDate() noexcept = default;

    /**
     * Initializes structure RawDate from an epoch timestamp.
     * @param t Epoch timestamp.
     */
    explicit RawDate(std::time_t t) noexcept;

    /**
     * Initializes structure RawDate.
     * @param year Year: -262144 ... +262143.
     * @param month Month: 0...11 -> Jan ... Dec.
     * @param dayOfMonth Day of month: 0...30 -> 1 ... 31.
     * @param dayOfWeek Day of week: 0...6 Sun-Sat.
     * @param hasTimePart Time part presence flag.
     */
    constexpr RawDate(int year, unsigned month, unsigned dayOfMonth, unsigned dayOfWeek,
            bool hasTimePart = false) noexcept
        : m_hasTimePart(hasTimePart)
        , m_dayOfWeek(dayOfWeek)
        , m_dayOfMonth(dayOfMonth)
        , m_month(month)
        , m_year(year)
    {
    }

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

    /**
     * Converts this time into UNIX epoch timestamp.
     * @return Corresponding UNIX epoch timestamp.
     */
    std::time_t toEpochTimestamp() const noexcept;
};

#pragma pack(pop)

/** Zero date constant */
constexpr const RawDate kZeroRawDate = RawDate(0, 0, 0, 6, false);

#pragma pack(push, 1)

/** Time value. */
struct RawTime {
    /** Reserved, must be 0, likely will be TZ presence flag. */
    bool m_reserved1 : 1;

    /** Number of nanoseconds */
    unsigned m_nanos : 30;

    /** Number of seconds */
    unsigned m_seconds : 6;

    /** Number of minutes */
    unsigned m_minutes : 6;

    /** Number of hours */
    unsigned m_hours : 5;

    /** Fill up to 64 bits */
    int m_reserved2 : 16;

    /** Minimum hours value */
    static constexpr int kMinHours = 0;

    /** Maximum hours value */
    static constexpr int kMaxHours = 23;

    /** Minimum minutes value */
    static constexpr int kMinMinutes = 0;

    /** Maximum minutes value */
    static constexpr int kMaxMinutes = 59;

    /** Minimum seconds value */
    static constexpr int kMinSeconds = 0;

    /** Maximum seconds value */
    static constexpr int kMaxSeconds = 59;

    /** Minimum nanoseconds value */
    static constexpr int kMinNanoseconds = 0;

    /** Maximum nanoseconds value */
    static constexpr int kMaxNanoseconds = 999999999;

    /** Initializes structure RawTime. */
    RawTime() noexcept = default;

    /**
     * Initializes structure RawTime from an epoch timestamp.
     * @param t Epoch timestamp.
     */
    explicit RawTime(std::time_t t) noexcept;

    /**
     * Initializes structure RawTime.
     * @param hours Hours: 0...23.
     * @param minutes Minutes: 0...59.
     * @param seconds Seconds: 0...59.
     * @param nanos Nanoseconds: 0...99999999.
     */
    constexpr RawTime(unsigned hours, unsigned minutes, unsigned seconds, unsigned nanos) noexcept
        : m_reserved1(0)
        , m_nanos(nanos)
        , m_seconds(seconds)
        , m_minutes(minutes)
        , m_hours(hours)
        , m_reserved2(0)
    {
    }

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

    /**
     * Converts this date into UNIX epoch timestamp.
     * @return Corresponding UNIX epoch timestamp.
     */
    std::time_t toEpochTimestamp() const noexcept;
};

#pragma pack(pop)

/** Zero time constant */
constexpr const RawTime kZeroRawTime = RawTime(0, 0, 0, 0);

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

    /** Default Datetime print string format */
    static constexpr const char* kDefaultDateTimePrintString = "%d-%02d-%02d %02d:%02d:%02d.%d";

    /** Default Date scan string format */
    static constexpr const char* kDefaultDateScanString = "%6d-%02d-%02d";

    /** Default Date print string format */
    static constexpr const char* kDefaultDatePrintString = "%d-%02d-%02d";

    /** Default date/time format */
    static constexpr const char* kDefaultDateTimeFormat = "%Y-%m-%d %H:%M:%S";

    /** Default date format */
    static constexpr const char* kDefaultDateFormat = "%Y-%m-%d";

    /** Default time format */
    static constexpr const char* kDefaultTimeFormat = "%H:%M:%S";

    /* Initializes structure RawDateTime. */
    RawDateTime() noexcept = default;

    /**
     * Initializes structure RawDateTime from a string.
     * @param s A string.
     * @param format Datetime string format.
     * @throw out_of_range if date range is invalid
     * @throw invalid_argument if date string can't be parsed
     */
    RawDateTime(const std::string& s, const char* format = kDefaultDateTimeFormat)
    {
        parse(s, format);
    }

    /**
     * Initializes structure RawDateTime from a string using default format.
     * @param s A string.
     * @param format Datetime string format.
     * @throw out_of_range if date range is invalid
     * @throw invalid_argument if date string can't be parsed
     */
    RawDateTime(const char* s, const char* format = kDefaultDateTimeFormat)
    {
        parse(s, std::strlen(s), format);
    }

    /**
     * Initializes structure RawDateTime from a string.
     * @param s A string.
     * @param len String length.
     * @param format Datetime string format.
     * @throw out_of_range if date range is invalid
     * @throw invalid_argument if date string can't be parsed
     */
    RawDateTime(const char* s, std::size_t len, const char* format = kDefaultDateTimeFormat)
    {
        parse(s, len, format);
    }

    /**
     * Initializes structure RawDateTime from an epoch timestamp.
     * @param t Epoch timestamp.
     */
    RawDateTime(std::time_t t) noexcept;

    /**
     * Initializes structure RawDateTime from an epoch timestamp.
     * @param year Year: -262144 ... +262143.
     * @param month Month: 0...11 -> Jan ... Dec.
     * @param dayOfMonth Day of month: 0...30 -> 1 ... 31.
     * @param dayOfWeek Day of week: 0...6 Sun-Sat.
     */
    RawDateTime(int year, unsigned month, unsigned dayOfMonth, unsigned dayOfWeek) noexcept
        : m_datePart(year, month, dayOfMonth, dayOfWeek, false)
    {
    }

    /**
     * Initializes structure RawDateTime from an epoch timestamp.
     * @param year Year: -262144 ... +262143.
     * @param month Month: 0...11 -> Jan ... Dec.
     * @param dayOfMonth Day of month: 0...30 -> 1 ... 31.
     * @param dayOfWeek Day of week: 0...6 Sun-Sat.
     * @param hours Hours: 0...23.
     * @param minutes Minutes: 0...59.
     * @param seconds Seconds: 0...59.
     * @param nanos Nanoseconds: 0...99999999.
     */
    RawDateTime(int year, unsigned month, unsigned dayOfMonth, unsigned dayOfWeek, unsigned hours,
            unsigned minutes, unsigned seconds, unsigned nanos) noexcept
        : m_timePart(hours, minutes, seconds, nanos)
        , m_datePart(year, month, dayOfMonth, dayOfWeek, true)
    {
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

    /**
     * Converts this date/time into UNIX epoch timestamp.
     * @return Corresponding UNIX epoch timestamp.
     */
    std::time_t toEpochTimestamp() const noexcept;

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
