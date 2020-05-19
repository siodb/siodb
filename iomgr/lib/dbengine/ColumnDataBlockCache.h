// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDataBlockPtr.h"

// Common project headers
#include <siodb/common/utils/UnorderedLruCache.h>

namespace siodb::iomgr::dbengine {

/** LRU cache for data blocks */
class ColumnDataBlockCache final
    : public utils::unordered_lru_cache<std::uint64_t, ColumnDataBlockPtr> {
private:
    using Base = utils::unordered_lru_cache<std::uint64_t, ColumnDataBlockPtr>;

public:
    /**
     * Initializes object of class ColumnDataBlockCache.
     * @param capacity Cache capacity.
     */
    explicit ColumnDataBlockCache(std::size_t capacity) noexcept
        : Base(capacity)
    {
    }

private:
    /**
     * Returns indication if item can be evicted.
     * @param key A key.
     * @param value A value.
     * @return true if item can be evicted, false otherwise.
     */
    bool can_evict(const std::uint64_t& key, const ColumnDataBlockPtr& value) const
            noexcept override;

    /**
     * Called before item gets evicted from the cache.
     * @param key A key.
     * @param value A value.
     * @param clearingCache Indication of that cache is being completely cleared.
     */
    void on_evict(
            const std::uint64_t& key, ColumnDataBlockPtr& value, bool clearingCache) const override;
};

}  // namespace siodb::iomgr::dbengine
