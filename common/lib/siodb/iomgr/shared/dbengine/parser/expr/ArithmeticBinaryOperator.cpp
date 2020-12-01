// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ArithmeticBinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

VariantType ArithmeticBinaryOperator::getResultValueType(
        const ExpressionEvaluationContext& context) const
{
    const auto leftType = m_left->getResultValueType(context);
    const auto rightType = m_right->getResultValueType(context);
    if (!isNumericType(leftType) || !isNumericType(rightType)) return VariantType::kNull;
    return getNumericResultType(leftType, rightType);
}

ColumnDataType ArithmeticBinaryOperator::getColumnDataType(
        const ExpressionEvaluationContext& context) const
{
    const auto leftType = m_left->getColumnDataType(context);
    const auto rightType = m_right->getColumnDataType(context);
    if (!isNumericType(leftType) || !isNumericType(rightType)) return COLUMN_DATA_TYPE_UNKNOWN;
    return getNumericResultType(leftType, rightType);
}

void ArithmeticBinaryOperator::validate(const ExpressionEvaluationContext& context) const
{
    m_left->validate(context);
    m_right->validate(context);

    const auto leftResultType = m_left->getResultValueType(context);
    if (!isNumericType(leftResultType) && !isNullType(leftResultType)) {
        throw std::runtime_error(getExpressionText().asMutableString()
                                 + " operator: left operand type isn't numeric");
    }

    const auto rightResultType = m_right->getResultValueType(context);
    if (!isNumericType(rightResultType) && !isNullType(rightResultType)) {
        throw std::runtime_error(getExpressionText().asMutableString()
                                 + " operator: right operand type isn't numeric");
    }
}

}  // namespace siodb::iomgr::dbengine::requests
