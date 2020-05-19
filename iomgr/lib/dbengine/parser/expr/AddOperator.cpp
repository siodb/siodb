// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "AddOperator.h"

namespace siodb::iomgr::dbengine::requests {

VariantType AddOperator::getResultValueType(const Context& context) const
{
    const auto leftType = m_left->getResultValueType(context);
    const auto rightType = m_right->getResultValueType(context);

    if (leftType == rightType && leftType == VariantType::kString) return VariantType::kString;

    return ArithmeticBinaryOperator::getResultValueType(context);
}

ColumnDataType AddOperator::getColumnDataType(const Context& context) const
{
    const auto leftType = m_left->getColumnDataType(context);
    const auto rightType = m_right->getColumnDataType(context);

    if (leftType == rightType
            && (leftType == COLUMN_DATA_TYPE_TEXT || leftType == COLUMN_DATA_TYPE_NTEXT))
        return leftType;

    return ArithmeticBinaryOperator::getColumnDataType(context);
}

MutableOrConstantString AddOperator::getExpressionText() const
{
    return "ADD";
}

void AddOperator::validate(const Context& context) const
{
    m_left->validate(context);
    m_right->validate(context);

    const auto leftResultType = m_left->getResultValueType(context);
    const auto rightResultType = m_right->getResultValueType(context);

    if ((isNullType(leftResultType) || isNumericType(leftResultType))
            && (isNullType(rightResultType) || isNumericType(rightResultType)))
        return;

    if ((isNullType(leftResultType) || isStringType(leftResultType))
            && (isNullType(rightResultType) || isStringType(rightResultType)))
        return;

    throw std::runtime_error("Add operator: operand types aren't strings or numeric");
}

Variant AddOperator::evaluate(Context& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);
    if (leftValue.isNull() || rightValue.isNull()) return nullptr;
    return leftValue + rightValue;
}

Expression* AddOperator::clone() const
{
    return cloneImpl<AddOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
