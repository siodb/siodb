// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ArithmeticUnaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

VariantType ArithmeticUnaryOperator::getResultValueType(const Context& context) const
{
    // + and - unary operators returns signed numbers always.
    const auto resultType = m_operand->getResultValueType(context);
    if (!isNumericType(resultType)) return VariantType::kNull;
    return resultType <= VariantType::kInt32 ? VariantType::kInt32 : getSignedType(resultType);
}

ColumnDataType ArithmeticUnaryOperator::getColumnDataType(const Context& context) const
{
    const auto resultType = m_operand->getColumnDataType(context);
    if (!isNumericType(resultType)) return COLUMN_DATA_TYPE_UNKNOWN;
    return resultType <= COLUMN_DATA_TYPE_INT32 ? COLUMN_DATA_TYPE_INT32
                                                : getSignedType(resultType);
}

void ArithmeticUnaryOperator::validate(const Context& context) const
{
    m_operand->validate(context);
    const auto resultType = m_operand->getResultValueType(context);
    if (!isNumericType(resultType) && !isNullType(resultType)) {
        throw std::runtime_error(
                getExpressionText().asMutableString() + " operator: operand type isn't numeric");
    }
}

}  // namespace siodb::iomgr::dbengine::requests
