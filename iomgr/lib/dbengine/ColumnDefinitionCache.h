// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDefinitionPtr.h"

// Common project headers
#include <siodb/common/utils/UnorderedLruCache.h>

namespace siodb::iomgr::dbengine {

class ColumnDefinition;

/** LRU cache for column definitions */
class ColumnDefinitionCache final
    : public utils::unordered_lru_cache<std::uint64_t, ColumnDefinitionPtr> {
private:
    using Base = utils::unordered_lru_cache<std::uint64_t, ColumnDefinitionPtr>;

public:
    /**
     * Initializes object of class ColumnDefinitionCache.
     * @param capacity Cache capacity.
     */
    explicit ColumnDefinitionCache(std::size_t capacity) noexcept
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
    bool can_evict([[maybe_unused]] const std::uint64_t& key,
            const ColumnDefinitionPtr& value) const noexcept override;
};

}  // namespace siodb::iomgr::dbengine
