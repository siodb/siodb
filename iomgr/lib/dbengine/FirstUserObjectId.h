// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/** First non-system user ID */
static constexpr const std::uint32_t kFirstUserUserId = 0x1000;

/** First user database ID */
static constexpr const std::uint32_t kFirstUserDatabaseId = 0x1000;

/** First user table ID */
static constexpr std::uint32_t kFirstUserTableId = 0x10000;

/** First user table column set ID */
static constexpr std::uint64_t kFirstUserTableColumnSetId = 0x1000000;

/** First user table column ID */
static constexpr std::uint64_t kFirstUserTableColumnId = 0x100000;

/** First user table column set column ID */
static constexpr std::uint64_t kFirstUserTableColumnSetColumnId = 0x10000000;

/** First user table constraint definition ID */
static constexpr std::uint64_t kFirstUserTableConstraintDefinitionId = 0x100000;

/** First user table constraint ID */
static constexpr std::uint64_t kFirstUserTableConstraintId = 0x1000000;

/** First user table column definition ID */
static constexpr std::uint64_t kFirstUserTableColumnDefinitionId = 0x1000000;

/** First user table column definition constraint ID */
static constexpr std::uint64_t kFirstUserTableColumnDefinitionConstraintId = 0x10000000;

/** First user table index ID */
static constexpr std::uint64_t kFirstUserTableIndexId = 0x1000000;

/** First user table index column ID */
static constexpr std::uint64_t kFirstUserTableIndexColumnId = 0x10000000;

}  // namespace siodb::iomgr::dbengine
