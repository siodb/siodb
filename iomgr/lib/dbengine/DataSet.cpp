// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DataSet.h"

namespace siodb::iomgr::dbengine {

std::optional<std::size_t> DataSet::getColumnIndex(const std::string& name) const noexcept
{
    const auto it = std::find_if(
            m_columnInfos.cbegin(),
            m_columnInfos.cend(), [&name](const auto& e) noexcept { return *e.m_name == name; });
    return it == m_columnInfos.cend()
                   ? std::nullopt
                   : std::optional<std::size_t>(std::distance(m_columnInfos.cbegin(), it));
}

}  // namespace siodb::iomgr::dbengine
