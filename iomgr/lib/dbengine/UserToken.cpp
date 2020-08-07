// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserToken.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

namespace siodb::iomgr::dbengine {

UserToken::UserToken(User& user, std::uint64_t id, std::string&& name, BinaryValue&& value,
        std::optional<std::time_t>&& expirationTimestamp, std::optional<std::string>&& description)
    : m_user(user)
    , m_id(id)
    , m_name(validateName(std::move(name)))
    , m_value(std::move(value))
    , m_expirationTimestamp(std::move(expirationTimestamp))
    , m_description(std::move(description))
{
}

UserToken::UserToken(User& user, const UserTokenRecord& tokenRecord)
    : m_user(validateUser(user, tokenRecord))
    , m_id(tokenRecord.m_id)
    , m_name(validateName(std::string(tokenRecord.m_name)))
    , m_value(tokenRecord.m_value)
    , m_expirationTimestamp(tokenRecord.m_expirationTimestamp)
    , m_description(tokenRecord.m_description)
{
}

// ----- internals -----

User& UserToken::validateUser(User& user, const UserTokenRecord& tokenRecord)
{
    if (user.getId() == tokenRecord.m_userId) return user;
    throwDatabaseError(
            IOManagerMessageId::kErrorInvalidUserForUserToken, user.getId(), tokenRecord.m_userId);
}

std::string&& UserToken::validateName(std::string&& accessKeyName)
{
    if (isValidDatabaseObjectName(accessKeyName)) return std::move(accessKeyName);
    throwDatabaseError(IOManagerMessageId::kErrorInvalidUserTokenName, accessKeyName);
}

}  // namespace siodb::iomgr::dbengine
