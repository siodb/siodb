// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnSpecification.h"

// Project headers
#include "parser/expr/ConstantExpression.h"

namespace siodb::iomgr::dbengine {

ColumnSpecification::ColumnSpecification(const SimpleColumnSpecification& src)
    : m_name(src.m_name)
    , m_dataType(src.m_dataType)
    , m_dataBlockDataAreaSize(kDefaultDataFileDataAreaSize)
{
    if (src.m_notNull) {
        auto expr = std::make_unique<requests::ConstantExpression>(Variant(*src.m_notNull));
        m_constraints.emplace_back(
                std::string(), ConstraintType::kNotNull, std::move(expr), std::nullopt);
    }
    if (!src.m_defaultValue.isNull()) {
        auto expr = std::make_unique<requests::ConstantExpression>(Variant(src.m_defaultValue));
        m_constraints.emplace_back(
                std::string(), ConstraintType::kDefaultValue, std::move(expr), std::nullopt);
    }
}

}  // namespace siodb::iomgr::dbengine
