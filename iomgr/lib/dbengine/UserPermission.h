// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "User.h"
#include "reg/UserPermissionRecord.h"

namespace siodb::iomgr::dbengine {

/** User permission */
class UserPermission {
public:
    /**
     * Initializes object of class UserPermission for the new user permission record.
     * @param id Permission record ID.
     * @param userId User ID.
     * @param databaseId Database ID.
     * @param objectType Object type.
     * @param objectId Object ID.
     * @param permissions Permissions.
     * @param grantOptions Grant options for permissions.
     */
    UserPermission(std::uint64_t id, std::uint32_t userId, std::uint32_t databaseId,
            DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
            std::uint64_t grantOptions);

    /**
     * Initializes object of class UserPermission for an existing user permission record.
     * @param userPermissionRecord User permissions record.
     */
    explicit UserPermission(const UserPermissionRecord& userPermissionRecord);

    /**
     * Returns permission record ID.
     * @return Permission record ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns user ID.
     * @return User ID.
     */
    auto getUserId() const noexcept
    {
        return m_userId;
    }

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
     * Returns granted permissions.
     * @return Granted permissions bitmask.
     */
    auto getPermissions() const noexcept
    {
        return m_permissions;
    }

    /**
     * Returns grant options for the granted permissions.
     * @return Grant options bitmask.
     */
    auto getGrantOptions() const noexcept
    {
        return m_grantOptions;
    }

private:
    /** Permission record ID */
    const std::uint64_t m_id;

    /** User ID */
    const std::uint32_t m_userId;

    /** Database ID */
    const std::uint32_t m_databaseId;

    /** Database object type */
    const DatabaseObjectType m_objectType;

    /** Object ID */
    const std::uint64_t m_objectId;

    /** Permissions */
    std::uint64_t m_permissions;

    /** Grant options */
    std::uint64_t m_grantOptions;
};

}  // namespace siodb::iomgr::dbengine
