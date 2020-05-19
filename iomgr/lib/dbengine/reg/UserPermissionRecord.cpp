// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserPermissionRecord.h"

// Project headers
#include "Helpers.h"
#include "../UserPermission.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

namespace siodb::iomgr::dbengine {

UserPermissionRecord::UserPermissionRecord(const UserPermission& userPermission)
    : m_id(userPermission.getId())
    , m_userId(userPermission.getUserId())
    , m_databaseId(userPermission.getDatabaseId())
    , m_objectType(userPermission.getObjectType())
    , m_objectId(userPermission.getObjectId())
    , m_permissions(userPermission.getPermissions())
    , m_grantOptions(userPermission.getGrantOptions())
{
}

std::size_t UserPermissionRecord::getSerializedSize() const noexcept
{
    return ::getVarIntSize(m_id) + ::getVarIntSize(m_userId) + ::getVarIntSize(m_databaseId)
           + ::getVarIntSize(static_cast<std::uint32_t>(m_objectType)) + ::getVarIntSize(m_objectId)
           + ::getVarIntSize(m_permissions) + ::getVarIntSize(m_grantOptions);
}

std::uint8_t* UserPermissionRecord::serializeUnchecked(std::uint8_t* buffer) const noexcept
{
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_userId, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_objectType), buffer);
    buffer = ::encodeVarInt(m_objectId, buffer);
    buffer = ::encodeVarInt(m_permissions, buffer);
    buffer = ::encodeVarInt(m_grantOptions, buffer);
    return buffer;
}

std::size_t UserPermissionRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    std::size_t totalConsumed = 0;

    int consumed = ::decodeVarInt(buffer, length, m_id);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "id", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_userId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "userId", consumed);
    totalConsumed += consumed;

    std::uint32_t uv32 = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, uv32);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "objectType", consumed);
    totalConsumed += consumed;
    m_objectType = static_cast<DatabaseObjectType>(uv32);

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_objectId);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "objectId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_permissions);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "permissions", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_grantOptions);
    if (consumed < 1) helpers::reportDeserializationFailure(kClassName, "grantOptions", consumed);
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
