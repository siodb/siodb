// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "UserPermissionRecord.h"

// Common project headers
#include <siodb/common/crt_ext/compiler_defs.h>

// Boost header
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index_container.hpp>

namespace siodb::iomgr::dbengine {

/** In-memory user permission registry */
class UserPermissionRegistry {
public:
    /** Registry record type */
    using value_type = UserPermissionRecord;

    /** Key for the index by object. */
    struct ByObjectKey {
        /** Initializes object of the class ByObjectKey. */
        ByObjectKey() noexcept
            : m_databaseId(0)
            , m_objectType(DatabaseObjectType::kMax)
            , m_objectId(0)
        {
        }

        /**
         * Initializes object of the class ByObjectKey.
         * @param databaseId Database identifier.
         * @param objectType Object type.
         * @param objectId Object identifier.
         */
        ByObjectKey(std::uint32_t databaseId, DatabaseObjectType objectType,
                std::uint64_t objectId) noexcept
            : m_databaseId(databaseId)
            , m_objectType(objectType)
            , m_objectId(objectId)
        {
        }

        /**
         * Equality operator.
         * @param other Other object.
         * @return true if objects are equal, false otherwise.
         */
        bool operator==(const ByObjectKey& other) const noexcept
        {
            return m_databaseId == other.m_databaseId && m_objectType == other.m_objectType
                   && m_objectId == other.m_objectId;
        }

        /** Database identifier */
        std::uint32_t m_databaseId;
        /** Object type */
        DatabaseObjectType m_objectType;
        /** Object identifier */
        std::uint64_t m_objectId;
    };

private:
    /** Index tag */
    struct ByIdTag {
    };

    /** Index tag */
    struct ByObjectTag {
    };

    /** Key extractor */
    struct ByObjectKeyExtractor {
        typedef ByObjectKey result_type;
        result_type operator()(const value_type& value) const noexcept
        {
            return result_type {
                    value.m_databaseId,
                    value.m_objectType,
                    value.m_objectId,
            };
        }
    };

    /** In-memory table registry container type */
    typedef boost::multi_index::multi_index_container<value_type,
            boost::multi_index::indexed_by<
                    boost::multi_index::hashed_unique<boost::multi_index::tag<ByIdTag>,
                            boost::multi_index::member<value_type, decltype(value_type::m_id),
                                    &value_type::m_id>>,
                    boost::multi_index::hashed_non_unique<boost::multi_index::tag<ByObjectTag>,
                            ByObjectKeyExtractor>
                    // add  more here
                    >>
            Container;

public:
    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const UserPermissionRegistry& other) const noexcept
    {
        return m_container == other.m_container;
    }

    /**
     * Returns read-only index by record ID.
     * @return Registry index object.
     */
    const auto& byId() const noexcept
    {
        return m_container.get<ByIdTag>();
    }

    /**
     * Returns mutable index by record ID.
     * @return Registry index object.
     */
    auto& byId() noexcept
    {
        return m_container.get<ByIdTag>();
    }

    /**
     * Returns read-only index by object.
     * @return Registry index object.
     */
    const auto& byObject() const noexcept
    {
        return m_container.get<ByObjectTag>();
    }

    /**
     * Returns mutable index by object.
     * @return Registry index object.
     */
    auto& byObject() noexcept
    {
        return m_container.get<ByObjectTag>();
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
     * Inserts user record to registry.
     * @param args User record construction parameters.
     */
    template<typename... Args>
    void emplace(Args&&... args)
    {
        m_container.emplace(std::forward<Args>(args)...);
    }

    /**
     * Inserts user record to registry.
     * @param record User record.
     */
    void insert(const value_type& record)
    {
        m_container.insert(record);
    }

    /**
     * Inserts user record to registry.
     * @param record User record.
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
    void swap(UserPermissionRegistry& other) noexcept
    {
        if (SIODB_LIKELY(&other != this)) m_container.swap(other.m_container);
    }

private:
    /** Data container */
    Container m_container;
};

/**
 * Hash function for the class UserPermissionRegistry::ByObjectKey
 * for the Boost hashed containers.
 * @param value A value to hash.
 * @return Hash value.
 */
inline std::size_t hash_value(const UserPermissionRegistry::ByObjectKey& value) noexcept
{
    std::size_t result = 0;
    boost::hash_combine(result, value.m_databaseId);
    boost::hash_combine(result, value.m_objectType);
    boost::hash_combine(result, value.m_objectId);
    return result;
}

}  // namespace siodb::iomgr::dbengine
