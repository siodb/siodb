// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnPtr.h"

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/** Table column list element. */
struct TableColumn {
    /** 
     * Initializes object of class TableColumn.
     * @param column Column object.
     * @param columnSetColumnId Column set column record ID.
     * @param position Column position in the column set.
     */
    TableColumn(const ColumnPtr& column, std::uint64_t columnSetColumnId,
            std::uint32_t position) noexcept
        : m_column(column)
        , m_columnSetColumnId(columnSetColumnId)
        , m_position(position)
    {
    }

    /** Column object */
    ColumnPtr m_column;

    /** Column set column record ID. */
    std::uint64_t m_columnSetColumnId;

    /** Column position in the column set. */
    std::uint32_t m_position;
};

}  // namespace siodb::iomgr::dbengine
