// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Column.h"
#include "ColumnDefinitionConstraintListPtr.h"
#include "ConstraintPtr.h"

namespace siodb::iomgr::dbengine {

class ColumnDefinitionConstraint;
class ColumnDefinitionConstraintList;

/** Set of conditions that completely define table column. */
class ColumnDefinition {
public:
    /**
     * Initializes object of class ColumnDefinition.
     * @param column A column.
     */
    explicit ColumnDefinition(Column& column);

    /**
     * Initializes object of class ColumnDefinition for existing column definition.
     * @param column A column to which this column definition belongs.
     * @param columnDefinitionRecord Source column definition registry record.
     */
    ColumnDefinition(Column& column, const ColumnDefinitionRecord& columnDefinitionRecord);

    /**
     * Returns database object.
     * @return Database object.
     */
    Database& getDatabase() const noexcept
    {
        return m_column.getDatabase();
    }

    /**
     * Returns database UUID.
     * @return Database UUID.
     */
    const Uuid& getDatabaseUuid() const noexcept
    {
        return m_column.getDatabaseUuid();
    }

    /**
     * Returns database name.
     * @return Database name.
     */
    const std::string& getDatabaseName() const noexcept
    {
        return m_column.getDatabaseName();
    }

    /**
     * Returns table object.
     * @return Table object.
     */
    Table& getTable() const noexcept
    {
        return m_column.getTable();
    }

    /**
     * Returns table ID.
     * @return Table ID.
     */
    std::uint32_t getTableId() const noexcept
    {
        return m_column.getTableId();
    }

    /**
     * Returns table name.
     * @return Table name.
     */
    const std::string& getTableName() const noexcept
    {
        return m_column.getTableName();
    }

    /**
     * Returns column object.
     * @return Column object.
     */
    Column& getColumn() const noexcept
    {
        return m_column;
    }

    /**
     * Returns column ID.
     * @return Column ID.
     */
    std::uint64_t getColumnId() const noexcept
    {
        return m_column.getId();
    }

    /**
     * Returns column name.
     * @return Column name.
     */
    const std::string& getColumnName() const noexcept
    {
        return m_column.getName();
    }

    /**
     * Return column definition ID.
     * @return Column definition ID.
     */
    std::uint64_t getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns list of attached constraints.
     * Returns valid list only if at least one constraint is specified.
     * Use hasConstraints() to check that.
     * @return List of constraints.
     */
    const auto& getConstraints() const noexcept
    {
        return *m_constraints;
    }

    /**
     * Returns indication that column definition is open for modification.
     * @return true if column definition is open for modification, false otherwise.
     */
    bool isOpenForModification() const noexcept
    {
        return m_openForModification;
    }

    /**
     * Returns indication that this column definition doesn't allow NOT NULL values.
     * @return true, if current column definition doesn't allow NULL values, false otherwise.
     */
    bool isNotNull() const noexcept;

    /**
     * Returns default value provided by this column definition.
     * @return Default value for the column or value of type VariantType::kNull,
     *         if default value is not specified.
     */
    Variant getDefaultValue() const;

    /** Marks column definition as closed for modification. */
    void markClosedForModification();

    /**
     * Adds new constraint to this column definition.
     * @param constraint A constraint.
     */
    std::uint64_t addConstraint(const ConstraintPtr& constraint);

private:
    /**
     * Validates column.
     * @param column Column to which this column definition is supposed to belong to.
     * @param columnDefinitionRecord Column definition registry record.
     * @return The same column, if it is valid.
     * @throw DatabaseError if column has different ID.
     */
    static Column& validateColumn(
            Column& column, const ColumnDefinitionRecord& columnDefinitionRecord);

    /**
     * Creates new empty list of column definition constraints.
     * @return New empty list of column definition constraints.
     */
    static std::shared_ptr<ColumnDefinitionConstraintList> createEmptyConstraints();

    /**
     * Creates list of column definition constraints from a column definition record.
     * @param columnDefinitionRecord Column definition record.
     * @return List of column definition constraints.
     */
    ColumnDefinitionConstraintListPtr createConstraints(
            const ColumnDefinitionRecord& columnDefinitionRecord);

private:
    /** Column object */
    Column& m_column;

    /** Column definition ID */
    const std::uint64_t m_id;

    /** List of constraints */
    ColumnDefinitionConstraintListPtr m_constraints;

    /** Indicates that column definition is open for modification. */
    bool m_openForModification;
};

}  // namespace siodb::iomgr::dbengine
