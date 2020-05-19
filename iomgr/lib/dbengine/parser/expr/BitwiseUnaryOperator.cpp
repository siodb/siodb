// Copyright (C) 2019 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BitwiseUnaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

VariantType BitwiseUnaryOperator::getResultValueType(const Context& context) const
{
    const auto resultType = m_operand->getResultValueType(context);
    if (!isIntegerType(resultType)) return VariantType::kNull;
    return resultType <= VariantType::kInt32 ? VariantType::kInt32 : resultType;
}

ColumnDataType BitwiseUnaryOperator::getColumnDataType(const Context& context) const
{
    const auto resultType = m_operand->getColumnDataType(context);
    if (!isIntegerType(resultType)) return COLUMN_DATA_TYPE_UNKNOWN;
    return resultType <= COLUMN_DATA_TYPE_INT32 ? COLUMN_DATA_TYPE_INT32 : resultType;
}

/**
 * Checks if operand is integer and valid.
 * @std::runtime_error if operand isn't integer or not valid
 */
void BitwiseUnaryOperator::validate(const Context& context) const
{
    m_operand->validate(context);
    const auto resultType = m_operand->getResultValueType(context);
    if (!isIntegerType(resultType) && !isNullType(resultType)) {
        throw std::runtime_error(
                getExpressionText().asMutableString() + " operator: operand type isn't integer");
    }
}

}  // namespace siodb::iomgr::dbengine::requests
