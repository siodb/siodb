// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnRecord.h"

// Common project headers
#include <siodb/common/config/CompilerDefs.h>

// Boost headers
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace siodb::iomgr::dbengine {

/** In-memory global column registry */
class ColumnRegistry {
public:
    /** Registry record type */
    using value_type = ColumnRecord;

private:
    /** Index tag */
    struct ByIdTag {
    };

    /** Index tag */
    struct ByTableIdAndNameTag {
    };

    /** Key extractor for column name */
    struct ExtractColumnName {
        typedef decltype(value_type::m_name) result_type;
        const result_type& operator()(const value_type& r) const noexcept
        {
            return r.m_name;
        }
    };

    /** Registry data container type */
    typedef boost::multi_index::multi_index_container<value_type,
            boost::multi_index::indexed_by<
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByIdTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_id),
                                    &value_type::m_id>>,
                    boost::multi_index::ordered_unique<boost::multi_index::tag<ByTableIdAndNameTag>,
                            boost::multi_index::composite_key<value_type,
                                    boost::multi_index::member<value_type,
                                            decltype(value_type::m_tableId),
                                            &value_type::m_tableId>,
                                    ExtractColumnName>>>>
            Container;

public:
    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const ColumnRegistry& other) const noexcept
    {
        return m_container == other.m_container;
    }

    /**
     * Returns read-only index by column ID.
     * @return Index object.
     */
    const auto& byId() const noexcept
    {
        return m_container.get<ByIdTag>();
    }

    /**
     * Returns mutable index by table ID and name.
     * @return Index object.
     */
    auto& byTableIdAndName() noexcept
    {
        return m_container.get<ByTableIdAndNameTag>();
    }

    /**
     * Returns read-only index by table ID and name.
     * @return Index object.
     */
    const auto& byTableIdAndName() const noexcept
    {
        return m_container.get<ByTableIdAndNameTag>();
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
     * Returns number of records in the registry.
     * @return number of records in the registry.
     */
    std::size_t size() const noexcept
    {
        return m_container.size();
    }

    /**
     * Inserts a record into the registry.
     * @param args Column record construction arguments.
     */
    template<typename... Args>
    void emplace(Args&&... args)
    {
        m_container.emplace(std::forward<Args>(args)...);
    }

    /**
     * Inserts a record into the registry.
     * @param record Column record.
     */
    void insert(const value_type& column)
    {
        m_container.insert(column);
    }

    /**
     * Inserts a record into the registry.
     * @param record Column record.
     */
    void insert(value_type&& column)
    {
        m_container.insert(std::move(column));
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
    void swap(ColumnRegistry& other) noexcept
    {
        if (SIODB_LIKELY(&other != this)) m_container.swap(other.m_container);
    }

private:
    /** Data container */
    Container m_container;
};

}  // namespace siodb::iomgr::dbengine
