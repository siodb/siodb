// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Instance.h"

// Common project headers
#include <siodb/common/utils/Uuid.h>

namespace siodb::iomgr::dbengine {

/** A helper class for scoped session ending */
class SessionGuard {
public:
    /**
     * Initializes object of class SessionGuard.
     * @param instance Instance.
     * @param sessionUuid Active session UUID.
     */
    SessionGuard(const InstancePtr& instance, const Uuid& sessionUuid) noexcept
        : m_instance(instance)
        , m_sessionUuid(sessionUuid)
    {
    }

    DECLARE_NONCOPYABLE(SessionGuard);

    /**
     * Deinitializes object of class SessionGuard.
     * Ends active session.
     */
    ~SessionGuard()
    {
        try {
            m_instance->endSession(m_sessionUuid);
        } catch (std::exception& e) {
            // ignore exception
        }
    }

private:
    /** Instance */
    const InstancePtr m_instance;

    /** Active session UUID */
    const Uuid m_sessionUuid;
};

}  // namespace siodb::iomgr::dbengine
