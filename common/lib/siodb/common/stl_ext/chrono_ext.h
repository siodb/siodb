// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "cstdint_ext.h"

// STL headers
#include <chrono>

namespace stdext::chrono {

/** Day duration. */
typedef std::chrono::duration<std::int64_t, std::ratio<86400>> days;

/** Week duration. */
typedef std::chrono::duration<std::int64_t, std::ratio<604800>> weeks;

/** High-capacity nanosecond duration. */
typedef std::chrono::duration<stdext::int128_t, std::nano> high_capacity_nanoseconds;

/** High-capacity microsecond duration. */
typedef std::chrono::duration<stdext::int128_t, std::micro> high_capacity_microseconds;

/** High-capacity millisecond duration. */
typedef std::chrono::duration<stdext::int128_t, std::milli> high_capacity_milliseconds;

/** High-capacity second duration. */
typedef std::chrono::duration<stdext::int128_t> high_capacity_seconds;

/** High-capacity minute duration. */
typedef std::chrono::duration<stdext::int128_t, std::ratio<60>> high_capacity_minutes;

/** High-capacity hour duration. */
typedef std::chrono::duration<stdext::int128_t, std::ratio<3600>> high_capacity_hours;

/** High-capacity day duration. */
typedef std::chrono::duration<stdext::int128_t, std::ratio<86400>> high_capacity_days;

/** High-capacity week duration. */
typedef std::chrono::duration<stdext::int128_t, std::ratio<604800>> high_capacity_weeks;

/**
 * High-capacity system clock definition.
 * Can represent large scope of time down to nanoseconds.
 * Uses 128-bit duration representation.
 */
struct high_capacity_system_clock {
    typedef high_capacity_nanoseconds duration;
    typedef duration::rep rep;
    typedef duration::period period;
    typedef std::chrono::time_point<high_capacity_system_clock, duration> time_point;
    static time_point now() noexcept;
};

}  // namespace stdext::chrono
