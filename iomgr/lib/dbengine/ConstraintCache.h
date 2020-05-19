// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ConstraintPtr.h"

// Common project headers
#include <siodb/common/utils/UnorderedLruCache.h>

namespace siodb::iomgr::dbengine {

class Constraint;
class Table;

/** LRU cache for constraints */
class ConstraintCache : public utils::unordered_lru_cache<std::uint64_t, ConstraintPtr> {
private:
    using Base = utils::unordered_lru_cache<std::uint64_t, ConstraintPtr>;

public:
    /**
     * Initializes object of class ConstraintCache.
     * @param table Parent table.
     * @param capacity Cache capacity.
     */
    explicit ConstraintCache(const Table& table, std::size_t capacity)
        : Base(capacity)
        , m_table(table)
    {
    }

private:
    /** Evicts most outdated element from cache which is allowed to be evicted */
    virtual void evict() override;

    /**
     * Returns indication if item can be evicted.
     * @param key A key.
     * @param value A value.
     * @return true if item can be evicted, false otherwise.
     */
    virtual bool can_evict([[maybe_unused]] const std::uint64_t& key,
            const ConstraintPtr& value) const noexcept override final;

private:
    /** Parent table. */
    const Table& m_table;
};

}  // namespace siodb::iomgr::dbengine
