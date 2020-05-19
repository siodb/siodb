// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDefinitionCache.h"

namespace siodb::iomgr::dbengine {

bool ColumnDefinitionCache::can_evict(
        [[maybe_unused]] const std::uint64_t& key, const ColumnDefinitionPtr& value) const noexcept
{
    return value.use_count() == 1;
}

}  // namespace siodb::iomgr::dbengine
