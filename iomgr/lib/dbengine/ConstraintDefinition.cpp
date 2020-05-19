// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConstraintDefinition.h"

namespace siodb::iomgr::dbengine {

BinaryValue ConstraintDefinition::serializeExpression() const
{
    BinaryValue result;
    if (m_expression) {
        result.resize(m_expression->getSerializedSize());
        m_expression->serializeUnchecked(result.data());
    }
    return result;
}

// ----- internals ------

std::uint64_t ConstraintDefinition::computeHash() const
{
    const auto expressionBinary = serializeExpression();
    return ConstraintDefinitionRecord::computeHash(m_type, expressionBinary);
}

requests::ExpressionPtr ConstraintDefinition::decodeExpression(const BinaryValue& expressionBinary)
{
    requests::ExpressionPtr result;
    requests::Expression::deserialize(expressionBinary.data(), expressionBinary.size(), result);
    return result;
}

}  // namespace siodb::iomgr::dbengine
