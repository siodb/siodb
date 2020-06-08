// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/Uuid.h>

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
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const UserPermissionRecord& other) const noexcept
    {
        return m_id == other.m_id && m_userId == other.m_userId
               && m_databaseId == other.m_databaseId && m_objectType == other.m_objectType
               && m_objectId == other.m_objectId && m_permissions == other.m_permissions
               && m_grantOptions == other.m_grantOptions;
    }

    /**
     * Returns buffer size required to serialize this object.
     * @param version Target version.
     * @return Number of bytes.
     */
    std::size_t getSerializedSize(unsigned version = kClassVersion) const noexcept;

    /**
     * Serializes object into buffer. Assumes buffer is big enough.
     * @param buffer Output buffer.
     * @param version Target version.
     * @return Address of byte after last written byte.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer, unsigned version = kClassVersion) const
            noexcept;

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

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "UserPermissionRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
