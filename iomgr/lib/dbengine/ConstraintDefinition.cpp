// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ConstraintDefinition.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

ConstraintDefinition::ConstraintDefinition(bool system, Database& database, ConstraintType type,
        requests::ConstExpressionPtr&& expression)
    : m_database(database)
    , m_id(m_database.generateNextConstraintDefinitionId(system))
    , m_type(type)
    , m_expression(std::move(expression))
    , m_hash(computeHash())
    , m_writtenToStorage(false)
{
}

ConstraintDefinition::ConstraintDefinition(
        Database& database, const ConstraintDefinitionRecord& constraintDefinitionRecord)
    : m_database(database)
    , m_id(constraintDefinitionRecord.m_id)
    , m_type(constraintDefinitionRecord.m_type)
    , m_expression(deserializeExpression(constraintDefinitionRecord.m_expression))
    , m_hash(constraintDefinitionRecord.m_hash)
    , m_writtenToStorage(true)
{
}

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

requests::ExpressionPtr ConstraintDefinition::deserializeExpression(
        const BinaryValue& expressionBinary)
{
    requests::ExpressionPtr result;
    requests::Expression::deserialize(expressionBinary.data(), expressionBinary.size(), result);
    return result;
}

}  // namespace siodb::iomgr::dbengine
