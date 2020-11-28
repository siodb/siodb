// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConstantExpression.h"

// STL headers
#include <sstream>

namespace siodb::iomgr::dbengine::requests {

bool ConstantExpression::isConstant() const noexcept
{
    return true;
}

bool ConstantExpression::canCastAsDateTime(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const noexcept
{
    if (m_value.isDateTime()) return true;

    if (m_value.isString()) {
        try {
            m_value.asDateTime();
            return true;
        } catch (...) {
        }
    }

    return false;
}

VariantType ConstantExpression::getResultValueType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    return m_value.getValueType();
}

ColumnDataType ConstantExpression::getColumnDataType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    return convertVariantTypeToColumnDataType(m_value.getValueType());
}

std::size_t ConstantExpression::getSerializedSize() const noexcept
{
    return getExpressionTypeSerializedSize(m_type) + m_value.getSerializedSize();
}

void ConstantExpression::validate([[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    // Constant expression is always valid.
}

Variant ConstantExpression::evaluate([[maybe_unused]] ExpressionEvaluationContext& context) const
{
    return m_value;
}

std::uint8_t* ConstantExpression::serializeUnchecked(std::uint8_t* buffer) const
{
    return m_value.serializeUnchecked(serializeExpressionTypeUnchecked(m_type, buffer));
}

MutableOrConstantString ConstantExpression::getExpressionText() const
{
    return *m_value.asString();
}

Expression* ConstantExpression::clone() const
{
    return new ConstantExpression(Variant(m_value));
}

// ----- internals -----

bool ConstantExpression::isEqualTo(const Expression& other) const noexcept
{
    return m_value == static_cast<const ConstantExpression&>(other).m_value;
}

void ConstantExpression::dumpImpl(std::ostream& os) const
{
    m_value.dump(os);
}

}  // namespace siodb::iomgr::dbengine::requests
