// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConstraintDefinitionCache.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ConstraintDefinition.h"
#include "Database.h"
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

void ConstraintDefinitionCache::evict()
{
    try {
        Base::evict();
    } catch (stdext::lru_cache_full_error&) {
        throwDatabaseError(
                IOManagerMessageId::kErrorConstraintDefinitionCacheFull, m_database.getName());
    }
}

bool ConstraintDefinitionCache::can_evict([[maybe_unused]] const std::uint64_t& key,
        const ConstraintDefinitionPtr& constraintDefinition) const noexcept
{
    return !constraintDefinition->isSystemConstraintDefinition()
           && constraintDefinition.use_count() == 1;
}

}  // namespace siodb::iomgr::dbengine
