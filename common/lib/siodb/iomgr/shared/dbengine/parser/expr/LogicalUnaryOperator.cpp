// Copyright (C) 2019 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "LogicalUnaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

VariantType LogicalUnaryOperator::getResultValueType(
        const ExpressionEvaluationContext& context) const
{
    return isBoolType(m_operand->getResultValueType(context)) ? VariantType::kBool
                                                              : VariantType::kNull;
}

ColumnDataType LogicalUnaryOperator::getColumnDataType(
        const ExpressionEvaluationContext& context) const
{
    return (m_operand->getColumnDataType(context) == COLUMN_DATA_TYPE_BOOL)
                   ? COLUMN_DATA_TYPE_BOOL
                   : COLUMN_DATA_TYPE_UNKNOWN;
}

void LogicalUnaryOperator::validate(const ExpressionEvaluationContext& context) const
{
    m_operand->validate(context);
    const auto resultType = m_operand->getResultValueType(context);
    if (!isBoolType(resultType) && !isNullType(resultType)) {
        throw std::runtime_error(
                getExpressionText().asMutableString() + " operator: operand type isn't boolean");
    }
}

}  // namespace siodb::iomgr::dbengine::requests
