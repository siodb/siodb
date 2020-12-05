// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UnaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

UnaryOperator::UnaryOperator(ExpressionType type, ExpressionPtr&& operand) noexcept
    : Expression(type)
    , m_operand(std::move(operand))
{
}

bool UnaryOperator::isUnaryOperator() const noexcept
{
    return true;
}

void UnaryOperator::validate(const ExpressionEvaluationContext& context) const
{
    m_operand->validate(context);
}

std::size_t UnaryOperator::getSerializedSize() const noexcept
{
    return getExpressionTypeSerializedSize(m_type) + m_operand->getSerializedSize();
}

std::uint8_t* UnaryOperator::serializeUnchecked(std::uint8_t* buffer) const
{
    buffer = serializeExpressionTypeUnchecked(m_type, buffer);
    return m_operand->serializeUnchecked(buffer);
}

// ----- internals -----

bool UnaryOperator::isEqualTo(const Expression& other) const noexcept
{
    return *m_operand == *static_cast<const UnaryOperator&>(other).m_operand;
}

void UnaryOperator::dumpImpl(std::ostream& os) const
{
    os << " op: " << *m_operand;
}

}  // namespace siodb::iomgr::dbengine::requests