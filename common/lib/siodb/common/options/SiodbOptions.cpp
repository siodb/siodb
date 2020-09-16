// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SiodbOptions.h"

// Project headers
#include "SiodbInstance.h"
#include "../net/NetConstants.h"
#include "../stl_wrap/filesystem_wrapper.h"

// CRT headers
#include <cstring>

// STL headers
#include <array>
#include <unordered_set>

// Boost headers
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace siodb::config {

namespace {
const std::array<const char*, static_cast<std::size_t>(boost::log::trivial::fatal) + 1>
        logLevelNames {"trace", "debug", "info", "warning", "error", "fatal"};

inline auto constructOptionPath(const std::string& str)
{
    return boost::property_tree::ptree::path_type(str, ';');
}

/** Boolean option value translator for the boost::ptree */
struct BoolTranslator {
    typedef std::string internal_type;
    typedef int external_type;
    boost::optional<bool> get_value(const std::string& option) noexcept;
};

boost::optional<bool> BoolTranslator::get_value(const std::string& option) noexcept
{
    const auto opt = option.c_str();
    if (::strcasecmp(opt, "true") == 0 || ::strcasecmp(opt, "yes") == 0
            || ::strcasecmp(opt, "1") == 0 || ::strcasecmp(opt, "t") == 0)
        return true;
    else if (::strcasecmp(opt, "false") == 0 || ::strcasecmp(opt, "no") == 0
             || ::strcasecmp(opt, "0") == 0 || ::strcasecmp(opt, "f") == 0)
        return false;
    else
        return boost::none;
}

}  // namespace

std::string SiodbOptions::getExecutableDir() const
{
    const fs::path path(m_generalOptions.m_executablePath);
    return path.parent_path().native();
}

void SiodbOptions::load(const std::string& instanceName)
{
    load(instanceName, composeInstanceConfigFilePath(instanceName));
}

void SiodbOptions::load(const std::string& instanceName, const std::string& configPath)
{
    boost::property_tree::ptree config;
    boost::property_tree::read_ini(configPath, config);

    SiodbOptions tmpOptions;

    // Instance options

    tmpOptions.m_generalOptions.m_name = instanceName;

    // Parse IPv4 port number
    {
        tmpOptions.m_generalOptions.m_ipv4Port = config.get<int>(
                constructOptionPath(kGeneralOptionIpv4Port), kDefaultIpv4PortNumber);
        if (tmpOptions.m_generalOptions.m_ipv4Port != 0
                && (tmpOptions.m_generalOptions.m_ipv4Port < net::kMinPortNumber
                        || tmpOptions.m_generalOptions.m_ipv4Port > net::kMaxPortNumber))
            throw InvalidConfigurationError("Invalid IPv4 server port number");
    }

    // Parse IPv6 port number
    {
        tmpOptions.m_generalOptions.m_ipv6Port = config.get<int>(
                constructOptionPath(kGeneralOptionIpv6Port), kDefaultIpv6PortNumber);
        if (tmpOptions.m_generalOptions.m_ipv6Port != 0
                && (tmpOptions.m_generalOptions.m_ipv6Port < net::kMinPortNumber
                        || tmpOptions.m_generalOptions.m_ipv6Port > net::kMaxPortNumber))
            throw InvalidConfigurationError("Invalid IPv6 server port number");
    }

    // Parse data directory
    {
        tmpOptions.m_generalOptions.m_dataDirectory = boost::trim_copy(
                config.get<std::string>(constructOptionPath(kGeneralOptionDataDirectory), ""));
        while (!tmpOptions.m_generalOptions.m_dataDirectory.empty()
                && tmpOptions.m_generalOptions.m_dataDirectory.back() == '/') {
            tmpOptions.m_generalOptions.m_dataDirectory.erase(
                    tmpOptions.m_generalOptions.m_dataDirectory.length() - 1);
        }
        if (tmpOptions.m_generalOptions.m_dataDirectory.empty())
            throw InvalidConfigurationError("Data directory not specified or empty");
    }

    // Parse admin connection listener backlog
    {
        const auto adminConnectionListenerBacklog =
                config.get<int>(constructOptionPath(kGeneralOptionAdminConnectionListenerBacklog),
                        kDefaultAdminConnectionListenerBacklog);
        if (adminConnectionListenerBacklog < 1
                || adminConnectionListenerBacklog > kMaxAdminConnectionListenerBacklog) {
            throw InvalidConfigurationError(
                    "Admin connection listener backlog value is out of range");
        }
        tmpOptions.m_generalOptions.m_adminConnectionListenerBacklog =
                adminConnectionListenerBacklog;
    }

    // Parse max number of admin connections
    {
        const auto maxAdminConnections =
                config.get<unsigned>(constructOptionPath(kGeneralOptionMaxAdminConnections),
                        kDefaultMaxAdminConnections);
        if (maxAdminConnections < 1 || maxAdminConnections > kMaxMaxAdminConnections) {
            throw InvalidConfigurationError("Max. number of admin connections is out of range");
        }
        tmpOptions.m_generalOptions.m_maxAdminConnections = maxAdminConnections;
    }

    // Parse user connection listener backlog
    {
        const auto userConnectionListenerBacklog =
                config.get<int>(constructOptionPath(kGeneralOptionUserConnectionListenerBacklog),
                        kDefaultUserConnectionListenerBacklog);
        if (userConnectionListenerBacklog < 1
                || userConnectionListenerBacklog > kMaxUserConnectionListenerBacklog) {
            throw InvalidConfigurationError(
                    "User connection listener backlog value is out of range");
        }
        tmpOptions.m_generalOptions.m_userConnectionListenerBacklog = userConnectionListenerBacklog;
    }

    // Parse max number of user connections
    {
        const auto maxUserConnections = config.get<unsigned>(
                constructOptionPath(kGeneralOptionMaxUserConnections), kDefaultMaxUserConnections);
        if (maxUserConnections < 1 || maxUserConnections > kMaxMaxUserConnections) {
            throw InvalidConfigurationError("Max. number of user connections is out of range");
        }
        tmpOptions.m_generalOptions.m_maxUserConnections = maxUserConnections;
    }

    // Parse dead connection cleanup period in seconds
    {
        const auto value = config.get<unsigned>(
                constructOptionPath(kGeneralOptionDeadConnectionCleanupInterval),
                kDefaultOptionDeadConnectionCleanupInterval);
        if (value < kMinOptionDeadConnectionCleanupInterval)
            throw InvalidConfigurationError("Dead connection cleanup interval is too small");
        if (value > kMaxOptionDeadConnectionCleanupInterval)
            throw InvalidConfigurationError("Dead connection cleanup interval is too big");
        tmpOptions.m_generalOptions.m_deadConnectionCleanupInterval = value;
    }

    // Parse "allow group permission on the config files" flag
    tmpOptions.m_generalOptions.m_allowGroupPermissionsOnConfigFiles =
            config.get<bool>(constructOptionPath(kGeneralOptionAllowGroupPermissionsOnConfigFiles),
                    kDefaultOptionAllowGroupPermissionsOnConfigFiles, BoolTranslator());

    // Parse "enable REST Server" flag
    tmpOptions.m_generalOptions.m_enableRestServer =
            config.get<bool>(constructOptionPath(kGeneralOptionEnableRestServer),
                    kDefaultOptionEnableRestServer, BoolTranslator());

    // Log options

    {
        // Collect and validate log channel names
        std::vector<std::string> channels;
        {
            std::unordered_set<std::string> knownChannels;
            const auto value = boost::trim_copy(
                    config.get<std::string>(constructOptionPath(kGeneralOptionLogChannels), ""));
            boost::split(channels, value, boost::is_any_of(","));
            for (auto& v : channels) {
                boost::trim(v);
                if (v.empty()) throw InvalidConfigurationError("Empty log channel name detected");
                if (!knownChannels.insert(v).second) {
                    std::ostringstream err;
                    err << "Duplicate log channel name " << v;
                    throw InvalidConfigurationError(err.str());
                }
            }
        }

        // Check that we have at least one log channel
        if (channels.empty()) throw InvalidConfigurationError("No log channels defined");

        // Parse log channel options
        for (const auto& logChannelName : channels) {
            const auto channelOptionPrefix = "log." + logChannelName + ".";

            LogChannelOptions channelOptions;
            channelOptions.m_name = logChannelName;

            // Channel type
            {
                const auto path = constructOptionPath(channelOptionPrefix + kLogChannelOptionType);
                const auto channelType = config.get<std::string>(path, "");
                if (channelType.empty()) {
                    std::ostringstream err;
                    err << "Type not defined for the log channel " << logChannelName;
                    throw InvalidConfigurationError(err.str());
                }
                if (channelType == "console")
                    channelOptions.m_type = LogChannelType::kConsole;
                else if (channelType == "file")
                    channelOptions.m_type = LogChannelType::kFile;
                else {
                    std::ostringstream err;
                    err << "Unsupported channel type '" << channelType
                        << "' specified for the log channel " << logChannelName;
                    throw InvalidConfigurationError(err.str());
                }
            }

            // Destination
            {
                const auto path =
                        constructOptionPath(channelOptionPrefix + kLogChannelOptionDestination);
                channelOptions.m_destination = boost::trim_copy(config.get<std::string>(path, ""));
                if (channelOptions.m_destination.empty()) {
                    std::ostringstream err;
                    err << "Destination not defined for the log channel " << logChannelName;
                    throw InvalidConfigurationError(err.str());
                }
            }

            // Max. file size
            try {
                const auto path =
                        constructOptionPath(channelOptionPrefix + kLogChannelOptionMaxFileSize);
                auto option = boost::trim_copy(config.get<std::string>(
                        path, std::to_string(defaults::kDefaultMaxLogFileSize / kBytesInMB)));
                off_t multiplier = 0;
                if (option.size() > 1) {
                    const auto lastChar = option.back();
                    switch (lastChar) {
                        case 'k':
                        case 'K': {
                            multiplier = kBytesInKB;
                            break;
                        }
                        case 'm':
                        case 'M': {
                            multiplier = kBytesInMB;
                            break;
                        }
                        case 'g':
                        case 'G': {
                            multiplier = kBytesInGB;
                            break;
                        }
                        default: break;
                    }
                    if (multiplier > 0) option.erase(option.length() - 1, 1);
                }
                if (multiplier == 0) multiplier = kBytesInMB;
                auto value = std::stoull(option);
                if (value == 0) throw std::out_of_range("value is zero");
                const auto upperLimit = defaults::kMaxMaxLogFileSize / multiplier;
                if (value > static_cast<unsigned long long>(upperLimit))
                    throw std::out_of_range("value is too big");
                channelOptions.m_maxLogFileSize = value * multiplier;
            } catch (std::exception& ex) {
                std::ostringstream err;
                err << "Invalid value of max. file size for the log channel " << logChannelName
                    << ": " << ex.what();
                throw InvalidConfigurationError(err.str());
            }

            // Max.number of files
            try {
                const auto path =
                        constructOptionPath(channelOptionPrefix + kLogChannelOptionMaxFiles);
                auto option = boost::trim_copy(config.get<std::string>(
                        path, std::to_string(defaults::kDefaultMaxLogFilesCount)));
                auto maxFiles = std::stoull(option);
                if (maxFiles == 0) throw std::out_of_range("value is zero");
                channelOptions.m_maxFiles = maxFiles;
            } catch (std::exception& ex) {
                std::ostringstream err;
                err << "Invalid value of max. number of log files for the log channel "
                    << logChannelName << ": " << ex.what();
                throw InvalidConfigurationError(err.str());
            }

            // Expiration time
            try {
                const auto path =
                        constructOptionPath(channelOptionPrefix + kLogChannelOptionExpirationTime);
                auto option = boost::trim_copy(config.get<std::string>(
                        path, std::to_string(
                                      defaults::kDefaultLogFileExpirationTimeout / kSecondsInDay)));
                std::time_t multiplier = 0;
                if (option.size() > 1) {
                    const auto lastChar = option.back();
                    switch (lastChar) {
                        case 's':
                        case 'S': {
                            multiplier = 1;
                            break;
                        }
                        case 'm':
                        case 'M': {
                            multiplier = kSecondsInMinute;
                            break;
                        }
                        case 'h':
                        case 'H': {
                            multiplier = kSecondsInHour;
                            break;
                        }
                        case 'd':
                        case 'D': {
                            multiplier = kSecondsInDay;
                            break;
                        }
                        case 'w':
                        case 'W': {
                            multiplier = kSecondsInWeek;
                            break;
                        }
                        default: break;
                    }
                    if (multiplier > 1) option.erase(option.length() - 1, 1);
                }
                if (multiplier == 0) multiplier = kSecondsInDay;
                std::time_t value = std::stoul(option);
                const auto upperLimit = defaults::kMaxLogFileExpirationTimeout / multiplier;
                if (value > upperLimit) throw std::out_of_range("value is too big");
                channelOptions.m_logFileExpirationTimeout = value * multiplier;
            } catch (std::exception& ex) {
                std::ostringstream err;
                err << "Invalid value of expiration time for the log channel " << logChannelName
                    << ": " << ex.what();
                throw InvalidConfigurationError(err.str());
            }

            // Severity
            {
                const auto path =
                        constructOptionPath(channelOptionPrefix + kLogChannelOptionSeverity);
                const auto option = boost::trim_copy(config.get<std::string>(
                        path, logLevelNames[static_cast<std::size_t>(
                                      defaults::kDefaultLogSeverityLevel)]));
                std::size_t i = 0;
                for (; i < logLevelNames.size(); ++i) {
                    if (strcasecmp(option.c_str(), logLevelNames[i]) == 0) {
                        channelOptions.m_severity =
                                static_cast<boost::log::trivial::severity_level>(i);
                        break;
                    }
                }
                if (i == logLevelNames.size()) {
                    std::ostringstream err;
                    err << "Invalid log severity level for the log channel " << logChannelName;
                    throw InvalidConfigurationError(err.str());
                }
                channelOptions.m_severity = static_cast<boost::log::trivial::severity_level>(i);
            }

            tmpOptions.m_logOptions.m_logChannels.push_back(std::move(channelOptions));

        }  // for (channels)
    }

    // IOManager options

    // Parse IPv4 port number
    {
        tmpOptions.m_ioManagerOptions.m_ipv4SqlPort =
                config.get<int>(constructOptionPath(kIOManagerOptionIpv4SqlPort),
                        kDefaultIOManagerIpv4SqlPortNumber);
        if (tmpOptions.m_ioManagerOptions.m_ipv4SqlPort != 0
                && (tmpOptions.m_ioManagerOptions.m_ipv4SqlPort < net::kMinPortNumber
                        || tmpOptions.m_ioManagerOptions.m_ipv4SqlPort > net::kMaxPortNumber))
            throw InvalidConfigurationError("Invalid IO Manager IPv4 SQL port number");
    }

    // Parse IPv6 port number
    {
        tmpOptions.m_ioManagerOptions.m_ipv6SqlPort =
                config.get<int>(constructOptionPath(kIOManagerOptionIpv6SqlPort),
                        kDefaultIOManagerIpv6SqlPortNumber);
        if (tmpOptions.m_ioManagerOptions.m_ipv6SqlPort != 0
                && (tmpOptions.m_ioManagerOptions.m_ipv6SqlPort < net::kMinPortNumber
                        || tmpOptions.m_ioManagerOptions.m_ipv6SqlPort > net::kMaxPortNumber))
            throw InvalidConfigurationError("Invalid IO Manager IPv6 port number");
    }

    // Parse IPv4 port number
    {
        tmpOptions.m_ioManagerOptions.m_ipv4RestPort =
                config.get<int>(constructOptionPath(kIOManagerOptionIpv4RestPort),
                        kDefaultIOManagerIpv4RestPortNumber);
        if (tmpOptions.m_ioManagerOptions.m_ipv4RestPort != 0
                && (tmpOptions.m_ioManagerOptions.m_ipv4RestPort < net::kMinPortNumber
                        || tmpOptions.m_ioManagerOptions.m_ipv4RestPort > net::kMaxPortNumber))
            throw InvalidConfigurationError("Invalid IO Manager IPv4 REST port number");
    }

    // Parse IPv6 port number
    {
        tmpOptions.m_ioManagerOptions.m_ipv6RestPort =
                config.get<int>(constructOptionPath(kIOManagerOptionIpv6RestPort),
                        kDefaultIOManagerIpv6RestPortNumber);
        if (tmpOptions.m_ioManagerOptions.m_ipv6RestPort != 0
                && (tmpOptions.m_ioManagerOptions.m_ipv6RestPort < net::kMinPortNumber
                        || tmpOptions.m_ioManagerOptions.m_ipv6RestPort > net::kMaxPortNumber))
            throw InvalidConfigurationError("Invalid IO Manager IPv6 REST port number");
    }

    // Parse worker thread number
    {
        tmpOptions.m_ioManagerOptions.m_workerThreadNumber =
                config.get<unsigned>(constructOptionPath(kIOManagerOptionWorkerThreadNumber),
                        kDefaultIOManagerWorkerThreadNumber);
        if (tmpOptions.m_ioManagerOptions.m_workerThreadNumber < 1) {
            throw InvalidConfigurationError("Number of IO Manager worker threads is out of range");
        }
    }

    // Parse writer thread number
    {
        tmpOptions.m_ioManagerOptions.m_writerThreadNumber =
                config.get<unsigned>(constructOptionPath(kIOManagerOptionWriterThreadNumber),
                        kDefaultIOManagerWriterThreadNumber);
        if (tmpOptions.m_ioManagerOptions.m_writerThreadNumber < 1) {
            throw InvalidConfigurationError("Number of IO Manager writer threads is out of range");
        }
    }

    // Parse user cache capacity
    {
        tmpOptions.m_ioManagerOptions.m_userCacheCapacity =
                config.get<unsigned>(constructOptionPath(kIOManagerOptionUserCacheCapacity),
                        kDefaultIOManagerUserCacheCapacity);
        if (tmpOptions.m_ioManagerOptions.m_userCacheCapacity < kMinIOManagerUserCacheCapacity) {
            throw InvalidConfigurationError("IO Manager user cache capacity is too small");
        }
    }

    // Parse database cache capacity
    {
        tmpOptions.m_ioManagerOptions.m_databaseCacheCapacity =
                config.get<unsigned>(constructOptionPath(kIOManagerOptionDatabaseCacheCapacity),
                        kDefaultIOManagerDatabaseCacheCapacity);
        if (tmpOptions.m_ioManagerOptions.m_databaseCacheCapacity
                < kMinIOManagerDatabaseCacheCapacity) {
            throw InvalidConfigurationError("IO Manager database cache capacity is too small");
        }
    }

    // Parse table cache capacity
    {
        tmpOptions.m_ioManagerOptions.m_tableCacheCapacity =
                config.get<unsigned>(constructOptionPath(kIOManagerOptionTableCacheCapacity),
                        kDefaultIOManagerTableCacheCapacity);
        if (tmpOptions.m_ioManagerOptions.m_tableCacheCapacity < kMinIOManagerTableCacheCapacity) {
            throw InvalidConfigurationError("IO Manager table cache capacity is too small");
        }
    }

    // Parse block cache capacity
    {
        tmpOptions.m_ioManagerOptions.m_blockCacheCapacity =
                config.get<unsigned>(constructOptionPath(kIOManagerOptionBlockCacheCapacity),
                        kDefaultIOManagerBlockCacheCapacity);
        if (tmpOptions.m_ioManagerOptions.m_blockCacheCapacity < kMinIOManagerBlockCacheCapacity) {
            throw InvalidConfigurationError("IO Manager block cache capacity is too small");
        }
    }

    // Parse dead connection cleanup period in seconds
    {
        const auto value = config.get<unsigned>(
                constructOptionPath(kIOManagerOptionDeadConnectionCleanupInterval),
                kDefaultIOManagerOptionDeadConnectionCleanupInterval);
        if (value < kMinIOManagerOptionDeadConnectionCleanupInterval) {
            throw InvalidConfigurationError(
                    "IO Manager dead connection cleanup interval is too small");
        }
        if (value > kMaxIOManagerOptionDeadConnectionCleanupInterval) {
            throw InvalidConfigurationError(
                    "IO Manager dead connection cleanup interval is too big");
        }
        tmpOptions.m_ioManagerOptions.m_deadConnectionCleanupInterval = value;
    }

    // Parse maximum JSON payload size
    try {
        const auto path = constructOptionPath(kIOManagerOptionMaxJsonPayloadSize);
        auto option = boost::trim_copy(config.get<std::string>(
                path, std::to_string(kDefaultIOManagerOptionMaxJsonPayloadSize / kBytesInKB)));
        off_t multiplier = 0;
        if (option.size() > 1) {
            const auto lastChar = option.back();
            switch (lastChar) {
                case 'k':
                case 'K': {
                    multiplier = kBytesInKB;
                    break;
                }
                case 'm':
                case 'M': {
                    multiplier = kBytesInMB;
                    break;
                }
                case 'g':
                case 'G': {
                    multiplier = kBytesInGB;
                    break;
                }
                default: break;
            }
            if (multiplier > 0) option.erase(option.length() - 1, 1);
        }
        if (multiplier == 0) multiplier = kBytesInKB;
        auto value = std::stoull(option);
        if (value == 0) throw std::out_of_range("value is zero");
        const auto upperLimit = kMaxIOManagerOptionMaxJsonPayloadSize / multiplier;
        if (value > upperLimit) throw std::out_of_range("value is too big");
        tmpOptions.m_ioManagerOptions.m_maxJsonPayloadSize = value * multiplier;
    } catch (std::exception& ex) {
        std::ostringstream err;
        err << "Invalid value of max. JSON payload size: " << ex.what();
        throw InvalidConfigurationError(err.str());
    }

    // Encryption options

    // Parse default cipher ID
    tmpOptions.m_encryptionOptions.m_defaultCipherId = boost::trim_copy(config.get<std::string>(
            constructOptionPath(kEncryptionOptionDefaultCipherId), kDefaultCipherId));

    // Parse master cipher ID
    tmpOptions.m_encryptionOptions.m_masterCipherId = boost::trim_copy(
            config.get<std::string>(constructOptionPath(kEncryptionOptionMasterCipherId),
                    tmpOptions.m_encryptionOptions.m_defaultCipherId));

    // Parse master cipher key path
    tmpOptions.m_encryptionOptions.m_masterCipherKeyPath = boost::trim_copy(
            config.get<std::string>(constructOptionPath(kEncryptionOptionMasterKey),
                    composeDefaultMasterEncryptionKeyFilePath(instanceName)));

    // Parse system database cipher ID
    tmpOptions.m_encryptionOptions.m_systemDbCipherId = boost::trim_copy(
            config.get<std::string>(constructOptionPath(kEncryptionOptionSystemDbCipherId),
                    tmpOptions.m_encryptionOptions.m_defaultCipherId));

    // Client options
    tmpOptions.m_clientOptions.m_enableEncryption =
            config.get<bool>(constructOptionPath(kClientOptionEnableEncryption),
                    kDefaultClientEnableEncryption, BoolTranslator());
    if (tmpOptions.m_clientOptions.m_enableEncryption) {
        tmpOptions.m_clientOptions.m_tlsCertificate = boost::trim_copy(
                config.get<std::string>(constructOptionPath(kClientOptionTlsCertificate), ""));

        tmpOptions.m_clientOptions.m_tlsCertificateChain = boost::trim_copy(
                config.get<std::string>(constructOptionPath(kClientOptionTlsCertificateChain), ""));

        tmpOptions.m_clientOptions.m_tlsPrivateKey = boost::trim_copy(
                config.get<std::string>(constructOptionPath(kClientOptionTlsPrivateKey), ""));

        if (tmpOptions.m_clientOptions.m_tlsCertificate.empty()
                && tmpOptions.m_clientOptions.m_tlsCertificate.empty())
            throw std::runtime_error(
                    "Client certificate or certificate chain must be set to create a "
                    "TLS connection");

        if (tmpOptions.m_clientOptions.m_tlsPrivateKey.empty())
            throw std::runtime_error("Client TLS private keys is empty");
    }

    // Check ports
    std::unordered_map<int, std::string> ports;
    constexpr const char* kConflictsWith = " conflicts with ";

    const char* portName = "Database IPv4 port";
    if (tmpOptions.m_generalOptions.m_ipv4Port != 0)
        ports.emplace(tmpOptions.m_generalOptions.m_ipv4Port, portName);

    portName = "Database IPv6 port";
    if (tmpOptions.m_generalOptions.m_ipv6Port != 0) {
        const auto result = ports.emplace(tmpOptions.m_generalOptions.m_ipv4Port, portName);
        if (!result.second) {
            std::ostringstream err;
            err << portName << kConflictsWith << result.first->second;
            throw std::runtime_error(err.str());
        }
    }

    portName = "IO Manager SQL IPv4 port";
    if (tmpOptions.m_ioManagerOptions.m_ipv4SqlPort != 0) {
        const auto result = ports.emplace(tmpOptions.m_ioManagerOptions.m_ipv4SqlPort, portName);
        if (!result.second) {
            std::ostringstream err;
            err << portName << kConflictsWith << result.first->second;
            throw std::runtime_error(err.str());
        }
    }

    portName = "IO Manager SQL IPv6 port";
    if (tmpOptions.m_ioManagerOptions.m_ipv6SqlPort != 0) {
        const auto result = ports.emplace(tmpOptions.m_ioManagerOptions.m_ipv6SqlPort, portName);
        if (!result.second) {
            std::ostringstream err;
            err << portName << kConflictsWith << result.first->second;
            throw std::runtime_error(err.str());
        }
    }

    portName = "IO Manager REST IPv4 port";
    if (tmpOptions.m_ioManagerOptions.m_ipv4RestPort != 0) {
        const auto result = ports.emplace(tmpOptions.m_ioManagerOptions.m_ipv4RestPort, portName);
        if (!result.second) {
            std::ostringstream err;
            err << portName << kConflictsWith << result.first->second;
            throw std::runtime_error(err.str());
        }
    }

    portName = "IO Manager REST IPv6 port";
    if (tmpOptions.m_ioManagerOptions.m_ipv6RestPort != 0) {
        const auto result = ports.emplace(tmpOptions.m_ioManagerOptions.m_ipv6RestPort, portName);
        if (!result.second) {
            std::ostringstream err;
            err << portName << kConflictsWith << result.first->second;
            throw std::runtime_error(err.str());
        }
    }

    // All options valid, save them
    *this = std::move(tmpOptions);
}

}  // namespace siodb::config
