// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConstraintCache.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Constraint.h"
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

// ----- internals -----

void ConstraintCache::evict()
{
    try {
        Base::evict();
    } catch (stdext::lru_cache_full_error&) {
        throwDatabaseError(IOManagerMessageId::kErrorConstraintCacheFull, m_table.getDatabaseName(),
                m_table.getName());
    }
}

bool ConstraintCache::can_evict(
        [[maybe_unused]] const std::uint64_t& key, const ConstraintPtr& constraint) const noexcept
{
    return !constraint->isSystemConstraint() && constraint.use_count() == 1;
}

}  // namespace siodb::iomgr::dbengine
