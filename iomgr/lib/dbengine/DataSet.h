// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDataType.h"
#include "DataSetPtr.h"
#include "Variant.h"

// Common project headers
#include <siodb/common/utils/Bitmask.h>

// STL headers
#include <optional>

namespace siodb::iomgr::dbengine {

/** Column information for dataset */
struct DataSetColumnInfo {
    /**
     * Initializes object of class DataSetColumnInfo.
     * @param pos Position of column in table.
     * @param name Column name.
     * @param alias Column alias.
     */
    DataSetColumnInfo(std::size_t pos, const std::string& name, const std::string& alias)
        : m_posInTable(pos)
        , m_name(std::make_unique<std::string>(name))
        , m_alias(std::make_unique<std::string>(alias))
    {
    }

    /** Position of the column in the table */
    const std::size_t m_posInTable;

    /** Column name */
    std::unique_ptr<std::string> m_name;

    /** Column alias */
    std::unique_ptr<std::string> m_alias;
};

/**
 * A base class for all data sets.
 */
class DataSet {
protected:
    /** 
     * Initializes object of class DataSet.
     * @param alias Data set alias.
     */
    DataSet(const std::string& alias)
        : m_alias(alias)
        , m_hasCurrentRow(false)
    {
    }

public:
    /** De-initializes object of class DataSet. */
    virtual ~DataSet() = default;

    /**
     * Returns data set name.
     * @return Data set name.
     */
    virtual const std::string& getName() const noexcept = 0;

    /**
     * Returns data set alias.
     * @return Data set alias.
     */
    const std::string& getAlias() const noexcept
    {
        return m_alias;
    }

    /**
     * Returns indication that row data is available for reading.
     * @return true if row data available for reading, false otherwise.
     */
    bool hasCurrentRow() const noexcept
    {
        return m_hasCurrentRow;
    }

    /** Reset cursor position to the first row. */
    virtual void resetCursor() = 0;

    /**
     * Moves dataset to the next row.
     * @return true if row data available for reading, false otherwise.
     */
    virtual bool moveToNextRow() = 0;

    /**
     * Returns current row. Reads current row data if it was not read before.
     * @return Current row.
     * @throw std::runtime_error if row data is not avaliable.
     */
    virtual const std::vector<Variant>& getCurrentRow() = 0;

    /**
     * Returns column value from the current row. Data can be read from an underlying source,
     * if it was not read before.
     * @param index Index of column in dataset.
     * @return Column value.
     * @throw std::out_of_range if index is out of range.
     */
    virtual const Variant& getColumnValue(std::size_t index) = 0;

    /**
     * Returns column data type.
     * @param index Column index.
     * @return Column data type.
     * @throw std::out_of_range if index is out of range.
     */
    virtual ColumnDataType getColumnDataType(std::size_t index) const = 0;

    /**
     * Returns cached column position in the data source.
     * @param index Column index.
     * @return Column position in the data source.
     * @throw std::out_of_range if index is out of range.
     */
    std::size_t getColumnPosition(std::size_t index) const
    {
        return m_columnInfos.at(index).m_posInTable;
    }

    /**
     * Returns column name.
     * @param index Column index.
     * @return Column name.
     * @throw std::out_of_range if index is out of range.
     */
    const std::string& getColumnName(std::size_t index) const
    {
        return *m_columnInfos.at(index).m_name;
    }

    /**
     * Returns column name.
     * @param index Column index.
     * @return Column alias or empty string if there is no alias.
     * @throw std::out_of_range if index is out of range.
     */
    const std::string& getColumnAlias(std::size_t index) const
    {
        return *m_columnInfos.at(index).m_alias;
    }

    /**
     * Returns column count in the row set.
     * @return Column count.
     */
    std::size_t getColumnCount() const noexcept
    {
        return m_columnInfos.size();
    }

    /**
     * Searhes column index from column name.
     * @param name Column name.
     * @return Index of column in dataset or empty value if column does not exist.
     */
    std::optional<std::size_t> getColumnIndex(const std::string& name) const noexcept;

    /**
     * Returns column position in the data source. Queries data source directly.
     * @param name Column name.
     * @return Column position in the data source.
     * @throw std::out_of_range if index is out of range.
     */
    virtual std::optional<std::uint32_t> getDataSourceColumnPosition(
            const std::string& name) const = 0;

    /**
     * Returns data source ID.
     * @return Underlying data source ID.
     */
    virtual std::uint32_t getDataSourceId() const noexcept = 0;

    /**
     * Emplaces DataSetColumnInfo into this data set meta-information.
     * @param args Arguments for DataSetColumnInfo constructor.
     */
    template<typename... Args>
    void emplaceColumnInfo(Args&&... args)
    {
        m_columnInfos.emplace_back(std::forward<Args>(args)...);
    }

protected:
    /** Data set alias */
    const std::string m_alias;

    /** Column infos */
    std::vector<DataSetColumnInfo> m_columnInfos;

    /** Current row values */
    std::vector<Variant> m_values;

    /** Indicates which values are already read */
    utils::Bitmask m_valueReadMask;

    /** Indication that row data is avaliable for reading. */
    bool m_hasCurrentRow;
};

}  // namespace siodb::iomgr::dbengine
