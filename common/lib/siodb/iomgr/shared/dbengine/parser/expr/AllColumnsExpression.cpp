// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "AllColumnsExpression.h"

namespace siodb::iomgr::dbengine::requests {

VariantType AllColumnsExpression::getResultValueType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    throw std::runtime_error("All columns expression doesn't have result value type");
}

ColumnDataType AllColumnsExpression::getColumnDataType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    throw std::runtime_error("All columns expression doesn't have column type");
}

MutableOrConstantString AllColumnsExpression::getExpressionText() const
{
    return "*";
}

std::size_t AllColumnsExpression::getSerializedSize() const noexcept
{
    return getCommonSerializedSize();
}

void AllColumnsExpression::validate(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    if (!m_datasetTableIndex) throw std::runtime_error("Dataset table index is not set");
}

Variant AllColumnsExpression::evaluate([[maybe_unused]] ExpressionEvaluationContext& context) const
{
    throw std::runtime_error("Evaluating of '*' column is prohibited");
}

std::uint8_t* AllColumnsExpression::serializeUnchecked(std::uint8_t* buffer) const
{
    return serializeCommonUnchecked(buffer);
}

Expression* AllColumnsExpression::clone() const
{
    return new AllColumnsExpression(std::string(m_tableName));
}

// ---- internals -----

void AllColumnsExpression::dumpImpl(std::ostream& os) const
{
    os << m_tableName;
}

}  // namespace siodb::iomgr::dbengine::requests
