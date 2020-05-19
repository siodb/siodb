// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/Uuid.h>

namespace siodb::iomgr {

/** Client session on the IO Manager. */
class ClientSession {
public:
    /** 
     * Initializes object of class ClientSession.
     * @param uuid Session UUID.
     */
    explicit ClientSession(const Uuid& uuid) noexcept
        : m_uuid(uuid)
    {
    }

    /**
     * Returns session UUID.
     * @return Session UUID.
     */
    const Uuid& getUuid() const noexcept
    {
        return m_uuid;
    }

private:
    /** Session UUID. */
    const Uuid m_uuid;
};

}  // namespace siodb::iomgr
