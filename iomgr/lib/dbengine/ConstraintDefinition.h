// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Database.h"
#include "parser/expr/Expression.h"

namespace siodb::iomgr::dbengine {

/** Constraint definition. */
class ConstraintDefinition {
public:
    /**
     * Initializes object of class Constraint for the new constraint definition.
     * @param system Indicates that constraint definition ID must belong to system range.
     * @param database Database that this constraint definition belongs to.
     * @param id Constraint definition ID.
     * @param type Constraint type.
     * @param expression Constraint expression.
     */
    ConstraintDefinition(bool system, Database& database, ConstraintType type,
            requests::ConstExpressionPtr&& expression)
        : m_database(database)
        , m_id(m_database.generateNextConstraintDefinitionId(system))
        , m_type(type)
        , m_expression(std::move(expression))
        , m_hash(computeHash())
        , m_writtenToStorage(false)
    {
    }

    /**
     * Initializes object of class ConstraintDefinition for the existing constraint.
     * @param database Database that this constraint definition belongs to.
     * @param constraintDefinitionRecord Constraint definition registry record.
     */
    ConstraintDefinition(
            Database& database, const ConstraintDefinitionRecord& constraintDefinitionRecord)
        : m_database(database)
        , m_id(constraintDefinitionRecord.m_id)
        , m_type(constraintDefinitionRecord.m_type)
        , m_expression(decodeExpression(constraintDefinitionRecord.m_expression))
        , m_hash(constraintDefinitionRecord.m_hash)
        , m_writtenToStorage(true)
    {
    }

    /**
     * Returns database object.
     * @return Database object.
     */
    Database& getDatabase() const
    {
        return m_database;
    }

    /**
     * Returns constraint definition ID.
     * @return Constraint ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns constraint type.
     * @return Constraint type.
     */
    auto getType() const noexcept
    {
        return m_type;
    }

    /**
     * Returns indication that constraint definition has expression.
     * @return Constraint expression.
     */
    bool hasExpression() const noexcept
    {
        return static_cast<bool>(m_expression);
    }

    /**
     * Returns constraint expression.
     * @return Constraint expression.
     */
    const auto& getExpression() const noexcept
    {
        return *m_expression;
    }

    /**
     * Returns constraint definition hash.
     * @return Constraint definition hash.
     */
    std::size_t getHash() const noexcept
    {
        return m_hash;
    }

    /**
     * Returns indication that this is system constraint definition.
     * @return true if this is system constraint definition, false otherwise.
     */
    bool isSystemConstraintDefinition() const noexcept
    {
        return m_id < kFirstUserTableConstraintDefinitionId;
    }

    /**
     * Returns indication that this constraint definition is written to storage.
     * @return true if this constraint definition is written to storage, false otherwise.
     */
    bool isWrittenToStorage() const noexcept
    {
        return m_writtenToStorage;
    }

    /** Sets indication that this constraint definition is written to storage. */
    void setWrittenToStorage() noexcept
    {
        m_writtenToStorage = true;
    }

    /**
     * Returns serialized expression in the binary format.
     * @return Serialized expression in the binary format.
     */
    BinaryValue serializeExpression() const;

private:
    /**
     * Computes hash of this constraint definition.
     * @return Hash value.
     */
    std::uint64_t computeHash() const;

    /**
     * Decodes constraint definition expression taking into account constraint type.
     * @param expressionBinary Serialized expression in the binary format.
     * @return Expression object.
     */
    static requests::ExpressionPtr decodeExpression(const BinaryValue& expressionBinary);

private:
    /** Database that this constaint definition belongs to */
    Database& m_database;

    /** Constraint definition ID */
    const std::uint64_t m_id;

    /** Constraint type */
    const ConstraintType m_type;

    /** Constraint expression. */
    const requests::ConstExpressionPtr m_expression;

    /** Hash value of this column definition. Must be last member variable. */
    const std::uint64_t m_hash;

    /** Indication that constraint definition written to storage */
    mutable bool m_writtenToStorage;
};

}  // namespace siodb::iomgr::dbengine
