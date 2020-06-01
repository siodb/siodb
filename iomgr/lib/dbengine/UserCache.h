// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "UserPtr.h"

// Common project headers
#include <siodb/common/stl_ext/lru_cache.h>

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/** User LRU cache */
class UserCache : public stdext::unordered_lru_cache<std::uint32_t, UserPtr> {
private:
    using Base = stdext::unordered_lru_cache<std::uint32_t, UserPtr>;

public:
    /**
     * Initializes object of class UserCache.
     * @param initialCapacity Initial capacity.
     */
    UserCache(std::size_t initialCapacity)
        : Base(initialCapacity)
    {
    }

private:
    /** Evicts most outdated element from cache which is allowed to be evicted */
    void evict() override;

    /** Returns indication that item can be evicted */
    bool can_evict(const std::uint32_t& key, const UserPtr& database) const noexcept override;
};

}  // namespace siodb::iomgr::dbengine
