// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ConstraintDefinitionPtr.h"

// Common project headers
#include <siodb/common/stl_ext/lru_cache.h>

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

class ConstraintDefinition;
class Database;

/** Table LRU cache */
class ConstraintDefinitionCache
    : public stdext::unordered_lru_cache<std::uint64_t, ConstraintDefinitionPtr> {
private:
    using Base = stdext::unordered_lru_cache<std::uint64_t, ConstraintDefinitionPtr>;

public:
    /**
     * Initializes object of class TableCache.
     * @param database Parent database.
     * @param initialCapacity Initial capacity.
     */
    ConstraintDefinitionCache(const Database& database, std::size_t initialCapacity)
        : Base(initialCapacity)
        , m_database(database)
    {
    }

private:
    /** Evicts most outdated element from cache which is allowed to be evicted */
    virtual void evict() override;

    /** Returns indication that item can be evicted */
    virtual bool can_evict(const std::uint64_t& key, const ConstraintDefinitionPtr& table) const
            noexcept override final;

private:
    /** Parent database */
    const Database& m_database;
};

}  // namespace siodb::iomgr::dbengine
