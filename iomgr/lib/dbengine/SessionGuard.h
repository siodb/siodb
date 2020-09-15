// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Instance.h"

namespace siodb::iomgr::dbengine {

/** A helper class for the scoped session management. */
class SessionGuard {
public:
    /**
     * Initializes object of class SessionGuard.
     * @param instance Database engine instance.
     * @param sessionUuid Active session UUID.
     */
    SessionGuard(Instance& instance, const Uuid& sessionUuid) noexcept
        : m_instance(instance)
        , m_sessionUuid(sessionUuid)
    {
    }

    DECLARE_NONCOPYABLE(SessionGuard);

    /**
     * De-initializes object of class SessionGuard.
     * Ends active session.
     */
    ~SessionGuard()
    {
        try {
            m_instance.endSession(m_sessionUuid);
        } catch (std::exception& e) {
            // ignore exceptions
        }
    }

private:
    /** Database engine instance */
    Instance& m_instance;

    /** Active session UUID */
    const Uuid m_sessionUuid;
};

}  // namespace siodb::iomgr::dbengine
