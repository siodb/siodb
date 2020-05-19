// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "TablePtr.h"

// Common project headers
#include <siodb/common/utils/UnorderedLruCache.h>

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/** Table LRU cache */
class TableCache : public utils::unordered_lru_cache<std::uint32_t, TablePtr> {
private:
    using Base = utils::unordered_lru_cache<std::uint32_t, TablePtr>;

public:
    /**
     * Initializes object of class TableCache.
     * @param databaseName Database name.
     * @param initialCapacity Initial capacity.
     */
    TableCache(const std::string& databaseName, std::size_t initialCapacity)
        : Base(initialCapacity)
        , m_databaseName(databaseName)
    {
    }

private:
    /** Evicts most outdated element from cache which is allowed to be evicted */
    void evict() override;

    /** Returns indication that item can be evicted */
    bool can_evict(const std::uint32_t& key, const TablePtr& table) const noexcept override;

private:
    /** Database name */
    const std::string& m_databaseName;
};

}  // namespace siodb::iomgr::dbengine
