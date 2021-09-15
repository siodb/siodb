// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "TableColumn.h"

// Common project headers
#include <siodb/common/crt_ext/compiler_defs.h>

// Boost header
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace siodb::iomgr::dbengine {

/** In-memory column registry */
class TableColumns {
public:
    /** Registry record type */
    using value_type = TableColumn;

private:
    /** Index tag */
    struct ByColumnSetColumnIdTag {
    };

    /** Index tag */
    struct ByColumnIdTag {
    };

    /** Index tag */
    struct ByPositionTag {
    };

    /** Index tag */
    struct ByNameTag {
    };

    /** Key extractor for column ID */
    struct ExtractColumnId {
        typedef std::uint64_t result_type;
        result_type operator()(const value_type& value) const noexcept;
    };

    /** Key extractor for column name */
    struct ExtractColumnName {
        typedef std::string result_type;
        const result_type& operator()(const value_type& value) const noexcept;
    };

    /** Registry data container type */
    typedef boost::multi_index::multi_index_container<value_type,
            boost::multi_index::indexed_by<
                    boost::multi_index::hashed_unique<
                            boost::multi_index::tag<ByColumnSetColumnIdTag>,
                            boost::multi_index::member<value_type,
                                    decltype(value_type::m_columnSetColumnId),
                                    &value_type::m_columnSetColumnId>>,
                    // must be ordered_unique!!!
                    boost::multi_index::ordered_unique<boost::multi_index::tag<ByPositionTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_position),
                                    &value_type::m_position>>,
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByColumnIdTag>,
                            ExtractColumnId>,
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByNameTag>,
                            ExtractColumnName>>>
            Container;

public:
    /**
     * Returns read-only index by column set column ID.
     * @return Index object
     */
    const auto& byColumnSetColumnId() const noexcept
    {
        return m_container.get<ByColumnSetColumnIdTag>();
    }

    /*
     * Returns read-only index by column ID.
     * @return Index object.
     */
    const auto& byColumnId() const noexcept
    {
        return m_container.get<ByColumnIdTag>();
    }

    /**
     * Returns read-only index by column position.
     * @return Index object.
     */
    const auto& byPosition() const noexcept
    {
        return m_container.get<ByPositionTag>();
    }

    /**
     * Returns read-only index by column name.
     * @return Index object.
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
     * Returns number of records in the registry.
     * @return number of records in the registry.
     */
    std::size_t size() const noexcept
    {
        return m_container.size();
    }

    /**
     * Inserts a record into the registry.
     * @param args Table column record construction rguments.
     */
    template<typename... Args>
    void emplace(Args&&... args)
    {
        m_container.emplace(std::forward<Args>(args)...);
    }

    /**
     * Inserts a record into the registry.
     * @param record Table column record.
     */
    void insert(const value_type& record)
    {
        m_container.insert(record);
    }

    /**
     * Inserts a record into the registry.
     * @param record Table column record.
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
    void swap(TableColumns& other) noexcept
    {
        if (SIODB_LIKELY(&other != this)) m_container.swap(other.m_container);
    }

private:
    /** Data container */
    Container m_container;
};

}  // namespace siodb::iomgr::dbengine
