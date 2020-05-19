// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDefinition.h"
#include "ColumnSet.h"

namespace siodb::iomgr::dbengine {

/** Column set associated column record */
class ColumnSetColumn {
public:
    /**
     * Initializes object of class ColumnSetColumn.
     * @param columnSet Column set.
     * @param columnDefinition A column definition.
     */
    ColumnSetColumn(ColumnSet& columnSet, const ColumnDefinition& columnDefinition);

    /**
     * Initializes object of class ColumnSetColumn.
     * @param columnSet Column set.
     * @param columnSetColumnRecord A column set column registry record.
     */
    ColumnSetColumn(ColumnSet& columnSet, const ColumnSetColumnRecord& columnSetColumnRecord);

    /**
     * Returns underlying column set obkect.
     * @return Column set.
     */
    ColumnSet& getColumnSet() const noexcept
    {
        return m_columnSet;
    }

    /**
     * Returns column definition constraint record ID.
     * @return Record ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns underlying column definition ID.
     * @return Column definition ID.
     */
    auto getColumnDefinitionId() const noexcept
    {
        return m_columnDefinitionId;
    }

    /**
     * Returns underlying column ID.
     * @return Column ID.
     */
    auto getColumnId() const noexcept
    {
        return m_columnId;
    }

private:
    /**
     * Validates column set.
     * @param columnSet Column set to which this column set column is supposed to belong to.
     * @param columnSetColumnRecord Column set column record.
     * @return The same column set, if it is valid.
     * @throw DatabaseError if column set has different column set ID.
     */
    ColumnSet& validateColumnSet(
            ColumnSet& columnSet, const ColumnSetColumnRecord& columnSetColumnRecord);

private:
    /** Parent column set. */
    ColumnSet& m_columnSet;

    /** Column set column record ID */
    const std::uint64_t m_id;

    /** Column definition ID */
    const std::uint64_t m_columnDefinitionId;

    /** Column ID (cached from the column definition) */
    const std::uint64_t m_columnId;
};

}  // namespace siodb::iomgr::dbengine
