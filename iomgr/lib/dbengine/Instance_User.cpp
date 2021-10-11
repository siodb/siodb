// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Instance.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "SystemDatabase.h"
#include "ThrowDatabaseError.h"
#include "UserAccessKey.h"
#include "UserToken.h"

// Common project headers
#include <siodb/common/log/Log.h>
#include <siodb/common/utils/RandomUtils.h>

namespace siodb::iomgr::dbengine {

UserPtr Instance::findUserChecked(const std::string& userName)
{
    std::lock_guard lock(m_mutex);
    return findUserCheckedUnlocked(userName);
}

UserPtr Instance::findUserChecked(std::uint32_t userId)
{
    std::lock_guard lock(m_mutex);
    return findUserCheckedUnlocked(userId);
}

UserPtr Instance::createUser(const std::string& name, const std::optional<std::string>& realName,
        const std::optional<std::string>& description, bool active, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto currentUser = findUserChecked(currentUserId);
    if (!currentUser->hasPermissions(0, DatabaseObjectType::kUser, 0, kCreatePermissionMask))
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);

    if (m_userRegistry.size() >= m_maxUsers)
        throwDatabaseError(IOManagerMessageId::kErrorTooManyUsers);

    // Check if user already exists
    if (m_userRegistry.byName().count(name) > 0)
        throwDatabaseError(IOManagerMessageId::kErrorUserAlreadyExists, name);

    // Create and register user
    const auto user = std::make_shared<User>(*m_systemDatabase, std::string(name),
            stdext::copy(realName), stdext::copy(description), active);
    m_userRegistry.emplace(*user);
    const TransactionParameters tp(currentUserId, m_systemDatabase->generateNextTransactionId());
    m_systemDatabase->recordUser(*user, tp);
    m_users.emplace(user->getId(), user);
    return user;
}

void Instance::dropUser(const std::string& name, bool userMustExist, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto currentUser = findUserChecked(currentUserId);
    if (!currentUser->hasPermissions(0, DatabaseObjectType::kUser, 0, kCreatePermissionMask))
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);

    auto& index = m_userRegistry.byName();
    const auto it = index.find(name);
    if (it == index.end()) {
        // Support DROP USER IF EXISTS
        if (!userMustExist) return;
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, name);
    }

    const auto id = it->m_id;
    if (id == User::kSuperUserId) throwDatabaseError(IOManagerMessageId::kErrorCannotDropSuperUser);

    auto user = findUserUnlocked(*it);
    m_users.erase(id);
    index.erase(it);

    // Delete assoiated access keys
    for (const auto& accessKey : user->getAccessKeys())
        m_systemDatabase->deleteUserAccessKey(accessKey->getId(), currentUserId);

    // Delete assoiated tokens
    for (const auto& token : user->getTokens())
        m_systemDatabase->deleteUserToken(token->getId(), currentUserId);

    m_systemDatabase->deleteUser(id, currentUserId);
}

void Instance::updateUser(
        const std::string& name, const UpdateUserParameters& params, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto currentUser = findUserChecked(currentUserId);
    if (!currentUser->hasPermissions(0, DatabaseObjectType::kUser, 0, kAlterPermissionMask))
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);

    const auto& index = m_userRegistry.byName();
    const auto it = index.find(name);
    if (it == index.cend()) throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, name);
    auto& mutableUserRecord = stdext::as_mutable(*it);

    const auto user = findUserUnlocked(*it);
    const auto id = user->getId();

    const bool needUpdateRealName = params.m_realName && *params.m_realName != it->m_realName;
    const bool needUpdateDescription =
            params.m_description && *params.m_description != it->m_description;
    const bool needUpdateState = params.m_active && *params.m_active != it->m_active;
    if (!needUpdateRealName && !needUpdateDescription && !needUpdateState) return;

    if (needUpdateRealName) {
        user->setRealName(stdext::copy(*params.m_realName));
        // NOTE: The following one still correct only because we don't index
        // by UserRecord::m_realName.
        mutableUserRecord.m_realName = *params.m_realName;
    }

    if (needUpdateDescription) {
        user->setDescription(stdext::copy(*params.m_description));
        // NOTE: The following one still correct only because we don't index
        // by UserRecord::m_description.
        mutableUserRecord.m_description = *params.m_description;
    }

    if (needUpdateState) {
        // Super user can't be blocked
        if (id == User::kSuperUserId && !*params.m_active)
            throwDatabaseError(IOManagerMessageId::kErrorCannotChangeSuperUserState);
        user->setActive(*params.m_active);
        // NOTE: The following one still correct only because we don't index
        // by UserRecord::m_active.
        mutableUserRecord.m_active = *params.m_active;
    }

    m_systemDatabase->updateUser(id, params, currentUserId);
}

// --- internals ---

UserPtr Instance::findUserCheckedUnlocked(const std::string& userName)
{
    auto user = findUserUnlocked(userName);
    if (user) return user;
    throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);
}

UserPtr Instance::findUserCheckedUnlocked(std::uint32_t userId)
{
    auto user = findUserUnlocked(userId);
    if (user) return user;
    throwDatabaseError(IOManagerMessageId::kErrorUserIdDoesNotExist, userId);
}

UserPtr Instance::findUserUnlocked(const std::string& userName)
{
    DBG_LOG_DEBUG("Looking up user '" << userName << "'");
    const auto& index = m_userRegistry.byName();
    const auto it = index.find(userName);
    if (it == index.cend()) return nullptr;
    return findUserUnlocked(*it);
}

UserPtr Instance::findUserUnlocked(std::uint32_t userId)
{
    DBG_LOG_DEBUG("Looking up user #" << userId);
    const auto& index = m_userRegistry.byId();
    const auto it = index.find(userId);
    if (it == index.cend()) return nullptr;
    return findUserUnlocked(*it);
}

UserPtr Instance::findUserUnlocked(const UserRecord& userRecord)
{
    const auto it = m_users.find(userRecord.m_id);
    if (it != m_users.end()) return it->second;
    auto user = std::make_shared<User>(userRecord);
    m_users.emplace(user->getId(), user);
    const auto userId = user->getId();
    for (const auto& accessKey : user->getAccessKeys())
        m_userAccessKeyIdToUserId.emplace(accessKey->getId(), userId);
    for (const auto& token : user->getTokens())
        m_userAccessKeyIdToUserId.emplace(token->getId(), userId);
    return user;
}

}  // namespace siodb::iomgr::dbengine
