// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserCache.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"
#include "User.h"

namespace siodb::iomgr::dbengine {

void UserCache::evict()
{
    try {
        Base::evict();
    } catch (utils::LruCacheFullError&) {
        throwDatabaseError(IOManagerMessageId::kErrorUserCacheFull);
    }
}

bool UserCache::can_evict([[maybe_unused]] const std::uint32_t& key, const UserPtr& user) const
        noexcept
{
    return !user->isSuperUser() && user.use_count() == 1;
}

}  // namespace siodb::iomgr::dbengine
