// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SingleColumnExpression.h"

// STL headers
#include <sstream>

namespace siodb::iomgr::dbengine::requests {

SingleColumnExpression::SingleColumnExpression(
        std::string&& tableName, std::string&& columnName) noexcept
    : ColumnExpressionBase(ExpressionType::kSingleColumnReference, std::move(tableName))
    , m_columnName(std::move(columnName))
{
}

VariantType SingleColumnExpression::getResultValueType(
        const ExpressionEvaluationContext& context) const
{
    checkHasTableAndColumnIndices();
    // TODO(cxxman): Make this somehow better, take CLOBs into account.
#if 0
    const auto& value = context.getColumnValue(*m_datasetTableIndex, *m_datasetColumnIndex);
    return value.getValueType();
#else
    return convertColumnDataTypeToVariantType(
            context.getColumnDataType(*m_datasetTableIndex, *m_datasetColumnIndex));
#endif
}

ColumnDataType SingleColumnExpression::getColumnDataType(
        const ExpressionEvaluationContext& context) const
{
    checkHasTableAndColumnIndices();
    return context.getColumnDataType(*m_datasetTableIndex, *m_datasetColumnIndex);
}

MutableOrConstantString SingleColumnExpression::getExpressionText() const
{
    std::ostringstream ss;
    ss << "Column '";
    if (!m_tableName.empty()) ss << m_tableName << '.';
    ss << m_columnName << '\'';
    return ss.str();
}

std::size_t SingleColumnExpression::getSerializedSize() const noexcept
{
    return getCommonSerializedSize() + ::getSerializedSize(m_columnName);
}

void SingleColumnExpression::validate(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    checkHasTableAndColumnIndices();
}

Variant SingleColumnExpression::evaluate(ExpressionEvaluationContext& context) const
{
    checkHasTableAndColumnIndices();
    return context.getColumnValue(*m_datasetTableIndex, *m_datasetColumnIndex);
}

std::uint8_t* SingleColumnExpression::serializeUnchecked(std::uint8_t* buffer) const
{
    buffer = serializeCommonUnchecked(buffer);
    return ::serializeUnchecked(m_columnName, buffer);
}

Expression* SingleColumnExpression::clone() const
{
    return new SingleColumnExpression(std::string(m_tableName), std::string(m_columnName));
}

// ---- internals ----

bool SingleColumnExpression::isEqualTo(const Expression& other) const noexcept
{
    const auto& otherSingleColumnExpression = static_cast<const SingleColumnExpression&>(other);
    return m_tableName == otherSingleColumnExpression.m_tableName
           && m_columnName == otherSingleColumnExpression.m_columnName;
}

void SingleColumnExpression::dumpImpl(std::ostream& os) const
{
    os << '\'' << m_tableName << "'.'" << m_columnName << '\'';
}

void SingleColumnExpression::checkHasTableAndColumnIndices() const
{
    if (!m_datasetTableIndex) throw std::runtime_error("Dataset table index is not set");
    if (!m_datasetColumnIndex) throw std::runtime_error("Dataset column index is not set");
}

}  // namespace siodb::iomgr::dbengine::requests
