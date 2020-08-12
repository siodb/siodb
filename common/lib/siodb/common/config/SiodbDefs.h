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

/** Siodb system database encryption key file */
constexpr const char* kInstanceSystemDbEncryptionKeyFile = "system_db_key";

/** Siodb initial supoer user access key file */
constexpr const char* kInstanceInitialSuperUserAccessKeyFile = "initial_access_key";

/** Siodb instance configuration file */
constexpr const char* kInstanceConfigFile = "config";

/** Siodb admin group */
constexpr const char* kAdminGroupName = "siodb";

/** Siodb service user */
constexpr const char* kServiceUserName = "siodb";

/** Siodb conn_worker executable */
constexpr const char* kUserConnectionWorkerExecutable = "siodb_conn_worker";

/** Siodb iomgr executable */
constexpr const char* kIOManagerExecutable = "siodb_iomgr";

/** Siodb internal time intervals */
constexpr int kUserConnectionWorkerShutdownTimeoutMs = 5 * 1000;
constexpr auto kIomgrInitializationCheckPeriod = std::chrono::seconds(1);

/** Siocli editor history size */
constexpr const int kClientEditorHistorySize = 100;

/** Maximum instance name size */
constexpr std::size_t kMaxInstanceNameLength = 63;

/** Data file extension */
constexpr const char* kDataFileExtension = ".siodf";

/** Temporary file extension */
constexpr const char* kTempFileExtension = ".tmp";

/** IO Manager initialization file extension */
constexpr const char* kIomgrInitializationFlagFileExtension = ".initialized";

/** Instance lock file extension */
constexpr const char* kInstanceLockFileExtension = ".lock";

/** Data file creation mode */
constexpr mode_t kDataFileCreationMode = 0660;

/** Lock file creation mode(write only) */
constexpr mode_t kLockFileCreationMode = 0220;

/** Data file header size */
constexpr std::size_t kDataFileHeaderSize = 1 * 1024;  // 1K

/** Data file data area size. Must be multiple of the index node size (typically 8K). */
constexpr std::size_t kDefaultDataFileDataAreaSize = 10 * 1024 * 1024;  // 10M

/** System table data file data area size. Must be multiple of index node size (typically 8K). */
constexpr std::size_t kSystemTableDataFileDataAreaSize = 128 * 1024;  // 128K

/** Data file size */
constexpr off_t kDefaultDataFileSize = kDataFileHeaderSize + kDefaultDataFileDataAreaSize;

/** System table data file size */
constexpr off_t kSystemTableDataFileSize = kDataFileHeaderSize + kSystemTableDataFileDataAreaSize;

/** Current data file version */
constexpr std::uint32_t kCurrentDataFileVersion = 1;

/** Maximum length of string */
constexpr const std::size_t kMaxStringLength = 0xFFFF;

/** Maximum length of CLOB */
constexpr const std::size_t kMaxClobLength = 0xFFFFFFFF;

/** Maximum length of binary */
constexpr const std::size_t kMaxBinaryLength = 0xFFFF;

/** Maximum length of BLOB */
constexpr const std::size_t kMaxBlobLength = 0xFFFFFFFF;

/** Minumum challenge length  */
constexpr const std::size_t kMinChallengeSize = 128;

/** Maximum challenge length   */
constexpr const std::size_t kMaxChallengeSize = 1024;

/** Maximum user access key size  */
constexpr const std::size_t kMaxUserAccessKeySize = 8 * 1024;

}  // namespace siodb
