// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ConstraintRecord.h"

// Common project headers
#include <siodb/common/crt_ext/compiler_defs.h>

// Boost header
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index_container.hpp>

namespace siodb::iomgr::dbengine {

/** In-memory global constraint registry */
class ConstraintRegistry {
public:
    /** Registry record type */
    using value_type = ConstraintRecord;

private:
    /** Index tag */
    struct ByIdTag {
    };

    /** Index tag */
    struct ByNameTag {
    };

    /** Index tag */
    struct ByTableIdTag {
    };

    /** Index tag */
    struct ByColumnIdTag {
    };

    /** Registry data container type */
    typedef boost::multi_index::multi_index_container<value_type,
            boost::multi_index::indexed_by<
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByIdTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_id),
                                    &value_type::m_id>>,
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByNameTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_name),
                                    &value_type::m_name>>,
                    boost::multi_index::hashed_non_unique<boost::multi_index::tag<ByTableIdTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_tableId),
                                    &value_type::m_tableId>>,
                    boost::multi_index::hashed_non_unique<boost::multi_index::tag<ByColumnIdTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_columnId),
                                    &value_type::m_columnId>>>>
            Container;

public:
    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const ConstraintRegistry& other) const noexcept
    {
        return m_container == other.m_container;
    }

    /**
     * Returns read-only index by constraint ID.
     * @return Registry index object.
     */
    const auto& byId() const noexcept
    {
        return m_container.get<ByIdTag>();
    }

    /**
     * Returns read-only index by constraint name.
     * @return Registry index object.
     */
    const auto& byName() const noexcept
    {
        return m_container.get<ByNameTag>();
    }

    /**
     * Returns read-only index by table ID.
     * @return Index object.
     */
    const auto& byTableId() const noexcept
    {
        return m_container.get<ByTableIdTag>();
    }

    /*
     * Returns read-only index by column ID.
     * @return Registry index object.
     */
    const auto& byColumnId() const noexcept
    {
        return m_container.get<ByColumnIdTag>();
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
     * @return Number of records in the registry.
     */
    std::size_t size() const noexcept
    {
        return m_container.size();
    }

    /**
     * Inserts a record into the registry.
     * @param args Arguments.
     */
    template<typename... Args>
    void emplace(Args&&... args)
    {
        m_container.emplace(std::forward<Args>(args)...);
    }

    /**
     * Inserts a record into the registry.
     * @param record Index record.
     */
    void insert(const value_type& record)
    {
        m_container.insert(record);
    }

    /**
     * Inserts a record into the registry.
     * @param record Index record.
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
    void swap(ConstraintRegistry& other) noexcept
    {
        if (SIODB_LIKELY(&other != this)) m_container.swap(other.m_container);
    }

private:
    /** Data container */
    Container m_container;
};

}  // namespace siodb::iomgr::dbengine
