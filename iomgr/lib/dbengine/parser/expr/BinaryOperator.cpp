// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

BinaryOperator::BinaryOperator(
        ExpressionType type, ExpressionPtr&& left, ExpressionPtr&& right) noexcept
    : Expression(type)
    , m_left(std::move(left))
    , m_right(std::move(right))

{
}

bool BinaryOperator::isBinaryOperator() const noexcept
{
    return true;
}

void BinaryOperator::validate(const Context& context) const
{
    m_left->validate(context);
    m_right->validate(context);
}

std::size_t BinaryOperator::getSerializedSize() const noexcept
{
    return getExpressionTypeSerializedSize(m_type) + m_left->getSerializedSize()
           + m_right->getSerializedSize();
}

std::uint8_t* BinaryOperator::serializeUnchecked(std::uint8_t* buffer) const
{
    buffer = serializeExpressionTypeUnchecked(m_type, buffer);
    buffer = m_left->serializeUnchecked(buffer);
    return m_right->serializeUnchecked(buffer);
}

// ----- internals -----

bool BinaryOperator::isEqualTo(const Expression& other) const noexcept
{
    const auto& otherBinaryOperator = static_cast<const BinaryOperator&>(other);
    return *m_left == *otherBinaryOperator.m_left && *m_right == *otherBinaryOperator.m_right;
}

void BinaryOperator::dumpImpl(std::ostream& os) const
{
    os << " left:" << *m_left << " right:" << *m_right;
}

}  // namespace siodb::iomgr::dbengine::requests
