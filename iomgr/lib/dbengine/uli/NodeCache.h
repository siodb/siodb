// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "NodePtr.h"
#include "../DebugDbEngine.h"

// Common project headers
#include <siodb/common/utils/UnorderedLruCache.h>

namespace siodb::iomgr::dbengine::uli {

struct FileData;

/** Regular cache of recently used nodes */
class NodeCache final : public utils::unordered_lru_cache<std::uint64_t, NodePtr> {
private:
    using Base = utils::unordered_lru_cache<std::uint64_t, NodePtr>;

public:
    /**
     * Initializes object of class NodeCache.
     * @param owner Owner object.
     * @param indexFileId Index file ID.
     * @param capacity Cache capacity (maximum allowed size).
     */
    explicit NodeCache(const FileData& owner, std::uint64_t indexFileId, std::size_t capacity)
        : Base(capacity)
        , m_owner(owner)
        , m_indexFileId(indexFileId)
    {
    }

    /** De-initializes object of class NodeCache. */
    ~NodeCache();

    /**
     * Returns index file ID.
     * @return Index file ID.
     */
    auto getIndexFileId() const noexcept
    {
        return m_indexFileId;
    }

    /** Writes all pending changes to disk. */
    void flush();

protected:
    /**
     * Returns indication if item can be evicted. By default all items can be evicted.
     * @param key A key.
     * @param value A value.
     * @return true if item can be evicted, false otherwise.
     */
    bool can_evict(const key_type& key, const mapped_type& value) const noexcept override;

    /**
     * Called before item gets evicted from the cache.
     * @param key A key.
     * @param value A value.
     * @param clearingCache Indication of that cache is being completely cleared.
     */
    void on_evict(const key_type& key, mapped_type& value, bool clearingCache) const override;

    /**
     * Called if evict() could not find any discardable element
     * to give last chance for making some additional cleanup.
     * By default no additional action taken.
     * @return true if some changes were made so that next eviction attempt
     *         must be performed, false otherwise.
     */
    bool on_last_chance_cleanup() override;

private:
    /**
     * Saves node to disk.
     * @param mapElement Map element containing node.
     */
    void saveNode(const map_type::value_type& mapElement) const;

private:
    /** Owner object */
    const FileData& m_owner;

    /** Index file ID */
    const std::uint64_t m_indexFileId;
};

}  // namespace siodb::iomgr::dbengine::uli
