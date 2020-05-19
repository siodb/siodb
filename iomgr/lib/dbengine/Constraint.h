// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ConstraintDefinition.h"
#include "ConstraintPtr.h"
#include "Table.h"

// Common project headers
#include <siodb/common/utils/BinaryValue.h>

namespace siodb::iomgr::dbengine {

/** Base class for all constraint classes. */
class Constraint {
protected:
    /**
     * Initializes object of class Constraint for the new constraint.
     * @param table Table to which this constraint belongs to.
     * @param name Constraint name.
     * @param constraintDefinition Constraint definition.
     */
    Constraint(Table& table, const std::string& name,
            const ConstConstraintDefinitionPtr& constraintDefinition);

    /**
     * Initializes object of class Constraint for the existing constraint.
     * @param table Table to which this constraint belongs to.
     * @param constraintRecord Constraint registry record.
     */
    Constraint(Table& table, const ConstraintRecord& constraintRecord);

public:
    /** De-initializes obect of class Constraint. */
    virtual ~Constraint() = default;

    /**
     * Returns database object.
     * @return Database object.
     */
    Database& getDatabase() const noexcept
    {
        return m_table.getDatabase();
    }

    /**
     * Returns database UUID.
     * @return Database UUID.
     */
    const Uuid& getDatabaseUuid() const noexcept
    {
        return m_table.getDatabaseUuid();
    }

    /**
     * Returns database name.
     * @return Database name.
     */
    const std::string& getDatabaseName() const noexcept
    {
        return m_table.getDatabaseName();
    }

    /**
     * Returns table object.
     * @return Table object.
     */
    Table& getTable() const noexcept
    {
        return m_table;
    }

    /**
     * Returns table ID.
     * @return Table ID.
     */
    std::uint32_t getTableId() const noexcept
    {
        return m_table.getId();
    }

    /**
     * Returns table name.
     * @return Table name.
     */
    const std::string& getTableName() const noexcept
    {
        return m_table.getName();
    }

    /**
     * Returns column object to which this constaint applies.
     * @return Column object or nullptr if this is table constraint.
     */
    virtual Column* getColumn() const noexcept;

    /**
     * Returns constraint ID.
     * @return Constraint ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns constraint state.
     * @return Constraint state.
     */
    auto getState() const noexcept
    {
        return m_state;
    }

    /**
     * Returns constraint name.
     * @return Constraint name.
     */
    const std::string& getName() const noexcept
    {
        return m_name;
    }

    /**
     * Returns constraint type.
     * @return Constraint type.
     */
    ConstraintType getType() const noexcept
    {
        return m_constraintDefinition->getType();
    }

    /**
     * Returns indication that expression is present.
     * @return true if this constraint definition has expression, false otherwise.
     */
    bool hasExpression() const noexcept
    {
        return m_constraintDefinition->hasExpression();
    }

    /**
     * Returns constraint definition expression.
     * Only makes sense if expression is present.
     * @return Constraint definition expression.
     */
    const auto& getExpression() const noexcept
    {
        return m_constraintDefinition->getExpression();
    }

    /**
     * Returns constraint definition ID.
     * @return Constraint definition ID.
     */
    std::size_t getDefinitionId() const noexcept
    {
        return m_constraintDefinition->getId();
    }

    /**
     * Returns constraint definition hash.
     * @return Constraint definition hash.
     */
    std::size_t getDefinitionHash() const noexcept
    {
        return m_constraintDefinition->getHash();
    }

    /**
     * Returns constraint definition.
     * @return Constraint definition.
     */
    const ConstraintDefinition& getDefinition() const noexcept
    {
        return *m_constraintDefinition;
    }

    /**
     * Returns indication that this is system constraint.
     * @return true if this is system constraint, false otherwise.
     */
    bool isSystemConstraint() const noexcept
    {
        return m_id < kFirstUserTableConstraintId;
    }

protected:
    /**
     * Checks that constraint type matches to the required one.
     * @param table Table, to which the constraint belongs.
     * @param constaintName Constraint name.
     * @param constraintDefinition Constraint definition.
     * @param expectedType Expected constraint type.
     * @return Same constraint definition if constraint type matches required.
     * @throw DatabaseError if constraint type doesn't match.
     */
    static const ConstraintDefinitionPtr& checkConstraintType(const Table& table,
            const std::string& constaintName, const ConstraintDefinitionPtr& constraintDefinition,
            ConstraintType expectedType);

    /**
     * Checks that constraint type matches to the required one.
     * @param table Table, to which the constraint belongs.
     * @param constraintRecord Constraint record.
     * @param expectedType Expected constraint type.
     * @return Same constraint record if constraint type matches required.
     * @throw DatabaseError if constraint type doesn't match.
     */
    static const ConstraintRecord& checkConstraintType(const Table& table,
            const ConstraintRecord& constraintRecord, ConstraintType expectedType);

private:
    /**
     * Validates constraint table.
     * @param table Table to which constaint is supposed to belong to.
     * @param constraintRecord Constraint record.
     * @return The same table, if it is valid.
     * @throw DatabaseError if table has different table ID.
     */
    static Table& validateTable(Table& table, const ConstraintRecord& constraintRecord);

    /**
     * Validates constraint name.
     * @param constraintName Constraint name.
     * @return The same constraint name, if it is valid.
     * @throw DatabaseError if constraint name is invalid.
     */
    const std::string& validateConstraintName(const std::string& constraintName) const;

    /** 
     * Generates new supposedly unique constraint name.
     * @return New constraint name.
     */
    std::string generateConstraintName() const;

protected:
    /** Table which this constraint belongs to */
    Table& m_table;

    /** Constraint name */
    std::string m_name;

    /** Constraint ID */
    const std::uint64_t m_id;

    /** Constraint state */
    ConstraintState m_state;

    /** Constraint definition */
    ConstConstraintDefinitionPtr m_constraintDefinition;
};

}  // namespace siodb::iomgr::dbengine
