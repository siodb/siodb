// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnConstraint.h"

namespace siodb::iomgr::dbengine {

Column* ColumnConstraint::getColumn() const noexcept
{
    return &m_column;
}

}  // namespace siodb::iomgr::dbengine
