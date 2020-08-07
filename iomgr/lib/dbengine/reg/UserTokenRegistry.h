// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "UserTokenRecord.h"

// Common project headers
#include <siodb/common/boost_ext/buffer_hash.hpp>

// Boost header
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index_container.hpp>

namespace siodb::iomgr::dbengine {

/** In-memory user access key registry */
class UserTokenRegistry {
public:
    /** Registry record type */
    using value_type = UserTokenRecord;

private:
    /** Index tag */
    struct ByIdTag {
    };

    /** Index tag */
    struct ByNameTag {
    };

    /** Index tag */
    struct ByValueTag {
    };

    /** In-memory table registry container type */
    typedef boost::multi_index::multi_index_container<value_type,
            boost::multi_index::indexed_by<
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByIdTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_id),
                                    &value_type::m_id>>,
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByNameTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_name),
                                    &value_type::m_name>>,
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByValueTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_value),
                                    &value_type::m_value>>>>
            Container;

public:
    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const UserTokenRegistry& other) const noexcept
    {
        return m_container == other.m_container;
    }

    /**
     * Returns read-only index by user access key ID.
     * @return Registry index object.
     */
    const auto& byId() const noexcept
    {
        return m_container.get<ByIdTag>();
    }

    /**
     * Returns read-only index by key name.
     * @return Registry index object.
     */
    const auto& byName() const noexcept
    {
        return m_container.get<ByNameTag>();
    }

    /**
     * Returns mutable index by key name.
     * @return Registry index object.
     */
    auto& byName() noexcept
    {
        return m_container.get<ByNameTag>();
    }

    /**
     * Returns read-only index by token vakue.
     * @return Registry index object.
     */
    const auto& byValue() const noexcept
    {
        return m_container.get<ByValueTag>();
    }

    /**
     * Returns indication that registry is empty.
     * @return true is registry is empty, false otherwise.
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
     * Inserts user access key record to registry.
     * @param args An access key construction parameters.
     */
    template<typename... Args>
    void emplace(Args&&... args)
    {
        m_container.emplace(std::forward<Args>(args)...);
    }

    /**
     * Inserts user record to registry.
     * @param record User access key record.
     */
    void insert(const value_type& record)
    {
        m_container.insert(record);
    }

    /**
     * Inserts user record to registry.
     * @param record User access key record.
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
     * Swap contents of two registries
     * @param other Other registry.
     */
    void swap(UserTokenRegistry& other) noexcept
    {
        if (SIODB_LIKELY(&other != this)) m_container.swap(other.m_container);
    }

private:
    /** Data container */
    Container m_container;
};

}  // namespace siodb::iomgr::dbengine
