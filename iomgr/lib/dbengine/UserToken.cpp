// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserToken.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

// OpenSSL
#include <openssl/sha.h>

namespace siodb::iomgr::dbengine {

UserToken::UserToken(User& user, std::uint64_t id, std::string&& name, BinaryValue&& value,
        std::optional<std::time_t>&& expirationTimestamp, std::optional<std::string>&& description)
    : m_user(user)
    , m_id(id)
    , m_name(validateName(std::move(name)))
    , m_value(validateValue(std::move(value)))
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

bool UserToken::checkValue(const BinaryValue& value, bool allowExpiredToken) const noexcept
{
    if (!allowExpiredToken && isExpired()) return false;
    uint8_t hash[kHashSize];
    hashValue(value, m_value.data(), hash);
    return std::memcmp(hash, m_value.data() + UserToken::kSaltSize, UserToken::kHashSize) == 0;
}

void UserToken::hashValue(const BinaryValue& value, const uint8_t* salt, uint8_t* hash) noexcept
{
    ::SHA512_CTX ctx;
    ::SHA512_Init(&ctx);
    ::SHA512_Update(&ctx, salt, kSaltSize);
    ::SHA512_Update(&ctx, value.data(), value.size());
    ::SHA512_Final(hash, &ctx);
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

BinaryValue&& UserToken::validateValue(BinaryValue&& tokenValue) const
{
    if (tokenValue.size() == kHashSize + kSaltSize) return std::move(tokenValue);
    throwDatabaseError(IOManagerMessageId::kErrorInvalidUserTokenHashedValue, m_name);
}

}  // namespace siodb::iomgr::dbengine
