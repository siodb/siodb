// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDefinition.h"
#include "Index.h"

// STL Headers
#include <ostream>

namespace siodb::iomgr::dbengine {

/** Index column  */
class IndexColumn {
public:
    /**
     * Initializes object of class IndexColumn for a new index column.
     * @param index Index that this index column belongs to.
     * @param columnDefinition Column definition.
     * @param sortDescending Indicates descending sorting order by this column.
     */
    IndexColumn(Index& index, const ColumnDefinitionPtr& columnDefinition, bool sortDescending)
        : m_index(index)
        , m_id(index.getTable().getDatabase().generateNextIndexColumnId(
                  index.getTable().isSystemTable()))
        , m_columnDefinition(columnDefinition)
        , m_isSortDescending(sortDescending)
    {
    }

    /**
     * Initializes object of class IndexColumn for an existing index column.
     * @param index Index that this index column belongs to.
     * @param columnDefinitionId Column definition ID.
     */
    IndexColumn(Index& index, const IndexColumnRecord& indexColumnRecord)
        : m_index(validateIndex(index, indexColumnRecord))
        , m_id(indexColumnRecord.m_id)
        , m_columnDefinition(m_index.getTable().getColumnDefinitionChecked(
                  indexColumnRecord.m_columnDefinitionId))
        , m_isSortDescending(indexColumnRecord.m_isSortDescending)
    {
    }

    /**
     * Returns index ID.
     * @return Index ID.
     */
    std::uint64_t getIndexId() const noexcept
    {
        return m_index.getId();
    }

    /**
     * Returns index column record ID.
     * @return Index column record ID.
     */
    std::uint64_t getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns column definition ID.
     * @return Column definition ID.
     */
    std::uint64_t getColumnDefinitionId() const noexcept
    {
        return m_columnDefinition->getId();
    }

    /**
     * Returns descending sorting order flag.
     * @return true if descending sorting should be applied for this column, false otherwise.
     */
    bool isDescendingSortOrder() const noexcept
    {
        return m_isSortDescending;
    }

private:
    /**
     * Validates index column index.
     * @param index Index to which this index column is supposed to belong to.
     * @param indexColumnRecord Index column record.
     * @return The same index, if it is valid.
     * @throw DatabaseError if index has different index ID.
     */
    static Index& validateIndex(Index& index, const IndexColumnRecord& indexColumnRecord);

private:
    /** Index to which this index column belongs to. */
    Index& m_index;

    /** Index column record ID */
    std::uint64_t m_id;

    /** Column definition */
    ColumnDefinitionPtr m_columnDefinition;

    /** Descending sorting order flag. */
    const bool m_isSortDescending;

    friend std::ostream& operator<<(std::ostream& os, const IndexColumn& indexColumn);
};

/**
 * Outputs IndexColumn object representation to the stream.
 * @param os Output stream.
 * @param indexColumn Index column object.
 */
inline std::ostream& operator<<(std::ostream& os, const IndexColumn& indexColumn)
{
    os << '(' << indexColumn.m_columnDefinition->getId() << ", "
       << (indexColumn.m_isSortDescending ? "Desc" : "Asc") << ')';
    return os;
}

}  // namespace siodb::iomgr::dbengine
