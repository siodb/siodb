// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Column.h"
#include "DataSet.h"
#include "Table.h"

// Common project headers
#include <siodb/common/utils/HelperMacros.h>

// STL headers
#include <bitset>

namespace siodb::iomgr::dbengine {

/**
 * Class for reading rows from table
 */
class TableDataSet final : public DataSet {
public:
    /**
     * Initializes object of class TableDataSet.
     * @param table Table object.
     * @param tableAlias Table alias.
     */
    TableDataSet(const TablePtr& table, const std::string& tableAlias);

    /**
     * Returns table object.
     * @return Table object.
     */
    Table& getTable() const noexcept
    {
        return *m_table;
    }

    /**
     * Returns collection of column objects.
     * @return Collection of column objects.
     */
    const auto& getColumns() const noexcept
    {
        return m_tableColumns;
    }

    /**
     * Returns data set name.
     * @return Data set name.
     */
    const std::string& getName() const noexcept override;

    /**
     * Returns current master column record.
     * @return Current master column record.
     */
    const auto& getCurrentMcr() const noexcept
    {
        return m_currentMcr;
    }

    /**
     * Returns column value from current row.
     * @return Column value.
     * @throw std::runtime_error If no more rows is available.
     * @throw std::out_of_range if table or column index is greater than or equal
     * to actual number of columns.
     * @throw std::runtime_error If data was not read before.
     */
    const Variant& getColumnValue(std::size_t columnIndex) override;

    /**
     * Returns column data type
     * @param columnIndex Column index.
     * @return Column data type.
     */
    ColumnDataType getColumnDataType(std::size_t columnIndex) const override;

    /**
     * Returns current row. Reads current row data if it was not read before.
     * @return Current row.
     * @throw std::runtime_error in case if row data is not avaliable.
     */
    const std::vector<Variant>& getCurrentRow() override;

    /**
     * Returns column position in the data source. Queries data source directly.
     * @param name Column name.
     * @return Column position in the data source.
     * @throw std::out_of_range if index is out of range.
     */
    std::optional<std::uint32_t> getDataSourceColumnPosition(
            const std::string& name) const override;

    /**
     * Returns data source ID.
     * @return Underlying data source ID.
     */
    std::uint32_t getDataSourceId() const noexcept override;

    /** Reset cursor position to the first row. */
    void resetCursor() override;

    /**
     * Moves dataset to the next row
     * @return true if row data available for reading, false otherwise
     */
    bool moveToNextRow() override;

    /**
     * Deletes current row.
     * @param currentUserId Current user ID.
     * @throw DatabaseError in case of database error.
     */
    void deleteCurrentRow(std::uint32_t currentUserId);

    /**
     * Updates dataset's current row
     * @param values New values.
     * @param columnPositions Positions of columns from the table, count must be equal to
     *                        values count.
     * @param currentUserId Current user ID.
     * @throw DatabaseError in case of database error.
     */
    void updateCurrentRow(std::vector<Variant>&& values,
            const std::vector<std::size_t>& columnPositions, std::uint32_t currentUserId);

private:
    /** Reads master column record of the current row. */
    void readMasterColumnRecord();

    /**
     * Reads value of the column.
     * @param index Column Index.
     */
    void readColumnValue(std::size_t index);

private:
    /** Table object */
    const TablePtr m_table;

    /** Table columns ordered by position */
    std::vector<ColumnPtr> m_tableColumns;

    /* Master column from m_table */
    ColumnPtr m_masterColumn;

    /* Index of master column */
    IndexPtr m_masterColumnIndex;

    /** Index key buffer */
    std::uint8_t m_key[16];

    /** Current master column record */
    MasterColumnRecord m_currentMcr;

    /** Current master column record address */
    ColumnDataAddress m_currentMcrAddress;

    /** Current row key from index */
    std::uint8_t* m_currentKey;

    /** Next row key from index */
    std::uint8_t* m_nextKey;
};

}  // namespace siodb::iomgr::dbengine
