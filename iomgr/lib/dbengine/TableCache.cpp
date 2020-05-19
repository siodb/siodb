// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TableCache.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Table.h"
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

void TableCache::evict()
{
    try {
        Base::evict();
    } catch (utils::LruCacheFullError&) {
        throwDatabaseError(IOManagerMessageId::kErrorTableCacheFull, m_databaseName);
    }
}

bool TableCache::can_evict([[maybe_unused]] const std::uint32_t& key, const TablePtr& table) const
        noexcept
{
    return !table->isSystemTable() && table.use_count() == 1;
}

}  // namespace siodb::iomgr::dbengine
