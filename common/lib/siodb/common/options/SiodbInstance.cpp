// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SiodbInstance.h"

// Project headers
#include "SiodbOptions.h"

// Common peroject headers
#include "../config/SiodbDefs.h"
#include "../stl_wrap/filesystem_wrapper.h"
#include "../utils/CheckOSUser.h"

// CRT headers
#include <cctype>
#include <cerrno>
#include <cstring>

// STL headers
#include <sstream>

// Boost headers
#include <boost/algorithm/string.hpp>

// System headers
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

namespace siodb {

namespace {

/**
 * Checks instance file permissions.
 * Intance file require following permissions:
 *   - They are owned by the user "siodb" and group "siodb".
 *   - No one has execute access to those files.
 *   - Owner user has read access to those file.
 *   - Other users and groups don't have any access to those files.
 *   - Owner group members don't have any access to those files
 * @param file File path
 * @param allowGroupPermissions Indicates that group permissions are allowed.
 * @throw std::runtime_error if file permissions are invalid
 */
[[maybe_unused]] void checkInstanceFilePermissions(
        const std::string& file, bool allowGroupPermissions)
{
    struct stat st;
    if (::stat(file.c_str(), &st) < 0) {
        const int errorCode = errno;
        std::ostringstream err;
        err << "Can't stat file '" << file << "\': (" << errorCode << ") "
            << std::strerror(errorCode);
        throw std::runtime_error(err.str());
    }

    std::string name;

    try {
        name = utils::getOsUserName(st.st_uid);
    } catch (std::exception& ex) {
        std::ostringstream err;
        err << "Failed to get owner user info for the instance configuration file '" << file
            << "\': " << ex.what();
        throw std::runtime_error(err.str());
    }

    if (name != kServiceUserName) {
        std::ostringstream err;
        err << "Owner user of the instance configuration file '" << file << '\'' << " is not "
            << kServiceUserName;
        throw std::runtime_error(err.str());
    }

    try {
        name = utils::getOsGroupName(st.st_gid);
    } catch (std::exception& ex) {
        std::ostringstream err;
        err << "Failed to get owner group info for the instance configuration file '" << file
            << "\': " << ex.what();
        throw std::runtime_error(err.str());
    }

    if (name != kAdminGroupName) {
        std::ostringstream err;
        err << "Owner group of the instance configuration file '" << file << '\'' << " is not "
            << kAdminGroupName;
        throw std::runtime_error(err.str());
    }

    std::vector<const char*> invalidPermissions;
    invalidPermissions.reserve(8);

    // User and group must not have execute permission
    if (st.st_mode & S_IXUSR) invalidPermissions.push_back("u+x");
    if (st.st_mode & S_IXGRP) invalidPermissions.push_back("g+x");
    // Others must not have any permissions
    if (st.st_mode & S_IROTH) invalidPermissions.push_back("o+r");
    if (st.st_mode & S_IWOTH) invalidPermissions.push_back("o+w");
    if (st.st_mode & S_IXOTH) invalidPermissions.push_back("o+x");
    // User must have read permission
    if (!(st.st_mode & S_IRUSR)) invalidPermissions.push_back("u-r");

    if (!allowGroupPermissions) {
        // Group must not have read and wite permissions
        if (st.st_mode & S_IRGRP) invalidPermissions.push_back("g+r");
        if (st.st_mode & S_IWGRP) invalidPermissions.push_back("g+w");
    }

    if (!invalidPermissions.empty()) {
        std::ostringstream err;
        err << "Configuration file '" << file << "' has invalid permissions:";
        for (const auto s : invalidPermissions)
            err << ' ' << s;
        throw std::runtime_error(err.str());
    }
}

}  // anonymous namespace

pid_t checkInstance(const std::string& instanceName, const std::string& serverExecutable)
{
    int serverPid = -1;
    const std::string instanceParameter("--instance");
    const fs::path path = "/proc";
    for (const auto& entry : fs::directory_iterator(path)) {
        const auto pidStr = entry.path().filename().string();
        int pid;
        try {
            pid = std::stoi(pidStr);
        } catch (std::exception& ex) {
            // Skip non-numbers
            continue;
        }
        const auto commandLinePath = entry.path().string() + "/cmdline";
        std::string commandLine;
        {
            std::ifstream ifs(commandLinePath);
            if (!ifs.is_open()) {
                // Skip file that can't be opened
                continue;
            }
            if (!std::getline(ifs, commandLine)) {
                // Skip read errors
                continue;
            }
        }

        // Filter out process
        std::vector<std::string> components;
        boost::split(components, commandLine, boost::is_space());
        if (components.size() < 2 || components.front() != serverExecutable) continue;
        auto it = std::find(components.begin(), components.end(), instanceParameter);
        if (it == components.end()) continue;
        ++it;
        if (it == components.end() || *it != instanceName) continue;
        // Found it
        serverPid = pid;
        break;
    }
    return static_cast<pid_t>(serverPid);
}

void validateInstance(const std::string& instanceName)
{
    // Validate instance name
    if (!validateInstanceName(instanceName)) {
        std::ostringstream err;
        err << "Invalid instance name '" << instanceName << "'";
        throw std::invalid_argument(err.str());
    }

    // Check instance configuration file
    const auto configPath = composeInstanceConfigFilePath(instanceName);
    if (!fs::exists(configPath)) {
        std::ostringstream err;
        err << "Instance '" << instanceName << "' is unknown";
        throw std::invalid_argument(err.str());
    }

    config::SiodbOptions options(instanceName);

#if defined(_DEBUG)
    options.m_generalOptions.m_ignorePermissionsOnConfigFiles = true;
#endif

    // Check permissions only in release version
    if (!options.m_generalOptions.m_ignorePermissionsOnConfigFiles) {
        checkInstanceFilePermissions(
                configPath, options.m_generalOptions.m_allowGroupPermissionsOnConfigFiles);
    }

    // Check master encryption key file
    const auto encryptionKeyPath = options.m_encryptionOptions.m_masterCipherKeyPath.empty()
                                           ? composeDefaultMasterEncryptionKeyFilePath(instanceName)
                                           : options.m_encryptionOptions.m_masterCipherKeyPath;
    if (!fs::exists(encryptionKeyPath)) {
        std::ostringstream err;
        err << "Missing master encryption key for the instance '" << instanceName << "'";
        throw std::invalid_argument(err.str());
    }

#if !defined(_DEBUG)
    // Check permissions only in release version
    checkInstanceFilePermissions(
            encryptionKeyPath, options.m_generalOptions.m_allowGroupPermissionsOnConfigFiles);
#endif
}

bool validateInstanceName(const std::string& instanceName)
{
    return !instanceName.empty() && instanceName.length() <= kMaxInstanceNameLength
           && instanceName[0] != '.' && instanceName[0] != '+' && instanceName[0] != '-'
           && instanceName[0] != '_' && !isdigit(instanceName[0])
           && instanceName.find_first_not_of(
                      "_-+.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")
                      == std::string::npos;
}

namespace {

std::string composeConfigFilePath(const std::string& instanceName, const char* configFile)
{
    std::ostringstream str;
    str << config::kSiodbInstanceConfigDirectory << '/' << instanceName << '/' << configFile;
    return str.str();
}

}  // anonymous namespace

std::string composeInstanceConfigFilePath(const std::string& instanceName)
{
    return composeConfigFilePath(instanceName, kInstanceConfigFile);
}

std::string composeDefaultMasterEncryptionKeyFilePath(const std::string& instanceName)
{
    return composeConfigFilePath(instanceName, kDefaultMasterEncryptionKeyFileName);
}

std::string composeInstanceInitialSuperUserAccessKeyFilePath(const std::string& instanceName)
{
    return composeConfigFilePath(instanceName, kInstanceInitialSuperUserAccessKeyFile);
}

std::string composeInstanceInitializationLockFilePath(const std::string& instanceName)
{
    std::ostringstream str;
    str << kInstanceInitializationLockFileDir << instanceName << kInstanceLockFileExtension;
    return str.str();
}

std::string composeIomgrInitializionFlagFilePath(const std::string& instanceName)
{
    std::ostringstream str;
    str << kIOManagerInitalizationFlagFileDir << instanceName << ".initialized";
    return str.str();
}

std::string composeInstanceSocketPath(const std::string& instanceName)
{
    std::ostringstream str;
    str << kInstanceSocketPrefix << instanceName << ".socket";
    return str.str();
}

}  // namespace siodb
