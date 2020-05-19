// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BitwiseBinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

VariantType BitwiseBinaryOperator::getResultValueType(const Context& context) const
{
    const auto leftType = m_left->getResultValueType(context);
    const auto rightType = m_right->getResultValueType(context);
    if (!isIntegerType(leftType) || !isIntegerType(rightType)) return VariantType::kNull;
    return getNumericResultType(leftType, rightType);
}

ColumnDataType BitwiseBinaryOperator::getColumnDataType(const Context& context) const
{
    const auto leftType = m_left->getColumnDataType(context);
    const auto rightType = m_right->getColumnDataType(context);
    if (!isIntegerType(leftType) || !isIntegerType(rightType)) return COLUMN_DATA_TYPE_UNKNOWN;
    return getNumericResultType(leftType, rightType);
}

void BitwiseBinaryOperator::validate(const Context& context) const
{
    m_left->validate(context);
    m_right->validate(context);

    const auto leftResultType = m_left->getResultValueType(context);
    if (!isIntegerType(leftResultType) && !isNullType(leftResultType)) {
        throw std::runtime_error(getExpressionText().asMutableString()
                                 + " operator: left operand type isn't integer");
    }

    const auto rightResultType = m_right->getResultValueType(context);
    if (!isIntegerType(rightResultType) && !isNullType(rightResultType)) {
        throw std::runtime_error(getExpressionText().asMutableString()
                                 + " operator: right operand type isn't integer");
    }
}

}  // namespace siodb::iomgr::dbengine::requests
