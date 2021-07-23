// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserPermission.h"

namespace siodb::iomgr::dbengine {

///// class UserPermissionKey ////////////////////////////////////////////////

UserPermissionKey::UserPermissionKey(const UserPermissionRecord& userPermissionRecord) noexcept
    : m_databaseId(userPermissionRecord.m_databaseId)
    , m_objectType(userPermissionRecord.m_objectType)
    , m_objectId(userPermissionRecord.m_objectId)
{
}

std::ostream& operator<<(std::ostream& os, const UserPermissionKey& permissionKey)
{
    return os << '[' << permissionKey.getDatabaseId() << ','
              << static_cast<int>(permissionKey.getObjectType()) << ','
              << permissionKey.getObjectId() << ']';
}

///// class UserPermissionData ///////////////////////////////////////////////

UserPermissionData::UserPermissionData(const UserPermissionRecord& userPermissionRecord) noexcept
    : m_permissions(userPermissionRecord.m_permissions)
    , m_grantOptions(userPermissionRecord.m_grantOptions)
{
}

///// class UserPermissionDataEx /////////////////////////////////////////////

UserPermissionDataEx::UserPermissionDataEx(
        const UserPermissionRecord& userPermissionRecord) noexcept
    : UserPermissionData(userPermissionRecord)
    , m_id(userPermissionRecord.m_id)
{
}

}  // namespace siodb::iomgr::dbengine
