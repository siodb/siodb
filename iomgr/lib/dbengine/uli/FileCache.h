// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "FileDataPtr.h"

// Common project headers
#include <siodb/common/stl_ext/lru_cache.h>

namespace siodb::iomgr::dbengine {

class UniqueLinearIndex;

}  // namespace siodb::iomgr::dbengine

namespace siodb::iomgr::dbengine::uli {

/** Regular cache of recently used files. */
class FileCache final : public stdext::unordered_lru_cache<std::uint64_t, FileDataPtr> {
private:
    using Base = stdext::unordered_lru_cache<std::uint64_t, FileDataPtr>;

public:
    /**
     * Initializes object of class FileCache.
     * @param owner Owner object.
     * @param capacity Cache capacity (maximum allowed size).
     */
    explicit FileCache(const UniqueLinearIndex& owner, std::size_t capacity)
        : Base(capacity)
        , m_owner(owner)
    {
    }

private:
    /** Owner object */
    const UniqueLinearIndex& m_owner;
};

}  // namespace siodb::iomgr::dbengine::uli
