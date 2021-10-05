// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
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

    for (const auto& tokenRecord : userRecord.m_tokens.byId())
        m_tokens.push_back(std::make_shared<UserToken>(*this, tokenRecord));

    for (const auto& permissionRecord : userRecord.m_permissions.byId()) {
        m_grantedPermissions.emplace(
                UserPermissionKey(permissionRecord), UserPermissionData(permissionRecord));
    }
}

UserAccessKeyPtr User::findAccessKeyChecked(const std::string& name) const
{
    auto accessKey = findAccessKeyUnlocked(name);
    if (accessKey) return accessKey;
    throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyDoesNotExist, m_name, name);
}

UserAccessKeyPtr User::findAccessKeyChecked(std::uint64_t id) const
{
    auto accessKey = findAccessKeyUnlocked(id);
    if (accessKey) return accessKey;
    throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyIdDoesNotExist, id);
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
    auto token = findTokenUnlocked(name);
    if (token) return token;
    throwDatabaseError(IOManagerMessageId::kErrorUserTokenDoesNotExist, m_name, name);
}

UserTokenPtr User::findTokenChecked(std::uint64_t id) const
{
    auto token = findTokenUnlocked(id);
    if (token) return token;
    throwDatabaseError(IOManagerMessageId::kErrorUserTokenIdDoesNotExist, id);
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

void User::checkHasPermissions(std::uint32_t databaseId, DatabaseObjectType objectType,
        std::uint64_t objectId, std::uint64_t permissions, bool withGrantOption) const
{
    const UserPermissionKey permissionKey(databaseId, objectType, objectId);
    if (!hasPermissions(permissionKey, permissions, withGrantOption))
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
}

bool User::hasPermissions(std::uint32_t databaseId, DatabaseObjectType objectType,
        std::uint64_t objectId, std::uint64_t permissions, bool withGrantOption) const noexcept
{
    const UserPermissionKey permissionKey(databaseId, objectType, objectId);
    return hasPermissions(permissionKey, permissions, withGrantOption);
}

bool User::hasPermissions(const UserPermissionKey& permissionKey, std::uint64_t permissions,
        bool withGrantOption) const noexcept
{
    if (isSuperUser()) return true;
    const auto it = m_grantedPermissions.find(permissionKey);
    return it != m_grantedPermissions.end()
           && (it->second.getPermissions() & permissions) == permissions
           && (!withGrantOption
                   || (it->second.getEffectiveGrantOptions() & permissions) == permissions);
}

UserPermissionDataEx User::grantPermissions(
        const UserPermissionKey& permissionKey, std::uint64_t permissions, bool withGrantOption)
{
    const UserPermissionDataEx newData(0, permissions, withGrantOption ? permissions : 0);
    const auto result = m_grantedPermissions.emplace(permissionKey, newData);
    if (result.second) return newData;
    auto& existingData = result.first->second;
    existingData.addPermissions(permissions, withGrantOption);
    return existingData;
}

std::optional<std::pair<UserPermissionDataEx, bool>> User::revokePermissions(
        const UserPermissionKey& permissisonKey, std::uint64_t permissions)
{
    const auto it = m_grantedPermissions.find(permissisonKey);
    if (it != m_grantedPermissions.end()
            && (it->second.getEffectiveGrantOptions() & permissions) != 0) {
        it->second.removePermissions(permissions);
        if (it->second.getPermissions() == 0) {
            const auto data = it->second;
            m_grantedPermissions.erase(it);
            return std::make_pair(data, true);
        }
        return std::make_pair(it->second, false);
    }
    return {};
}

std::pair<bool, bool> User::setPermissionRecordId(
        const UserPermissionKey& permissionKey, std::uint64_t id)
{
    const auto it = m_grantedPermissions.find(permissionKey);
    if (it == m_grantedPermissions.end()) return std::make_pair(false, false);
    if (it->second.getId() != 0) return std::make_pair(true, false);
    it->second.setId(id);
    return std::make_pair(true, true);
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
            || tokenValue.length() > UserToken::kMaxSize * 2) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidUserTokenValue);
    }

    BinaryValue bv(tokenValue.length() / 2);
    try {
        boost::algorithm::unhex(tokenValue.cbegin(), tokenValue.cend(), bv.begin());
    } catch (...) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidUserTokenValue);
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

// --- internals ---

std::string&& User::validateUserName(std::string&& userName)
{
    if (isValidDatabaseObjectName(userName)) return std::move(userName);
    throwDatabaseError(IOManagerMessageId::kErrorInvalidUserName, userName);
}

UserAccessKeyPtr User::findAccessKeyUnlocked(const std::string& name) const noexcept
{
    const auto it = std::find_if(m_accessKeys.begin(), m_accessKeys.end(),
            [&name](const auto& accessKey) noexcept { return accessKey->getName() == name; });
    return (it == m_accessKeys.end()) ? nullptr : *it;
}

UserAccessKeyPtr User::findAccessKeyUnlocked(std::uint64_t id) const noexcept
{
    const auto it = std::find_if(m_accessKeys.begin(), m_accessKeys.end(),
            [id](const auto& accessKey) noexcept { return accessKey->getId() == id; });
    return (it == m_accessKeys.end()) ? nullptr : *it;
}

UserTokenPtr User::findTokenUnlocked(const std::string& name) const noexcept
{
    const auto it = std::find_if(m_tokens.begin(), m_tokens.end(),
            [&name](const auto& token) noexcept { return token->getName() == name; });
    return (it == m_tokens.end()) ? nullptr : *it;
}

UserTokenPtr User::findTokenUnlocked(std::uint64_t id) const noexcept
{
    const auto it = std::find_if(m_tokens.begin(), m_tokens.end(),
            [id](const auto& token) noexcept { return token->getId() == id; });
    return (it == m_tokens.end()) ? nullptr : *it;
}

}  // namespace siodb::iomgr::dbengine
