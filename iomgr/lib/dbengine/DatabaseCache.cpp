// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DatabaseCache.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Database.h"
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

void DatabaseCache::evict()
{
    try {
        Base::evict();
    } catch (stdext::lru_cache_full_error&) {
        throwDatabaseError(IOManagerMessageId::kErrorDatabaseCacheFull);
    }
}

bool DatabaseCache::can_evict(
        [[maybe_unused]] const std::uint32_t& key, const DatabasePtr& database) const noexcept
{
    return !database->isSystemDatabase() && database.use_count() == 1 && !database->isUsed();
}

}  // namespace siodb::iomgr::dbengine
