// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DatabaseContext.h"

namespace siodb::iomgr::dbengine::requests {

const Variant& DatabaseContext::getColumnValue(
        std::size_t tableIndex, const std::size_t columnIndex)
{
    return m_dataSets.at(tableIndex)->getColumnValue(columnIndex);
}

ColumnDataType DatabaseContext::getColumnDataType(
        std::size_t tableIndex, std::size_t columnIndex) const
{
    return m_dataSets.at(tableIndex)->getColumnDataType(columnIndex);
}

/// ------ internals ------

DatabaseContext::NameToIndexMapping DatabaseContext::makeNameToIndexMapping() const
{
    NameToIndexMapping result;
    if (!m_dataSets.empty()) {
        result.reserve(m_dataSets.size());
        for (std::size_t i = 0; i < m_dataSets.size(); ++i) {
            const auto& e = m_dataSets[i];
            result.emplace(std::cref(e->getName()), i);
            const auto& alias = e->getAlias();
            if (!alias.empty()) result.emplace(std::cref(alias), i);
        }
    }
    return result;
}

}  // namespace siodb::iomgr::dbengine::requests
