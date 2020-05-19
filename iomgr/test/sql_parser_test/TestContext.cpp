// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TestContext.h"

const siodb::iomgr::dbengine::Variant& TestContext::getColumnValue(
        [[maybe_unused]] std::size_t tableIndex, [[maybe_unused]] std::size_t columnIndex)
{
    return m_value;
}

siodb::ColumnDataType TestContext::getColumnDataType(
        [[maybe_unused]] std::size_t tableIndex, [[maybe_unused]] std::size_t columnIndex) const
{
    return siodb::COLUMN_DATA_TYPE_UNKNOWN;
}
