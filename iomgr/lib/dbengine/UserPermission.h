// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "reg/UserPermissionRecord.h"

// Common project headers
#include <siodb/common/stl_ext/functional_ext.h>

// STL headers
#include <ostream>
#include <unordered_map>

namespace siodb::iomgr::dbengine {

/** User permission key */
class UserPermissionKey {
public:
    /**
     * Initializes object of the class UserPermission for the new user permission record.
     * @param databaseId Database ID.
     * @param objectType Object type.
     * @param objectId Object ID.
     */
    UserPermissionKey(std::uint32_t databaseId, DatabaseObjectType objectType,
            std::uint64_t objectId) noexcept
        : m_databaseId(databaseId)
        , m_objectType(objectType)
        , m_objectId(objectId)
    {
    }

    /**
     * Initializes object of the class UserPermission for an existing user permission record.
     * @param userPermissionRecord User permissions record.
     */
    explicit UserPermissionKey(const UserPermissionRecord& userPermissionRecord) noexcept;

    /**
     * Returns database ID.
     * @return Database ID.
     */
    auto getDatabaseId() const noexcept
    {
        return m_databaseId;
    }

    /**
     * Returns database object type.
     * @return Database object type.
     */
    auto getObjectType() const noexcept
    {
        return m_objectType;
    }

    /**
     * Returns database object ID.
     * @return Database object ID.
     */
    auto getObjectId() const noexcept
    {
        return m_objectId;
    }

    /**
     * Equality operator.
     * @param other Other object.
     * @return true if this object is equal to other, false otherwise.
     */
    bool operator==(const UserPermissionKey& other) const noexcept
    {
        return m_databaseId == other.m_databaseId && m_objectType == other.m_objectType
               && m_objectId == other.m_objectId;
    }

private:
    /** Database ID */
    std::uint32_t m_databaseId;

    /** Object type */
    DatabaseObjectType m_objectType;

    /** Object ID */
    std::uint64_t m_objectId;
};

/**
 * Outputs UserPermissionKey to the standard output stream.
 * @param os Output stream.
 * @param permissionKey Permission key object.
 * @return The same output stream.
 */
std::ostream& operator<<(std::ostream& os, const UserPermissionKey& permissionKey);

/**
 * Function object which produces hash value for the UserPermissionKey object
 * according to composite object ID.
 */
struct UserPermissionKeyHash {
    /**
     * Produces hash value for the UserPermission object according to composite object ID.
     * @param value Object to compute hash for.
     * @return Hash value.
     */
    std::size_t operator()(const UserPermissionKey& value) const noexcept
    {
        return stdext::hash_val(value.getDatabaseId(), value.getObjectType(), value.getObjectId());
    }
};

/** User permission data */
class UserPermissionData {
public:
    /**
     * Initializes object of the class UserPermissionData for the new user permission record.
     * @param permissions Permissions.
     * @param grantOptions Grant options for permissions.
     */
    UserPermissionData(std::uint64_t permissions, std::uint64_t grantOptions) noexcept
        : m_permissions(permissions)
        , m_grantOptions(grantOptions)
    {
    }

    /**
     * Initializes object of the class UserPermission for an existing user permission record.
     * @param userPermissionRecord User permissions record.
     */
    explicit UserPermissionData(const UserPermissionRecord& userPermissionRecord) noexcept;

    /**
     * Returns granted permissions.
     * @return Granted permissions bitmask.
     */
    auto getPermissions() const noexcept
    {
        return m_permissions;
    }

    /**
     * Adds more permissions.
     * @param permission Permission bitmask.
     * @param withGrantOption GRANT OPTION flag.
     */
    void addPermissions(std::uint64_t permissions, bool withGrantOption) noexcept
    {
        m_permissions |= permissions;
        if (withGrantOption) m_grantOptions |= permissions;
    }

    /**
     * Removes permissions.
     * @param permission Permission bitmask.
     */
    void removePermissions(std::uint64_t permissions) noexcept
    {
        m_permissions &= ~permissions;
        m_grantOptions &= ~permissions;
    }

    /**
     * Returns raw grant options for the granted permissions.
     * @return Grant options bitmask.
     */
    auto getRawGrantOptions() const noexcept
    {
        return m_grantOptions;
    }

    /**
     * Returns effective grant options for the granted permissions.
     * @return Grant options bitmask.
     */
    auto getEffectiveGrantOptions() const noexcept
    {
        return m_grantOptions & m_permissions;
    }

private:
    /** Permissions */
    std::uint64_t m_permissions;

    /** Grant options */
    std::uint64_t m_grantOptions;
};

/** Extended user permission data */
class UserPermissionDataEx : public UserPermissionData {
public:
    /**
     * Initializes object of the class UserPermissionDataEx with default values.
     */
    UserPermissionDataEx() noexcept
        : UserPermissionData(0, 0)
        , m_id(0)
    {
    }

    /**
     * Initializes object of the class UserPermissionDataEx for the new user permission record.
     * @param id Permission record ID.
     * @param permissions Permissions.
     * @param grantOptions Grant options for permissions.
     */
    explicit UserPermissionDataEx(const UserPermissionData& src, std::uint64_t id = 0) noexcept
        : UserPermissionData(src)
        , m_id(id)
    {
    }

    /**
     * Initializes object of the class UserPermissionDataEx from existing
     * user permission data object of the class UserPermissionData.
     * @param src Source object.
     * @param id Permission record ID.
     */
    UserPermissionDataEx(
            std::uint64_t id, std::uint64_t permissions, std::uint64_t grantOptions) noexcept
        : UserPermissionData(permissions, grantOptions)
        , m_id(id)
    {
    }

    /**
     * Initializes object of the class UserPermissionDataEx for an existing user permission record.
     * @param userPermissionRecord User permissions record.
     */
    explicit UserPermissionDataEx(const UserPermissionRecord& userPermissionRecord) noexcept;

    /**
     * Returns permission record ID.
     * @return Permission record ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Sets permission record ID.
     * @param id Permission record ID.
     */
    void setId(std::uint64_t id) noexcept
    {
        m_id = id;
    }

private:
    /** Permission record ID */
    std::uint64_t m_id;
};

/** Map of user permissions with normal data */
using UserPermissionMap =
        std::unordered_map<UserPermissionKey, UserPermissionData, UserPermissionKeyHash>;

/** Map of user permissions with extended data */
using UserPermissionMapEx =
        std::unordered_map<UserPermissionKey, UserPermissionDataEx, UserPermissionKeyHash>;

}  // namespace siodb::iomgr::dbengine
