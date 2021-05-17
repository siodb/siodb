// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/** User ID generator interface */
class UserIdGenerator {
public:
    /** De-initializes object of class UserIdGenerator */
    virtual ~UserIdGenerator() = default;

    /**
     * Generates new unique user ID.
     * @return New user ID.
     */
    virtual std::uint32_t generateNextUserId() = 0;
};

}  // namespace siodb::iomgr::dbengine
