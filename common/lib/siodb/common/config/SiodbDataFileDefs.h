// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstddef>
#include <cstdint>

// System headers
#include <sys/types.h>

namespace siodb {

/** Data file extension */
constexpr const char* kDataFileExtension = ".siodf";

/** Temporary file extension */
constexpr const char* kTempFileExtension = ".tmp";

/** Initialization flag file */
constexpr const char* kInitializationFlagFile = "initialized";

/** Data file creation mode */
constexpr mode_t kDataFileCreationMode = 0660;

/** Data file header size */
constexpr std::size_t kDataFileHeaderSize = 1 * 1024;  // 1K

/** Data file data area size. Must be multiple of the index node size (typically 8K). */
constexpr std::size_t kDefaultDataFileDataAreaSize = 10 * 1024 * 1024;  // 10M

/** System table data file data area size. Must be multiple of index node size (typically 8K). */
constexpr std::size_t kSystemTableDataFileDataAreaSize = 16 * 1024;  // 16K
//constexpr std::size_t kSystemTableDataFileDataAreaSize = 128 * 1024;  // 128K

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

}  // namespace siodb
