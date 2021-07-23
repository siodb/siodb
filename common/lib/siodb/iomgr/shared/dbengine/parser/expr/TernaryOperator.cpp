// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TernaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

TernaryOperator::TernaryOperator(ExpressionType type, ExpressionPtr&& left, ExpressionPtr&& middle,
        ExpressionPtr&& right) noexcept
    : Expression(type)
    , m_left(std::move(left))
    , m_middle(std::move(middle))
    , m_right(std::move(right))
{
}

bool TernaryOperator::isTernaryOperator() const noexcept
{
    return true;
}

std::size_t TernaryOperator::getSerializedSize() const noexcept
{
    return getExpressionTypeSerializedSize(m_type) + m_left->getSerializedSize()
           + m_middle->getSerializedSize() + m_right->getSerializedSize();
}

void TernaryOperator::validate(const ExpressionEvaluationContext& context) const
{
    m_left->validate(context);
    m_middle->validate(context);
    m_right->validate(context);
}

std::uint8_t* TernaryOperator::serializeUnchecked(std::uint8_t* buffer) const
{
    buffer = serializeExpressionTypeUnchecked(m_type, buffer);
    buffer = m_left->serializeUnchecked(buffer);
    buffer = m_middle->serializeUnchecked(buffer);
    return m_right->serializeUnchecked(buffer);
}

// --- internals ---

bool TernaryOperator::isEqualTo(const Expression& other) const noexcept
{
    const auto& otherTernaryOperator = static_cast<const TernaryOperator&>(other);
    return *m_left == *otherTernaryOperator.m_left && *m_middle == *otherTernaryOperator.m_middle
           && *m_right == *otherTernaryOperator.m_right;
}

void TernaryOperator::dumpImpl(std::ostream& os) const
{
    os << " left:" << *m_left << " middle:" << *m_middle << " right:" << *m_right;
}

}  // namespace siodb::iomgr::dbengine::requests
