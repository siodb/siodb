// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/Uuid.h>

namespace siodb::iomgr::dbengine {

/** User authentication result structure */
struct AuthenticationResult {
    /** Initializes object of class AuthenticationResult */
    AuthenticationResult() noexcept
        : m_userId(0)
        , m_sessionUuid(utils::getZeroUuid())
    {
    }

    /**
     * Initializes object of class AuthenticationResult.
     * @param userId User ID.
     * @param sessionUuid Session UUID.
     */
    AuthenticationResult(std::uint32_t userId, const Uuid& sessionUuid) noexcept
        : m_userId(userId)
        , m_sessionUuid(sessionUuid)
    {
    }

    /** User ID */
    std::uint32_t m_userId;

    /** Session ID */
    Uuid m_sessionUuid;
};

}  // namespace siodb::iomgr::dbengine
