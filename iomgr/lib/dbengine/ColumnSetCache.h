// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnSetPtr.h"

// Common project headers
#include <siodb/common/stl_ext/lru_cache.h>

namespace siodb::iomgr::dbengine {

/** LRU cache for column sets */
class ColumnSetCache : public stdext::ordered_lru_cache<std::uint64_t, ColumnSetPtr> {
private:
    using Base = stdext::ordered_lru_cache<std::uint64_t, ColumnSetPtr>;

public:
    /**
     * Initializes object of class ColumnSetCache.
     * @param capacity Cache capacity.
     */
    explicit ColumnSetCache(std::size_t capacity) noexcept
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
    virtual bool can_evict([[maybe_unused]] const std::uint64_t& key,
            const ColumnSetPtr& value) const noexcept override final
    {
        return value.use_count() == 1;
    }
};

}  // namespace siodb::iomgr::dbengine
