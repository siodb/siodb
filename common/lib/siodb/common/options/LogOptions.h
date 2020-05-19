// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../utils/Constants.h"

// STL headers
#include <limits>
#include <string>
#include <vector>

// System headers
#include <sys/types.h>

// Boost headers
#include <boost/log/trivial.hpp>

namespace siodb::config {

namespace defaults {

constexpr off_t kMaxMaxLogFileSize = std::numeric_limits<off_t>::max() / 2;
constexpr off_t kDefaultMaxLogFileSize = kMaxMaxLogFileSize;

constexpr std::size_t kDefaultMaxLogFilesCount = std::numeric_limits<std::size_t>::max();

constexpr std::time_t kMaxLogFileExpirationTimeout = kTypicalWeeksPerYear * kSecondsInWeek;
constexpr std::time_t kDefaultLogFileExpirationTimeout = kMaxLogFileExpirationTimeout;

constexpr boost::log::trivial::severity_level kDefaultLogSeverityLevel = boost::log::trivial::info;

}  // namespace defaults

/** Supported log channel types */
enum class LogChannelType { kConsole, kFile };

/** Log channel options */
struct LogChannelOptions {
    /** Channel name */
    std::string m_name;

    /** Channel type */
    LogChannelType m_type = LogChannelType::kConsole;

    /**
     * Log file path base.
     * I.e. /var/log/siodb/siodb1/log/siodb_
     * which will be appended log file timestamp.
     */
    std::string m_destination;

    /**
     * Maximum log file size in bytes.
     * Minimal valid value is 1024.
     * 0 means no size limit.
     */
    off_t m_maxLogFileSize = defaults::kDefaultMaxLogFileSize;

    /** Maximum number of files to keep */
    std::size_t m_maxFiles = defaults::kDefaultMaxLogFilesCount;

    /**
     * Number of seconds to use current log file since it was created.
     * After this period new log will will be created.
     * Minimal value is 60 (each minute).
     * 0 means do not check expiration.
     */
    std::time_t m_logFileExpirationTimeout = defaults::kDefaultLogFileExpirationTimeout;

    /** Minimum log message severity */
    boost::log::trivial::severity_level m_severity = defaults::kDefaultLogSeverityLevel;
};

/** Log options */
struct LogOptions {
    /** Logging channels */
    std::vector<LogChannelOptions> m_logChannels;
    /** Log file base name */
    std::string m_logFileBaseName;
};

}  // namespace siodb::config
