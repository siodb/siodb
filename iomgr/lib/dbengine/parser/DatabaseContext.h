// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "expr/Expression.h"
#include "../DataSet.h"

namespace siodb::iomgr::dbengine::requests {

/**
 * Context used for expression evaluation
 */
class DatabaseContext final : public Expression::Context {
public:
    /**
     * Initializes object of class DatabaseContext
     * @param dataSets Data sets.
     */
    explicit DatabaseContext(std::vector<DataSetPtr>&& dataSets)
        : m_dataSets(std::move(dataSets))
        , m_nameToIndexMapping(makeNameToIndexMapping())
    {
    }

    /**
     * Returns collection of data sets.
     * @return Collections of data sets.
     */
    const auto& getDataSets() const noexcept
    {
        return m_dataSets;
    }

    /**
     * Retuns data set index.
     * @param name Data set name.
     * @return Data set index if data set exists, empty value otherwise.
     */
    std::optional<std::size_t> getDataSetIndex(const std::string& name) const noexcept
    {
        const auto it = m_nameToIndexMapping.find(name);
        if (it != m_nameToIndexMapping.cend()) return it->second;
        return {};
    }

    /**
     * Returns column value.
     * @param tableIndex Table index.
     * @param columnIndex Column index.
     * @return Value of the specified column.
     * @throw std::out_of_range if table or column index is greater than or equal
     * to actual number of tables or columns
     * @throw std::runtime_error If no more rows is available.
     */
    const Variant& getColumnValue(std::size_t tableIndex, const std::size_t columnIndex) override;

    /**
     * Returns column data type
     * @param tableIndex Table index.
     * @param columnIndex Column index.
     * @return Column data type.
     */
    ColumnDataType getColumnDataType(
            std::size_t tableIndex, std::size_t columnIndex) const override;

private:
    /** Name to index mapping type */
    using NameToIndexMapping = std::unordered_map<std::reference_wrapper<const std::string>,
            std::size_t, std::hash<std::string>, std::equal_to<std::string>>;

private:
    /**
     * Generates name to index mapping.
     * @return Name to index mapping.
     */
    NameToIndexMapping makeNameToIndexMapping() const;

private:
    /** Data sets */
    const std::vector<DataSetPtr> m_dataSets;

    /** Name to index mapping */
    const NameToIndexMapping m_nameToIndexMapping;
};

}  // namespace siodb::iomgr::dbengine::requests
