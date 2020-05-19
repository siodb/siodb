// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

// System headers
#include <unistd.h>

namespace siodb {

/**
 * Checks that SIODB database instance is running.
 * @param instanceName Database instance name.
 * @param serverExecutable Server executable path.
 * @return Database server process PID if instance is running or -1 if instance not found.
 * @throw std::system_error On I/O errors.
 * @throw std::runtime_error On various errors.
 */
pid_t checkInstance(const std::string& instanceName, const std::string& serverExecutable);

/**
 * Validates database instance. Returns if instance is known, throws exception otherwise.
 * @param instanceName Database instance name.
 * @throw std::invalid_argument If instance name is invalid or such instance is not known.
 */
void validateInstance(const std::string& instanceName);

/**
 * Validates database instance name.
 * @param instanceName Database instance name.
 * @return true if instance name is valid, false otherwise.
 */
bool validateInstanceName(const std::string& instanceName);

/**
 * Composes database instance configuration file path.
 * @param instanceName Database instance name.
 * @return Database instance configuration file path.
 */
std::string composeInstanceConfigFilePath(const std::string& instanceName);

/**
 * Composes instance system database encryption key file path.
 * @param instanceName Database instance name.
 * @return Instance system database encryption key file path.
 */
std::string composeInstanceSysDbEncryptionKeyFilePath(const std::string& instanceName);

/**
 * Composes instance initial super user access key file path.
 * @param instanceName Database instance name.
 * @return Initial super user access key file path.
 */
std::string composeInstanceInitialSuperUserAccessKeyFilePath(const std::string& instanceName);

/**
 * Composes instance initialization lock file path.
 * @param instanceName Database instance name.
 * @return Instance initialization lock file path.
 */
std::string composeInstanceInitializationLockFilePath(const std::string& instanceName);

/**
 * Composes iomgr initialization flag file path.
 * @param instanceName Database instance name.
 * @return iomgr initialization flag file path.
 */
std::string composeIomgrInitializionFlagFilePath(const std::string& instanceName);

}  // namespace siodb
