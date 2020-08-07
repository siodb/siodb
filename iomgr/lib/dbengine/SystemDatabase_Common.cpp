// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SystemDatabase.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Table.h"
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

bool SystemDatabase::isSystemDatabase() const noexcept
{
    return true;
}

std::uint32_t SystemDatabase::generateNextUserId()
{
    return static_cast<std::uint32_t>(m_sysUsersTable->generateNextUserTrid());
}

std::uint64_t SystemDatabase::generateNextUserAccessKeyId()
{
    return m_sysUserAccessKeysTable->generateNextUserTrid();
}

std::uint64_t SystemDatabase::generateNextUserTokenId()
{
    return m_sysUserTokensTable->generateNextUserTrid();
}

std::uint32_t SystemDatabase::generateNextDatabaseId(bool system)
{
    const auto databaseId =
            system ? (m_sysDatabasesTable ? m_sysDatabasesTable->generateNextSystemTrid()
                                          : kSystemDatabaseId)
                   : m_sysDatabasesTable->generateNextUserTrid();
    if (databaseId >= std::numeric_limits<std::uint32_t>::max())
        throwDatabaseError(IOManagerMessageId::kErrorInstanceResourceExhausted, "Database ID");
    return static_cast<std::uint32_t>(databaseId);
}

std::uint64_t SystemDatabase::generateNextUserPermissionId()
{
    return m_sysUserPermissionsTable->generateNextUserTrid();
}

}  // namespace siodb::iomgr::dbengine
