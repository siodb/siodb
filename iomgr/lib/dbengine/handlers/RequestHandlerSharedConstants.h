// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/** Row size that indicates "no more rows" */
static constexpr std::uint64_t kNoMoreRows = 0;

/** LOB chunk size */
static constexpr std::uint32_t kLobChunkSize = 4096;

/** JSON chunk size */
static constexpr std::size_t kJsonChunkSize = 65536;

/** REST status code field name */
static constexpr const char* kRestStatusCodeFieldName = "status";

/** REST rows field name */
static constexpr const char* kRestRowsFieldName = "rows";

}  // namespace siodb::iomgr::dbengine
