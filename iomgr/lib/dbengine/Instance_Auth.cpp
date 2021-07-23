// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Instance.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"
#include "User.h"

// Common project headers
#include <siodb/common/log/Log.h>

namespace siodb::iomgr::dbengine {

void Instance::beginUserAuthentication(const std::string& userName)
{
    const auto user = findUserChecked(userName);
    if (!user->isActive()) throwDatabaseError(IOManagerMessageId::kErrorUserAccessDenied, userName);
    if (user->getActiveAccessKeyCount() == 0)
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessDenied, userName);
}

AuthenticationResult Instance::authenticateUser(
        const std::string& userName, const std::string& signature, const std::string& challenge)
{
    const auto user = findUserChecked(userName);
    if (!user->authenticate(signature, challenge))
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessDenied, userName);
    LOG_INFO << "Instance: User '" << userName << "' authenticated.";
    return AuthenticationResult(user->getId(), beginSession());
}

std::uint32_t Instance::authenticateUser(const std::string& userName, const std::string& token)
{
    const auto user = findUserChecked(userName);
    if (!user->authenticate(token))
        throwDatabaseError(IOManagerMessageId::kErrorUserAccessDenied, userName);
    LOG_INFO << "Instance: User '" << userName << "' authenticated via token.";
    return user->getId();
}

Uuid Instance::beginSession()
{
    std::lock_guard lock(m_sessionMutex);

    Uuid sessionUuid;
    do {
        sessionUuid = m_sessionUuidGenerator();
    } while (m_activeSessions.find(sessionUuid) != m_activeSessions.end());

    m_activeSessions.emplace(sessionUuid, std::make_shared<ClientSession>(sessionUuid));
    LOG_INFO << "Session " << sessionUuid << " started";
    return sessionUuid;
}

void Instance::endSession(const Uuid& sessionUuid)
{
    std::lock_guard lock(m_sessionMutex);
    if (m_activeSessions.erase(sessionUuid) == 0)
        throwDatabaseError(IOManagerMessageId::kErrorSessionDoesNotExist, sessionUuid);
    LOG_INFO << "Session " << sessionUuid << " finished";
}

}  // namespace siodb::iomgr::dbengine
