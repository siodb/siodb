// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "User.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"
#include "UserAccessKey.h"
#include "UserToken.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/crypto/DigitalSignatureKey.h>
#include <siodb/common/utils/RandomUtils.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

// Boost headers
#include <boost/algorithm/hex.hpp>

namespace siodb::iomgr::dbengine {

User::User(UserIdGenerator& userIdGenerator, std::string&& name,
        std::optional<std::string>&& realName, std::optional<std::string>&& description,
        bool active)
    : m_name(validateUserName(std::move(name)))
    , m_realName(std::move(realName))
    , m_description(std::move(description))
    , m_active(active)
    , m_id(userIdGenerator.generateNextUserId())
{
}

User::User(const UserRecord& userRecord)
    : m_name(validateUserName(std::string(userRecord.m_name)))
    , m_realName(userRecord.m_realName)
    , m_description(userRecord.m_description)
    , m_active(userRecord.m_active)
    , m_id(userRecord.m_id)
{
    for (const auto& accessKeyRecord : userRecord.m_accessKeys.byId())
        m_accessKeys.push_back(std::make_shared<UserAccessKey>(*this, accessKeyRecord));
}

UserAccessKeyPtr User::findAccessKeyChecked(const std::string& name) const
{
    if (auto userAccessKey = findAccessKeyUnlocked(name)) return userAccessKey;
    throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyDoesNotExist, m_name, name);
}

UserAccessKeyPtr User::addAccessKey(std::uint64_t id, std::string&& name, std::string&& text,
        std::optional<std::string>&& description, bool active)
{
    const auto it = std::find_if(
            m_accessKeys.begin(), m_accessKeys.end(), [id, &name](const auto& accessKey) noexcept {
                return accessKey->getId() == id || accessKey->getName() == name;
            });
    if (it != m_accessKeys.end()) {
        if ((*it)->getId() == id)
            throwDatabaseError(IOManagerMessageId::kErrorDuplicateUserAccessKeyId, m_name, id);
        else
            throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyAlreadyExists, m_name, name);
    }

    if (text.size() > siodb::kMaxUserAccessKeySize) {
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyIsTooLong, m_name, name,
                text.size(), siodb::kMaxUserAccessKeySize);
    }

    try {
        siodb::crypto::DigitalSignatureKey key;
        key.parseFromString(text);
    } catch (std::exception& e) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidUserAccessKey, m_name, name);
    }

    auto accessKey = std::make_shared<UserAccessKey>(
            *this, id, std::move(name), std::move(text), std::move(description), active);
    m_accessKeys.push_back(accessKey);
    return accessKey;
}

void User::deleteAccessKey(const std::string& name)
{
    const auto it = std::find_if(m_accessKeys.begin(), m_accessKeys.end(),
            [&name](const auto& accessKey) noexcept { return accessKey->getName() == name; });
    if (it == m_accessKeys.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyDoesNotExist, m_name, name);

    if (isSuperUser() && getActiveAccessKeyCount() == 1 && (*it)->isActive())
        throwDatabaseError(IOManagerMessageId::kErrorCannotDeleteLastSuperUserAccessKey, name);

    m_accessKeys.erase(it);
}

std::size_t User::getActiveAccessKeyCount() const noexcept
{
    return std::count_if(m_accessKeys.begin(), m_accessKeys.end(),
            [](const auto& key) noexcept { return key->isActive(); });
}

UserTokenPtr User::findTokenChecked(const std::string& name) const
{
    if (auto token = findTokenUnlocked(name)) return token;
    throwDatabaseError(IOManagerMessageId::kErrorUserTokenDoesNotExist, m_name, name);
}

UserTokenPtr User::addToken(std::uint64_t id, std::string&& name, const BinaryValue& value,
        std::optional<std::time_t>&& expirationTimestamp, std::optional<std::string>&& description)
{
    const auto it =
            std::find_if(m_tokens.begin(), m_tokens.end(), [id, &name](const auto& token) noexcept {
                return token->getId() == id || token->getName() == name;
            });

    if (it != m_tokens.end()) {
        if ((*it)->getId() == id)
            throwDatabaseError(IOManagerMessageId::kErrorDuplicateUserTokenId, m_name, id);
        else
            throwDatabaseError(IOManagerMessageId::kErrorUserTokenAlreadyExists, m_name, name);
    }

    if (value.size() == 0 || value.size() < UserToken::kMinSize
            || value.size() > UserToken::kMaxSize)
        throwDatabaseError(IOManagerMessageId::kErrorInvalidUserTokenValue2);

    if (checkToken(value, true)) throwDatabaseError(IOManagerMessageId::kErrorDuplicateUserToken);

    BinaryValue savedValue(UserToken::kSaltSize + UserToken::kHashSize);
    utils::getRandomBytes(savedValue.data(), UserToken::kSaltSize);
    UserToken::hashValue(value, savedValue.data(), savedValue.data() + UserToken::kSaltSize);

    auto token = std::make_shared<UserToken>(*this, id, std::move(name), std::move(savedValue),
            std::move(expirationTimestamp), std::move(description));
    m_tokens.push_back(token);
    return token;
}

void User::deleteToken(const std::string& name)
{
    const auto it = std::find_if(m_tokens.begin(), m_tokens.end(),
            [&name](const auto& token) noexcept { return token->getName() == name; });
    if (it == m_tokens.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserTokenDoesNotExist, m_name, name);
    m_tokens.erase(it);
}

std::size_t User::getActiveTokenCount() const noexcept
{
    return std::count_if(m_tokens.begin(), m_tokens.end(),
            [](const auto& token) noexcept { return !token->isExpired(); });
}

bool User::authenticate(const std::string& signature, const std::string& challenge) const
{
    if (!isActive()) return false;
    const auto itKey =
            std::find_if(m_accessKeys.cbegin(), m_accessKeys.cend(), [&](const auto& accessKey) {
                if (!accessKey->isActive()) return false;
                siodb::crypto::DigitalSignatureKey key;
                try {
                    key.parseFromString(accessKey->getText());
                    return key.verifySignature(challenge, signature);
                } catch (std::exception& e) {
                    return false;
                }
            });
    return itKey != m_accessKeys.cend();
}

bool User::authenticate(const std::string& tokenValue) const
{
    // Validate and parse token value
    if (tokenValue.empty() || tokenValue.length() % 2 != 0
            || tokenValue.length() > UserToken::kMaxSize * 2)
        throwDatabaseError(IOManagerMessageId::kErrorInvalidUserTokenValue1);
    BinaryValue bv(tokenValue.length() / 2);
    try {
        boost::algorithm::unhex(tokenValue.cbegin(), tokenValue.cend(), bv.begin());
    } catch (...) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidUserTokenValue1);
    }

    // Check that user is active
    if (!isActive()) return false;

    // Check token
    return checkToken(bv);
}

bool User::checkToken(const BinaryValue& tokenValue, bool allowExpiredToken) const noexcept
{
    if (!m_tokens.empty()) {
        for (const auto& token : m_tokens) {
            if (token->checkValue(tokenValue, allowExpiredToken)) return true;
        }
    }
    return false;
}

// ----- internals -----

std::string&& User::validateUserName(std::string&& userName)
{
    if (isValidDatabaseObjectName(userName)) return std::move(userName);
    throwDatabaseError(IOManagerMessageId::kErrorInvalidUserName, userName);
}

UserAccessKeyPtr User::findAccessKeyUnlocked(const std::string& name) const noexcept
{
    auto it = std::find_if(m_accessKeys.begin(), m_accessKeys.end(),
            [&name](const auto& accessKey) noexcept { return accessKey->getName() == name; });
    return (it == m_accessKeys.end()) ? nullptr : *it;
}

UserTokenPtr User::findTokenUnlocked(const std::string& name) const noexcept
{
    auto it = std::find_if(m_tokens.begin(), m_tokens.end(),
            [&name](const auto& token) noexcept { return token->getName() == name; });
    return (it == m_tokens.end()) ? nullptr : *it;
}

}  // namespace siodb::iomgr::dbengine
