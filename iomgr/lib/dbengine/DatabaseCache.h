// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "DatabasePtr.h"

// Common project headers
#include <siodb/common/stl_ext/lru_cache.h>

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/** Database LRU cache */
class DatabaseCache : public stdext::unordered_lru_cache<std::uint32_t, DatabasePtr> {
private:
    using Base = stdext::unordered_lru_cache<std::uint32_t, DatabasePtr>;

public:
    /**
     * Initializes object of class DatabaseCache.
     * @param initialCapacity Initial capacity.
     */
    DatabaseCache(std::size_t initialCapacity)
        : Base(initialCapacity)
    {
    }

private:
    /** Evicts most outdated element from cache which is allowed to be evicted */
    void evict() override;

    /** Returns indication that item can be evicted */
    bool can_evict(const std::uint32_t& key, const DatabasePtr& database) const noexcept override;
};

}  // namespace siodb::iomgr::dbengine
