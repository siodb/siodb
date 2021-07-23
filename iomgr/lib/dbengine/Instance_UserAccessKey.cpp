// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Instance.h"

// Project headers
#include "UserAccessKey.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "SystemDatabase.h"
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

std::uint64_t Instance::createUserAccessKey(const std::string& userName, const std::string& keyName,
        const std::string& text, const std::optional<std::string>& description, bool active,
        std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto& index = m_userRegistry.byName();
    const auto it = index.find(userName);
    if (it == index.cend())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);

    // NOTE: We don't index by UserRecord::m_accessKeys, that's why this works
    auto& mutableUserRecord = stdext::as_mutable(*it);

    const auto user = findUserUnlocked(*it);

    const auto id = m_systemDatabase->generateNextUserAccessKeyId();
    const auto accessKey = user->addAccessKey(id, std::string(keyName), std::string(text),
            std::optional<std::string>(description), active);

    // NOTE: We don't index by UserRecord::m_accessKeys, that's why this works
    mutableUserRecord.m_accessKeys.emplace(*accessKey);

    const TransactionParameters tp(currentUserId, m_systemDatabase->generateNextTransactionId());
    m_systemDatabase->recordUserAccessKey(*accessKey, tp);

    return id;
}

void Instance::dropUserAccessKey(const std::string& userName, const std::string& keyName,
        bool mustExist, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto& userIndex = m_userRegistry.byName();
    const auto itUser = userIndex.find(userName);
    if (itUser == userIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);

    // NOTE: We don't index by UserRecord::m_accessKeys, that's why this works
    auto& accessKeyIndex = stdext::as_mutable(*itUser).m_accessKeys.byName();
    const auto itKey = accessKeyIndex.find(keyName);
    if (itKey == accessKeyIndex.end()) {
        if (mustExist) {
            throwDatabaseError(
                    IOManagerMessageId::kErrorUserAccessKeyDoesNotExist, userName, keyName);
        }
        return;
    }

    const auto user = findUserUnlocked(*itUser);
    const auto accessKeyId = itKey->m_id;
    accessKeyIndex.erase(itKey);
    user->deleteAccessKey(keyName);
    m_systemDatabase->deleteUserAccessKey(accessKeyId, currentUserId);
}

void Instance::updateUserAccessKey(const std::string& userName, const std::string& keyName,
        const UpdateUserAccessKeyParameters& params, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto& userIndex = m_userRegistry.byName();
    const auto itUser = userIndex.find(userName);
    if (itUser == userIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);

    auto& accessKeyIndex = stdext::as_mutable(*itUser).m_accessKeys.byName();
    const auto itKey = accessKeyIndex.find(keyName);
    if (itKey == accessKeyIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessKeyDoesNotExist, userName, keyName);

    const auto user = findUserUnlocked(*itUser);
    const auto userAccessKey = user->findAccessKeyChecked(keyName);

    const bool needUpdateDescription =
            params.m_description && *params.m_description != itKey->m_description;
    const bool needUpdateState = params.m_active && *params.m_active != itKey->m_active;
    if (!needUpdateDescription && !needUpdateState) return;

    if (needUpdateDescription) {
        userAccessKey->setDescription(stdext::copy(*params.m_description));
        // NOTE: The following one still correct only because we don't index
        // by UserAccessKeyRecord::m_description.
        stdext::as_mutable(*itKey).m_description = *params.m_description;
    }

    if (needUpdateState) {
        if (user->isSuperUser() && !*params.m_active && user->getActiveAccessKeyCount() == 1) {
            throwDatabaseError(
                    IOManagerMessageId::kErrorCannotDeactivateLastSuperUserAccessKey, keyName);
        }

        // NOTE: We do index by UserAccessKeyRecord::m_active, so modify it in a special way.
        const bool activeNow = userAccessKey->isActive();
        if (accessKeyIndex.modify(
                    itKey,
                    [&params](UserAccessKeyRecord& record) noexcept {
                        record.m_active = *params.m_active;
                    },
                    [activeNow](UserAccessKeyRecord& record) noexcept {
                        record.m_active = activeNow;
                    })) {
            userAccessKey->setActive(*params.m_active);
        } else {
            throwDatabaseError(
                    IOManagerMessageId::kErrorAlterUserAccessKeyFailed, userName, keyName);
        }
    }

    m_systemDatabase->updateUserAccessKey(userAccessKey->getId(), params, currentUserId);
}

}  // namespace siodb::iomgr::dbengine
