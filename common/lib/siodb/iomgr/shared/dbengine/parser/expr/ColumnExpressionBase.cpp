// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnExpressionBase.h"

namespace siodb::iomgr::dbengine::requests {

// ----- internals -----

bool ColumnExpressionBase::isEqualTo(const Expression& other) const noexcept
{
    const auto& otherColumnExpressionBase = static_cast<const ColumnExpressionBase&>(other);
    return m_tableName == otherColumnExpressionBase.m_tableName;
}

}  // namespace siodb::iomgr::dbengine::requests
