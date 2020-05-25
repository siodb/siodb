// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserAccessKey.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "DatabaseObjectName.h"
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

UserAccessKey::UserAccessKey(User& user, std::uint64_t id, std::string&& name, std::string&& text,
        std::optional<std::string>&& description, bool active)
    : m_user(user)
    , m_id(id)
    , m_name(validateUserAccessKeyName(std::move(name)))
    , m_text(std::move(text))
    , m_description(std::move(description))
    , m_active(active)
{
}

UserAccessKey::UserAccessKey(User& user, const UserAccessKeyRecord& accessKeyRecord)
    : m_user(validateUser(user, accessKeyRecord))
    , m_id(accessKeyRecord.m_id)
    , m_name(validateUserAccessKeyName(std::string(accessKeyRecord.m_name)))
    , m_text(accessKeyRecord.m_text)
    , m_description(accessKeyRecord.m_description)
    , m_active(accessKeyRecord.m_active)
{
}

// ----- internals -----

User& UserAccessKey::validateUser(User& user, const UserAccessKeyRecord& accessKeyRecord)
{
    if (user.getId() == accessKeyRecord.m_userId) return user;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidUserForUserAccessKey, user.getId(),
            accessKeyRecord.m_userId);
}

std::string&& UserAccessKey::validateUserAccessKeyName(std::string&& accessKeyName)
{
    if (isValidDatabaseObjectName(accessKeyName)) return std::move(accessKeyName);
    throwDatabaseError(IOManagerMessageId::kErrorInvalidUserAccessKeyName, accessKeyName);
}

}  // namespace siodb::iomgr::dbengine
