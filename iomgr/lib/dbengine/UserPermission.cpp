// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserPermission.h"

namespace siodb::iomgr::dbengine {

UserPermission::UserPermission(std::uint64_t id, std::uint32_t userId, std::uint32_t databaseId,
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

}  // namespace siodb::iomgr::dbengine
