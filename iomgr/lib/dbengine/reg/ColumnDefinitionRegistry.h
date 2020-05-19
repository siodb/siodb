// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDefinitionRecord.h"

// Boost header
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace siodb::iomgr::dbengine {

/** In-memory column definition registry */
class ColumnDefinitionRegistry {
public:
    /** Registry record type */
    using value_type = ColumnDefinitionRecord;

private:
    /** Index tag */
    struct ByIdTag {
    };

    /** Index tag */
    struct ByColumnIdAndIdTag {
    };

    /** Key extractor for the column ID and column definition ID pair*/
    struct ExtractColumnIdAndIdPair {
        typedef std::pair<decltype(value_type::m_columnId), decltype(value_type::m_id)> result_type;
        result_type operator()(const value_type& r) const noexcept
        {
            return std::make_pair(r.m_columnId, r.m_id);
        }
    };

    /** Registry data container type */
    typedef boost::multi_index::multi_index_container<value_type,
            boost::multi_index::indexed_by<
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByIdTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_id),
                                    &value_type::m_id>>,
                    boost::multi_index::ordered_unique<boost::multi_index::tag<ByColumnIdAndIdTag>,
                            ExtractColumnIdAndIdPair>>>
            Container;

public:
    /**
     * Returns mutable index by column definition ID.
     * @return Registry index object.
     */
    auto& byId() noexcept
    {
        return m_container.get<ByIdTag>();
    }

    /**
     * Returns read-only index by column definition ID.
     * @return Registry index object.
     */
    const auto& byId() const noexcept
    {
        return m_container.get<ByIdTag>();
    }

    /**
     * Returns read-only index by column ID.
     * @return Registry index object.
     */
    const auto& byColumnIdAndId() const noexcept
    {
        return m_container.get<ByColumnIdAndIdTag>();
    }

    /**
     * Returns indication that registry is empty.
     * @return true if registry is empty, false otherwise.
     */
    bool empty() const noexcept
    {
        return m_container.empty();
    }

    /**
     * Returns number of records in the registry.
     * @return Number of records in the registry.
     */
    std::size_t size() const noexcept
    {
        return m_container.size();
    }

    /**
     * Inserts column definition record to the registry.
     * @param args Column definitio record construction arguments.
     */
    template<typename... Args>
    void emplace(Args&&... args)
    {
        m_container.emplace(std::forward<Args>(args)...);
    }

    /**
     * Inserts column definition record to registry.
     * @param record Column definition record.
     */
    void insert(const value_type& record)
    {
        m_container.insert(record);
    }

    /**
     * Inserts column definition record to registry.
     * @param record Column definition record.
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
    void swap(ColumnDefinitionRegistry& other) noexcept
    {
        if (&other != this) m_container.swap(other.m_container);
    }

private:
    /** Data container */
    Container m_container;
};

}  // namespace siodb::iomgr::dbengine
