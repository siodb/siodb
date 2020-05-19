// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// CRT headers
#include <cstddef>
#include <ctime>

namespace siodb {

// Conversion of different size magnitudes to bytes
constexpr std::size_t kBytesInKB = 1024;
constexpr std::size_t kBytesInMB = kBytesInKB * 1024;
constexpr std::size_t kBytesInGB = kBytesInMB * 1024;

// Conversion of different time intervals to seconds
constexpr std::time_t kSecondsInMinute = 60;
constexpr std::time_t kMinutesInHour = 60;
constexpr std::time_t kHoursInDay = 24;
constexpr std::time_t kDaysInWeek = 7;
constexpr std::time_t kTypicalWeeksPerYear = 52;
constexpr std::time_t kSecondsInHour = kSecondsInMinute * kMinutesInHour;
constexpr std::time_t kSecondsInDay = kSecondsInHour * kHoursInDay;
constexpr std::time_t kSecondsInWeek = kSecondsInDay * kDaysInWeek;
constexpr std::time_t kMinutesInDay = kMinutesInHour * kHoursInDay;
constexpr std::time_t kMinutesInWeek = kMinutesInDay * kDaysInWeek;
constexpr std::time_t kHoursInWeek = kHoursInDay * kDaysInWeek;

}  // namespace siodb
