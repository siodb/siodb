// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConcatenationOperator.h"

namespace siodb::iomgr::dbengine::requests {

VariantType ConcatenationOperator::getResultValueType(
        const ExpressionEvaluationContext& context) const
{
    return hasNullOperand(context) ? VariantType::kNull : VariantType::kString;
}

ColumnDataType ConcatenationOperator::getColumnDataType(
        const ExpressionEvaluationContext& context) const
{
    return hasNullOperand(context) ? COLUMN_DATA_TYPE_UNKNOWN : COLUMN_DATA_TYPE_TEXT;
}

MutableOrConstantString ConcatenationOperator::getExpressionText() const
{
    return "Concatenation";
}

Variant ConcatenationOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);

    if (leftValue.isNull() || rightValue.isNull()) return nullptr;

    return leftValue.concatenate(rightValue);
}

Expression* ConcatenationOperator::clone() const
{
    return cloneImpl<ConcatenationOperator>();
}

}  // namespace siodb::iomgr::dbengine::requests
