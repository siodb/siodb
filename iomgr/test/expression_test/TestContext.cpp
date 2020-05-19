// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TestContext.h"

const dbengine::Variant& TestContext::getColumnValue(
        [[maybe_unused]] std::size_t tableIndex, std::size_t columnIndex)
{
    return m_values.at(columnIndex);
}

siodb::ColumnDataType TestContext::getColumnDataType(
        [[maybe_unused]] std::size_t tableIndex, std::size_t columnIndex) const
{
    return dbengine::convertVariantTypeToColumnDataType(m_values.at(columnIndex).getValueType());
}
