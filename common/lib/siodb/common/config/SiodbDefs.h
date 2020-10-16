// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

// STL headers
#include <chrono>
#include <limits>

// System headers
#include <sys/types.h>

namespace siodb {

/** Instance initialization lock file directory */
constexpr const char* kInstanceInitializationLockFileDir = "/run/lock/siodb/";

/** Siodb Unix sockets path */
constexpr const char* kInstanceSocketPrefix = "/run/siodb/";

/** IO manager initialization flag file directory */
constexpr const char* kIoManagerInitaliationFlagFileDir = "/run/siodb/";

/** Siodb default master encryption key file name */
constexpr const char* kDefaultMasterEncryptionKeyFileName = "master_key";

/** Siodb initial supoer user access key file */
constexpr const char* kInstanceInitialSuperUserAccessKeyFile = "initial_access_key";

/** Siodb instance configuration file */
constexpr const char* kInstanceConfigFile = "config";

/** Siodb admin group */
constexpr const char* kAdminGroupName = "siodb";

/** Siodb service user */
constexpr const char* kServiceUserName = "siodb";

/** Siodb Connection Worker executable */
constexpr const char* kUserConnectionWorkerExecutable = "siodb_conn_worker";

/** Siodb IO Manager executable */
constexpr const char* kIOManagerExecutable = "siodb_iomgr";

/** Siodb REST Server executable */
constexpr const char* kRestServerExecutable = "siodb_rest_server";

/** User connection worker shutdown interval */
constexpr int kUserConnectionWorkerShutdownTimeoutMs = 5 * 1000;

/** IO Manager initialization check period */
constexpr auto kIomgrInitializationCheckPeriod = std::chrono::seconds(1);

/** Maximum instance name size */
constexpr std::size_t kMaxInstanceNameLength = 63;

/** IO Manager initialization file extension */
constexpr const char* kIomgrInitializationFlagFileExtension = ".initialized";

/** Instance lock file extension */
constexpr const char* kInstanceLockFileExtension = ".lock";

/** Lock file creation mode(write only) */
constexpr mode_t kLockFileCreationMode = 0220;

/** Minumum challenge length  */
constexpr const std::size_t kMinChallengeSize = 128;

/** Maximum challenge length   */
constexpr const std::size_t kMaxChallengeSize = 1024;

/** Maximum user access key size  */
constexpr const std::size_t kMaxUserAccessKeySize = 8 * 1024;

/** Siocli editor history size */
constexpr const int kClientEditorHistorySize = 100;

}  // namespace siodb
