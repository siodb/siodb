// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../options/LogOptions.h"
#include "../utils/Format.h"
#include "../utils/HelperMacros.h"

// Boost headers
#include <boost/log/trivial.hpp>

namespace siodb::log {

/**
 * Initializes logging subsystem.
 * @param logOption Log options.
 */
void initLogging(const config::LogOptions& logOptions);

/** Shuts down logging subsystem */
void shutdownLogging();

/** Logging subsystem initialization and shutdown guard */
struct LogSubsystemGuard {
    LogSubsystemGuard(const config::LogOptions& logOptions)
    {
        initLogging(logOptions);
    }

    ~LogSubsystemGuard()
    {
        shutdownLogging();
    }
};

}  // namespace siodb::log

// Logging macros
#define LOG_TRACE BOOST_LOG_TRIVIAL(trace)
#define LOG_DEBUG BOOST_LOG_TRIVIAL(debug)
#define LOG_INFO BOOST_LOG_TRIVIAL(info)
#define LOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define LOG_ERROR BOOST_LOG_TRIVIAL(error)
#define LOG_FATAL BOOST_LOG_TRIVIAL(fatal)

#define DBG_LOG_NO_OUTPUT(x) \
    do {                     \
    } while (false)

// Debug build only logging macros
#ifdef _DEBUG
#define DBG_LOG_TRACE(x) LOG_TRACE << x
#define DBG_LOG_DEBUG(x) LOG_DEBUG << x
#define DBG_LOG_INFO(x) LOG_INFO << x
#define DBG_LOG_WARNING(x) LOG_WARNING << x
#define DBG_LOG_ERROR(x) LOG_ERROR << x
#define DBG_LOG_FATAL(x) LOG_FATAL << x
#else
#define DBG_LOG_TRACE(x) DBG_LOG_NO_OUTPUT(x)
#define DBG_LOG_DEBUG(x) DBG_LOG_NO_OUTPUT(x)
#define DBG_LOG_INFO(x) DBG_LOG_NO_OUTPUT(x)
#define DBG_LOG_WARNING(x) DBG_LOG_NO_OUTPUT(x)
#define DBG_LOG_ERROR(x) DBG_LOG_NO_OUTPUT(x)
#define DBG_LOG_FATAL(x) DBG_LOG_NO_OUTPUT(x)
#endif
