// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BetweenOperator.h"

namespace siodb::iomgr::dbengine::requests {

VariantType BetweenOperator::getResultValueType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    return VariantType::kBool;
}

ColumnDataType BetweenOperator::getColumnDataType(
        [[maybe_unused]] const ExpressionEvaluationContext& context) const
{
    return COLUMN_DATA_TYPE_BOOL;
}

MutableOrConstantString BetweenOperator::getExpressionText() const
{
    return m_notBetween ? "NOT BETWEEN" : "BETWEEN";
}

std::size_t BetweenOperator::getSerializedSize() const noexcept
{
    return TernaryOperator::getSerializedSize() + 1;
}

void BetweenOperator::validate(const ExpressionEvaluationContext& context) const
{
    m_left->validate(context);
    m_right->validate(context);

    const auto leftResultType = m_left->getResultValueType(context);
    const auto middleResultType = m_middle->getResultValueType(context);
    const auto rightResultType = m_right->getResultValueType(context);

    if ((isNullType(leftResultType) || isNumericType(leftResultType))
            && (isNullType(middleResultType) || isNumericType(middleResultType))
            && (isNullType(rightResultType) || isNumericType(rightResultType))) {
        return;
    }

    if ((isNullType(leftResultType) || isDateTimeType(leftResultType)
                || m_left->canCastAsDateTime(context))
            && (isNullType(middleResultType) || isDateTimeType(middleResultType)
                    || m_middle->canCastAsDateTime(context))
            && (isNullType(rightResultType) || isDateTimeType(rightResultType)
                    || m_right->canCastAsDateTime(context))) {
        return;
    }

    throw std::runtime_error("BETWEEN operands aren't dates or numeric");
}

Variant BetweenOperator::evaluate(ExpressionEvaluationContext& context) const
{
    const auto value = m_left->evaluate(context);
    const auto lowerBound = m_middle->evaluate(context);
    const auto upperBound = m_right->evaluate(context);

    if (value.isNull() || lowerBound.isNull() || upperBound.isNull()) {
        // TODO: SIODB-172
        return false;
    }

    if (!value.isNumeric() && !(value.isString() || value.isDateTime()))
        throw std::runtime_error("Expression value type isn't compatible with BETWEEN operator");

    const bool valueIsBetween =
            lowerBound.compatibleLessOrEqual(value) && upperBound.compatibleGreaterOrEqual(value);
    return valueIsBetween != m_notBetween;
}

std::uint8_t* BetweenOperator::serializeUnchecked(std::uint8_t* buffer) const
{
    buffer = TernaryOperator::serializeUnchecked(buffer);
    *buffer = m_notBetween ? 1 : 0;
    return buffer + 1;
}

Expression* BetweenOperator::clone() const
{
    ExpressionPtr left(m_left->clone()), middle(m_middle->clone()), right(m_right->clone());
    return new BetweenOperator(std::move(left), std::move(middle), std::move(right), m_notBetween);
}

// --- internals ---

bool BetweenOperator::isEqualTo(const Expression& other) const noexcept
{
    const auto& otherBetweenOperator = static_cast<const BetweenOperator&>(other);
    return m_notBetween == otherBetweenOperator.m_notBetween
           && *m_left == *otherBetweenOperator.m_left && *m_middle == *otherBetweenOperator.m_middle
           && *m_right == *otherBetweenOperator.m_right;
}

}  // namespace siodb::iomgr::dbengine::requests
