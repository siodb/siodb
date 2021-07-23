// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnExpressionBase.h"

namespace siodb::iomgr::dbengine::requests {

void ColumnExpressionBase::setSingleDatasetTableIndex(std::size_t datasetTableIndex)
{
    std::vector<std::size_t> datasetTableIndices;
    datasetTableIndices.push_back(datasetTableIndex);
    m_datasetTableIndices = std::move(datasetTableIndices);
}

// --- internals ---

bool ColumnExpressionBase::isEqualTo(const Expression& other) const noexcept
{
    const auto& otherColumnExpressionBase = static_cast<const ColumnExpressionBase&>(other);
    return m_tableName == otherColumnExpressionBase.m_tableName;
}

}  // namespace siodb::iomgr::dbengine::requests
