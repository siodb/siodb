// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserPermissionRecord.h"

// Project headers
#include "Helpers.h"
#include "../UserPermission.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

// Boost headers
#include <boost/lexical_cast.hpp>

namespace siodb::iomgr::dbengine {

const Uuid UserPermissionRecord::s_classUuid =
        boost::lexical_cast<Uuid>("560ff756-a68d-4e8b-a3b9-213e4e80f808");

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

std::size_t UserPermissionRecord::getSerializedSize(unsigned version) const noexcept
{
    return Uuid::static_size() + ::getVarIntSize(version) + ::getVarIntSize(m_id)
           + ::getVarIntSize(m_userId) + ::getVarIntSize(m_databaseId)
           + ::getVarIntSize(static_cast<std::uint32_t>(m_objectType)) + ::getVarIntSize(m_objectId)
           + ::getVarIntSize(m_permissions) + ::getVarIntSize(m_grantOptions);
}

std::uint8_t* UserPermissionRecord::serializeUnchecked(std::uint8_t* buffer, unsigned version) const
        noexcept
{
    std::memcpy(buffer, s_classUuid.data, Uuid::static_size());
    buffer += Uuid::static_size();
    buffer = ::encodeVarInt(version, buffer);
    buffer = ::encodeVarInt(m_id, buffer);
    buffer = ::encodeVarInt(m_userId, buffer);
    buffer = ::encodeVarInt(m_databaseId, buffer);
    buffer = ::encodeVarInt(static_cast<std::uint32_t>(m_objectType), buffer);
    buffer = ::encodeVarInt(m_objectId, buffer);
    buffer = ::encodeVarInt(m_permissions, buffer);
    buffer = ::encodeVarInt(m_grantOptions, buffer);
    return buffer;
}

std::size_t UserPermissionRecord::deserialize(const std::uint8_t* buffer, std::size_t length)
{
    if (length < Uuid::static_size())
        helpers::reportInvalidOrNotEnoughData(kClassName, "$classUuid", 0);
    if (std::memcmp(s_classUuid.data, buffer, Uuid::static_size()) != 0)
        helpers::reportClassUuidMismatch(kClassName, buffer, s_classUuid.data);

    std::size_t totalConsumed = Uuid::static_size();

    std::uint32_t classVersion = 0;
    int consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, classVersion);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "$classVersion", consumed);
    totalConsumed += consumed;

    if (classVersion > kClassVersion)
        helpers::reportClassVersionMismatch(kClassName, classVersion, kClassVersion);

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_id);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "id", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_userId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "userId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_databaseId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "databaseId", consumed);
    totalConsumed += consumed;

    std::uint32_t objectType = 0;
    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, objectType);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "objectType", consumed);
    totalConsumed += consumed;
    m_objectType = static_cast<DatabaseObjectType>(objectType);

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_objectId);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "objectId", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_permissions);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "permissions", consumed);
    totalConsumed += consumed;

    consumed = ::decodeVarInt(buffer + totalConsumed, length - totalConsumed, m_grantOptions);
    if (consumed < 1) helpers::reportInvalidOrNotEnoughData(kClassName, "grantOptions", consumed);
    totalConsumed += consumed;

    return totalConsumed;
}

}  // namespace siodb::iomgr::dbengine
