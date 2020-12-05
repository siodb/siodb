// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "LogicalBinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

VariantType LogicalBinaryOperator::getResultValueType(
        const ExpressionEvaluationContext& context) const
{
    const auto leftType = m_left->getResultValueType(context);
    const auto rightType = m_right->getResultValueType(context);

    if (leftType != VariantType::kBool || rightType != VariantType::kBool)
        return VariantType::kNull;

    return VariantType::kBool;
}

ColumnDataType LogicalBinaryOperator::getColumnDataType(
        const ExpressionEvaluationContext& context) const
{
    const auto leftType = m_left->getColumnDataType(context);
    const auto rightType = m_right->getColumnDataType(context);

    if (leftType != COLUMN_DATA_TYPE_BOOL || rightType != COLUMN_DATA_TYPE_BOOL)
        return COLUMN_DATA_TYPE_UNKNOWN;

    return COLUMN_DATA_TYPE_BOOL;
}

void LogicalBinaryOperator::validate(const ExpressionEvaluationContext& context) const
{
    m_left->validate(context);
    m_right->validate(context);

    const auto leftResultType = m_left->getResultValueType(context);
    if (!isBoolType(leftResultType) && !isNullType(leftResultType)) {
        throw std::runtime_error(getExpressionText().asMutableString()
                                 + " operator: left operand type isn't boolean");
    }

    const auto rightResultType = m_right->getResultValueType(context);
    if (!isBoolType(rightResultType) && !isNullType(rightResultType)) {
        throw std::runtime_error(getExpressionText().asMutableString()
                                 + " operator: right operand type isn't boolean");
    }
}

}  // namespace siodb::iomgr::dbengine::requests
