// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

// Project headers
#include "../DatabaseObjectType.h"
#include "../PermissionType.h"

namespace siodb::iomgr::dbengine {

class UserPermission;

/** User permission registry record. */
struct UserPermissionRecord {
    /** Initializes object of class UserPermissionRecord. */
    UserPermissionRecord()
        : m_id(0)
        , m_userId(0)
        , m_databaseId(0)
        , m_objectType(static_cast<DatabaseObjectType>(0))
        , m_objectId(0)
        , m_permissions(0)
        , m_grantOptions(0)
    {
    }

    /**
     * Initializes object of class UserPermissionRecord.
     * @param id User permission record ID.
     * @param userId User ID.
     * @param databaseId Database ID.
     * @param objectType Database object type.
     * @param objectId Object ID.
     * @param permissions Granted permissions.
     * @param grantOptions Grant options for the granted permissions.
     */
    UserPermissionRecord(std::uint64_t id, std::uint32_t userId, std::uint32_t databaseId,
            DatabaseObjectType objectType, std::uint64_t objectId, std::uint64_t permissions,
            std::uint64_t grantOptions)
        : m_id(id)
        , m_userId(userId)
        , m_databaseId(databaseId)
        , m_objectType(objectType)
        , m_objectId(objectId)
        , m_permissions(permissions)
        , m_grantOptions(grantOptions)
    {
    }

    /**
     * Initializes object of class UserPermissionRecord.
     * @param userPermission User permission object.
     */
    explicit UserPermissionRecord(const UserPermission& userPermission);

    /**
     * Returns buffer size required to serialize this object.
     * @return Number of bytes.
     */
    std::size_t getSerializedSize() const noexcept;

    /**
     * Serializes object into buffer. Assumes buffer is big enough.
     * @param buffer Output buffer.
     * @return Address of byte after last written byte.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer) const noexcept;

    /**
     * Deserializes object from buffer.
     * @param buffer Input buffer.
     * @param length Length of data in the buffer.
     * @return Number of bytes consumed.
     */
    std::size_t deserialize(const std::uint8_t* buffer, std::size_t length);

    /** Permission record ID */
    std::uint64_t m_id;

    /** User ID */
    std::uint32_t m_userId;

    /** Database ID */
    std::uint32_t m_databaseId;

    /** Database object type */
    DatabaseObjectType m_objectType;

    /** Object ID */
    std::uint64_t m_objectId;

    /** Permissions */
    std::uint64_t m_permissions;

    /** Grant options */
    std::uint64_t m_grantOptions;

    /** Structure name */
    static constexpr const char* kClassName = "UserPermissionRecord";
};

}  // namespace siodb::iomgr::dbengine
