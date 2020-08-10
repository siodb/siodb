// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Log.h"

// Project headers
#include "../options/InvalidConfigurationError.h"
#include "../stl_wrap/filesystem_wrapper.h"
#include "../sys/Syscalls.h"
#include "../utils/Debug.h"

// STL headers
#include <iostream>

// System headers
#include <unistd.h>

// Boost headers
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/phoenix.hpp>

BOOST_LOG_ATTRIBUTE_KEYWORD(
        process_id, "ProcessID", boost::log::attributes::current_process_id::value_type)

namespace siodb::log {

namespace {

SyscallsLibraryGuard syscallsGuard;

class FilterBySeverity {
public:
    FilterBySeverity(boost::log::trivial::severity_level logLevel)
        : m_logLevel(logLevel)
    {
    }

    bool operator()(const boost::log::attribute_value_set& attrs) const noexcept
    {
        const auto severity = attrs[boost::log::trivial::severity];
        return !severity || severity >= m_logLevel;
    }

private:
    const boost::log::trivial::severity_level m_logLevel;
    const boost::log::attribute_name m_severityAttributeName;
};

boost::log::attributes::current_process_id::value_type::native_type getNativeProcessId(
        const boost::log::value_ref<boost::log::attributes::current_process_id::value_type,
                tag::process_id>& pid)
{
    return pid ? pid->native_id() : 0;
}

auto createLogFormatter1()
{
    boost::log::expressions::stream_type stream;
    return stream << boost::log::expressions::format_date_time<boost::posix_time::ptime>(
                   "TimeStamp", "%Y-%m-%d %H:%M:%S.%f ")
                  << boost::log::trivial::severity << ' '
                  << boost::phoenix::bind(&getNativeProcessId, process_id.or_none()) << ' '
                  << boost::log::expressions::attr<pid_t>("KernelThreadID") << ' '
                  << boost::log::expressions::smessage;
}

auto createLogFormatter2()
{
    boost::log::expressions::stream_type stream;
    return stream << boost::log::expressions::format_date_time<boost::posix_time::ptime>(
                   "TimeStamp", "%Y-%m-%d %H:%M:%S.%f ")
                  << boost::log::trivial::severity << ' '
                  << boost::phoenix::bind(&getNativeProcessId, process_id.or_none()) << ' '
                  << boost::log::expressions::attr<pid_t>("KernelThreadID") << ' '
                  << boost::log::expressions::smessage;
}

}  // namespace

void initLogging(const config::LogOptions& options)
{
    if (options.m_logChannels.empty()) {
        throw std::runtime_error("No log channels defined");
    }

    boost::log::add_common_attributes();
    boost::log::core::get()->add_global_attribute(
            "KernelThreadID", boost::log::attributes::make_function(&::gettid));

    // Ensure log folders
    for (const auto& channelOptions : options.m_logChannels) {
        if (channelOptions.m_type != config::LogChannelType::kFile) continue;
        const fs::path logDir(channelOptions.m_destination);
        if (!fs::exists(logDir)) {
            fs::create_directories(logDir);
        } else if (!fs::is_directory(logDir)) {
            throw std::runtime_error("Log directory path is not a directory");
        }
    }

    // Set up log channels
    for (const auto& channelOptions : options.m_logChannels) {
        switch (channelOptions.m_type) {
            case config::LogChannelType::kConsole: {
                if (channelOptions.m_destination == "stdout") {
                    auto sink = boost::log::add_console_log(
                            std::cout, boost::log::keywords::format = createLogFormatter2());
                    sink->set_filter(FilterBySeverity(channelOptions.m_severity));
                    sink->locked_backend()->auto_flush(true);
                } else if (channelOptions.m_destination == "stderr") {
                    auto sink = boost::log::add_console_log(
                            std::cerr, boost::log::keywords::format = createLogFormatter2());
                    sink->set_filter(FilterBySeverity(channelOptions.m_severity));
                    sink->locked_backend()->auto_flush(true);
                } else {
                    std::ostringstream err;
                    err << "Invalid channel destination '" << channelOptions.m_destination
                        << "' for the log channel " << channelOptions.m_name;
                    throw config::InvalidConfigurationError(err.str());
                }
                DEBUG_TRACE("Log channel '" << channelOptions.m_name << "' initialized.");
                break;
            }

            case config::LogChannelType::kFile: {
                std::string logFileName;
                {
                    std::ostringstream str;
                    str << channelOptions.m_destination << '/' << options.m_logFileBaseName
                        << "_%Y%m%d_%H%M%S_" << ::getpid() << ".log";
                    logFileName = str.str();
                }
                auto sink = boost::log::add_file_log(boost::log::keywords::file_name = logFileName,
                        boost::log::keywords::rotation_size = channelOptions.m_maxLogFileSize,
                        boost::log::keywords::max_files = channelOptions.m_maxFiles,
                        boost::log::keywords::time_based_rotation =
                                boost::log::sinks::file::rotation_at_time_interval(
                                        boost::posix_time::seconds(
                                                channelOptions.m_logFileExpirationTimeout)),
                        boost::log::keywords::format = createLogFormatter1());
                sink->set_filter(FilterBySeverity(channelOptions.m_severity));
                sink->locked_backend()->auto_flush();
                DEBUG_TRACE("Log channel '" << channelOptions.m_name << "' initialized.");
                break;
            }
        }
    }

    LOG_INFO << "Logging started.";
}

void shutdownLogging()
{
    LOG_INFO << "Logging stopped.";
    boost::log::core::get()->remove_all_sinks();
}

}  // namespace siodb::log
