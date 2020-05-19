// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDataBlockCache.h"

namespace siodb::iomgr::dbengine {

bool ColumnDataBlockCache::can_evict(
        [[maybe_unused]] const std::uint64_t& key, const ColumnDataBlockPtr& value) const noexcept
{
    return value.use_count() == 1;
}

void ColumnDataBlockCache::on_evict([[maybe_unused]] const std::uint64_t& key,
        [[maybe_unused]] ColumnDataBlockPtr& value, [[maybe_unused]] bool clearingCache) const
{
    // save block if buffered in memory and has changes
}

}  // namespace siodb::iomgr::dbengine
