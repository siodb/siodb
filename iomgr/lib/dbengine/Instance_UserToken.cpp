// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Instance.h"

// Project headers
#include "UserToken.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "SystemDatabase.h"
#include "ThrowDatabaseError.h"
#include <siodb/common/utils/RandomUtils.h>

namespace siodb::iomgr::dbengine {

std::pair<std::uint64_t, BinaryValue> Instance::createUserToken(const std::string& userName,
        const std::string& tokenName, const std::optional<BinaryValue>& value,
        const std::optional<std::string>& description,
        const std::optional<std::time_t>& expirationTimestamp, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto currentUser = findUserChecked(currentUserId);
    if (!currentUser->hasPermissions(0, DatabaseObjectType::kUserToken, 0, kCreatePermissionMask))
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);

    std::pair<std::uint64_t, BinaryValue> result;

    const auto& index = m_userRegistry.byName();
    const auto it = index.find(userName);
    if (it == index.cend())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);

    // NOTE: We don't index by UserRecord::m_tokens.
    auto& mutableUserRecord = stdext::as_mutable(*it);

    const auto user = findUserUnlocked(*it);
    const auto id = m_systemDatabase->generateNextUserTokenId();
    BinaryValue v;
    if (value)
        v = *value;
    else {
        // Need to generate new token value
        result.second.resize(kGeneratedTokenLength);
        do {
            try {
                utils::getRandomBytes(result.second.data(), result.second.size());
            } catch (std::exception& ex) {
                throwDatabaseError(IOManagerMessageId::kErrorCannotGenerateUserToken, ex.what());
            }
        } while (user->checkToken(result.second, true));
        v = result.second;
    }

    const auto token = user->addToken(id, std::string(tokenName), v,
            std::optional<std::time_t>(expirationTimestamp),
            std::optional<std::string>(description));

    // NOTE: We don't index by UserRecord::m_tokens, that's why this works.
    mutableUserRecord.m_tokens.emplace(*token);

    const TransactionParameters tp(currentUserId, m_systemDatabase->generateNextTransactionId());
    m_systemDatabase->recordUserToken(*token, tp);
    return result;
}

void Instance::dropUserToken(const std::string& userName, const std::string& tokenName,
        bool mustExist, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto currentUser = findUserChecked(currentUserId);
    if (!currentUser->hasPermissions(0, DatabaseObjectType::kUserToken, 0, kDropPermissionMask))
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);

    const auto& userIndex = m_userRegistry.byName();
    const auto itUser = userIndex.find(userName);
    if (itUser == userIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);

    // NOTE: We don't index by UserRecord::m_accessKeys.
    auto& tokenIndex = stdext::as_mutable(*itUser).m_tokens.byName();
    const auto itToken = tokenIndex.find(tokenName);
    if (itToken == tokenIndex.end()) {
        if (mustExist) {
            throwDatabaseError(
                    IOManagerMessageId::kErrorUserTokenDoesNotExist, userName, tokenName);
        }
        return;
    }

    const auto user = findUserUnlocked(*itUser);

    const auto tokenId = itToken->m_id;
    tokenIndex.erase(itToken);
    user->deleteToken(tokenName);
    m_systemDatabase->deleteUserToken(tokenId, currentUserId);
}

void Instance::updateUserToken(const std::string& userName, const std::string& tokenName,
        const UpdateUserTokenParameters& params, std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    const auto currentUser = findUserChecked(currentUserId);
    if (!currentUser->hasPermissions(0, DatabaseObjectType::kUserToken, 0, kAlterPermissionMask))
        throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);

    const auto& userIndex = m_userRegistry.byName();
    const auto itUser = userIndex.find(userName);
    if (itUser == userIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);

    auto& tokenIndex = stdext::as_mutable(*itUser).m_tokens.byName();
    const auto itToken = tokenIndex.find(tokenName);
    if (itToken == tokenIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserTokenDoesNotExist, userName, tokenName);

    const auto user = findUserUnlocked(*itUser);
    const auto userToken = user->findTokenChecked(tokenName);

    const bool needUpdateDescription =
            params.m_description && *params.m_description != itToken->m_description;
    const bool needUpdateExpirationTimestamp =
            params.m_expirationTimestamp
            && *params.m_expirationTimestamp != itToken->m_expirationTimestamp;
    if (!needUpdateDescription && !needUpdateExpirationTimestamp) return;

    if (needUpdateDescription) {
        userToken->setDescription(stdext::copy(*params.m_description));
        // NOTE: The following one still correct only because we don't index
        // by UserTokenRecord::m_description.
        stdext::as_mutable(*itToken).m_description = *params.m_description;
    }

    if (needUpdateExpirationTimestamp)
        userToken->setExpirationTimestamp(*params.m_expirationTimestamp);

    m_systemDatabase->updateUserToken(userToken->getId(), params, currentUserId);
}

void Instance::checkUserToken(const std::string& userName, const std::string& tokenName,
        const BinaryValue& tokenValue, [[maybe_unused]] std::uint32_t currentUserId)
{
    std::lock_guard lock(m_cacheMutex);

    // No permission check here at this time, but need to think again if we ever need it here.

    const auto& userIndex = m_userRegistry.byName();
    const auto itUser = userIndex.find(userName);
    if (itUser == userIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserDoesNotExist, userName);

    auto& tokenIndex = stdext::as_mutable(*itUser).m_tokens.byName();
    const auto itToken = tokenIndex.find(tokenName);
    if (itToken == tokenIndex.end())
        throwDatabaseError(IOManagerMessageId::kErrorUserTokenDoesNotExist, userName, tokenName);

    const auto user = findUserUnlocked(*itUser);
    const auto userToken = user->findTokenChecked(tokenName);
    if (!userToken->checkValue(tokenValue))
        throwDatabaseError(IOManagerMessageId::kErrorUserTokenCheckFailed, userName, tokenName);
}

}  // namespace siodb::iomgr::dbengine
