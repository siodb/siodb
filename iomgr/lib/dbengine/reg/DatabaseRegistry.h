// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "DatabaseRecord.h"

// Common project headers
#include <siodb/common/config/CompilerDefs.h>

// Boost headers
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index_container.hpp>

namespace siodb::iomgr::dbengine {

/** In-memory database registry */
class DatabaseRegistry {
public:
    /** Registry record type */
    using value_type = DatabaseRecord;

private:
    /** Index tag */
    struct ByIdTag {
    };

    /** Index tag */
    struct ByUuidTag {
    };

    /** Index tag */
    struct ByNameTag {
    };

    /** Registry data container type */
    typedef boost::multi_index::multi_index_container<value_type,
            boost::multi_index::indexed_by<
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByIdTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_id),
                                    &value_type::m_id>>,
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByUuidTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_uuid),
                                    &value_type::m_uuid>>,
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByNameTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_name),
                                    &value_type::m_name>>>>
            Container;

public:
    /**
     * Returns mutable index by database ID.
     * @return Registry index object.
     */
    auto& byId() noexcept
    {
        return m_container.get<ByIdTag>();
    }

    /**
     * Returns read-only index by database ID.
     * @return Registry index object.
     */
    const auto& byId() const noexcept
    {
        return m_container.get<ByIdTag>();
    }

    /**
     * Returns read-only index by database UUID.
     * @return Registry index object.
     */
    const auto& byUuid() const noexcept
    {
        return m_container.get<ByUuidTag>();
    }

    /**
     * Returns read-only index by database name.
     * @return Registry index object.
     */
    const auto& byName() const noexcept
    {
        return m_container.get<ByNameTag>();
    }

    /**
     * Returns indication that the registry is empty.
     * @return true if registry is empty, false otherwise.
     */
    bool empty() const noexcept
    {
        return m_container.empty();
    }

    /**
     * Returns number of records in the map.
     * @return Number of records in the map.
     */
    std::size_t size() const noexcept
    {
        return m_container.size();
    }

    /**
     * Inserts a record into the registry.
     * @param args Database record construction arguments.
     */
    template<typename... Args>
    void emplace(Args&&... args)
    {
        m_container.emplace(std::forward<Args>(args)...);
    }

    /**
     * Adds a record to the registry.
     * @param record Database record.
     */
    void insert(const value_type& record)
    {
        m_container.insert(record);
    }

    /**
     * Adds a record to the registry.
     * @param record Database record.
     */
    void insert(value_type&& record)
    {
        m_container.insert(std::move(record));
    }

    /** Clears registry */
    void clear() noexcept
    {
        m_container.clear();
    }

    /**
     * Swaps content with another registry.
     * @param other Other registry.
     */
    void swap(DatabaseRegistry& other) noexcept
    {
        if (SIODB_LIKELY(&other != this)) m_container.swap(other.m_container);
    }

private:
    /** Data container */
    Container m_container;
};

}  // namespace siodb::iomgr::dbengine
