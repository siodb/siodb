// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TableColumns.h"

// Project headers
#include "Column.h"

namespace siodb::iomgr::dbengine {

///// struct TableColumns::ExtractColumnId ///////////////////////////////////////////////////////

TableColumns::ExtractColumnId::result_type TableColumns::ExtractColumnId::operator()(
        const TableColumn& value) const noexcept
{
    return value.m_column->getId();
}

///// struct TableColumns::ExtractColumnName /////////////////////////////////////////////////////

const TableColumns::ExtractColumnName::result_type& TableColumns::ExtractColumnName::operator()(
        const TableColumn& value) const noexcept
{
    return value.m_column->getName();
}

}  // namespace siodb::iomgr::dbengine
