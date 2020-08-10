// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TestContext.h"

TestContext::TestContext()
{
    m_values.reserve(6);
    m_values.emplace_back(std::uint64_t(1));
    m_values.emplace_back("121 Anselmo str.");
    m_values.emplace_back(std::int32_t(-25));
    m_values.emplace_back(1230.0165432);
    m_values.emplace_back(siodb::RawDateTime("2019-12-19", siodb::RawDateTime::kDefaultDateFormat));
    m_values.emplace_back();
}

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
