// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IsOperator.h"

namespace siodb::iomgr::dbengine::requests {

MutableOrConstantString IsOperator::getExpressionText() const
{
    return m_isNot ? "IS NOT" : "IS";
}

std::size_t IsOperator::getSerializedSize() const noexcept
{
    return ComparisonBinaryOperator::getSerializedSize() + 1;
}

Variant IsOperator::evaluate(Context& context) const
{
    const auto leftValue = m_left->evaluate(context);
    const auto rightValue = m_right->evaluate(context);

    if (leftValue.isNull() || rightValue.isNull())
        return m_isNot != (leftValue.isNull() == rightValue.isNull());

    return leftValue.compatibleEqual(m_right->evaluate(context)) != m_isNot;
}

std::uint8_t* IsOperator::serializeUnchecked(std::uint8_t* buffer) const
{
    buffer = ComparisonBinaryOperator::serializeUnchecked(buffer);
    *buffer = m_isNot ? 1 : 0;
    return buffer + 1;
}

Expression* IsOperator::clone() const
{
    ExpressionPtr left(m_left->clone()), right(m_right->clone());
    return new IsOperator(std::move(left), std::move(right), m_isNot);
}

// ----- internals -----

bool IsOperator::isEqualTo(const Expression& other) const noexcept
{
    const auto& otherIsOperator = static_cast<const IsOperator&>(other);
    return m_isNot == otherIsOperator.m_isNot && *m_left == *otherIsOperator.m_left
           && *m_right == *otherIsOperator.m_right;
}

}  // namespace siodb::iomgr::dbengine::requests
