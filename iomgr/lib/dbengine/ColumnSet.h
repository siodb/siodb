// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnSetColumnPtr.h"
#include "Table.h"

namespace siodb::iomgr::dbengine {

class ColumnSetRecord;

/** Set of columns that defines table */
class ColumnSet {
public:
    /** List of columns */
    using Columns = std::vector<ConstColumnSetColumnPtr>;

public:
    /**
     * Initializes object of class ColumnSet for new column set.
     * @param table A table to which this column set belongs.
     * @param columns List of columns.
     */
    ColumnSet(Table& table, Columns&& columns = Columns());

    /**
     * Initializes object of class ColumnSet for existing column set.
     * @param table A table to which this column set belongs.
     * @param columnSetRecord Source column set registry record.
     */
    ColumnSet(Table& table, const ColumnSetRecord& columnSetRecord);

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
     * Returns column set identifier.
     * @return Column set identifier.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns collection of columns.
     * @return Collection of columns.
     */
    const Columns& getColumns() const noexcept
    {
        return m_columns;
    }

    /**
     * Returns indication that column set is open for modification.
     * @return true if column set is open for modification, false otherwise.
     */
    bool isOpenForModification() const noexcept
    {
        return m_openForModification;
    }

    /** 
     * Marks column set as closed for modification. Updates column postion cache.
     * @throw DatabaseError if column set is already closed for modification.
     */
    void markClosedForModification();

    /**
     * Adds new column definition to this column set.
     * @param columnDefinition Column definition.
     * @return Record ID.
     */
    std::uint64_t addColumn(const ColumnDefinition& columnDefinition);

    /**
     * Return column position in the column set.
     * @param columnId Column ID.
     * @return Column position.
     * @throw DatabaseError if column does not belong to this ColumnSet.
     */
    std::uint32_t findColumnPosition(std::uint64_t columnId) const;

private:
    /** Column ID to position in the column set mapping type. */
    using ColumnIdToPoisitionMapping = std::unordered_map<std::uint64_t, std::uint32_t>;

private:
    /**
     * Validates column set table.
     * @param table Table to which this column set is supposed to belong to.
     * @param columnSetRecord Column set record.
     * @return The same table, if it is valid.
     * @throw DatabaseError if table has different table ID.
     */
    static Table& validateTable(Table& table, const ColumnSetRecord& columnSetRecord);

    /**
     * Creates list of columns from a column set record.
     * @param columnSetRecord Column set record.
     * @return List of column set columns.
     */
    Columns makeColumns(const ColumnSetRecord& columnSetRecord);

    /**
     * Creates mapping of column ID to position.
     * @return Mapping object.
     */
    ColumnIdToPoisitionMapping createColumIdToPoisitionMapping() const;

private:
    /** Table to which this column set belongs. */
    Table& m_table;

    /** Column set ID */
    const std::uint64_t m_id;

    /** List of column definitions */
    Columns m_columns;

    /** Mapping of column ID to position */
    ColumnIdToPoisitionMapping m_columnIdToPoisitionMapping;

    /** Indicates that column set is open for modification. */
    bool m_openForModification;
};

}  // namespace siodb::iomgr::dbengine
