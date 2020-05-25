// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDefinitionPtr.h"

// STL headers
#include <vector>

namespace siodb::iomgr::dbengine {

/** Index column specificatopm */
struct IndexColumnSpecification {
    /**
     * Initializes object of class IndexColumnSpecification.
     * @param columnDefintion Column definition.
     * @param sortDescending Indication of the descending sort order.
     */
    IndexColumnSpecification(const ColumnDefinitionPtr& columnDefinition, bool sortDescending)
        : m_columnDefinition(columnDefinition)
        , m_sortDescending(sortDescending)
    {
    }

    /** Column definition */
    ColumnDefinitionPtr m_columnDefinition;

    /** Descesing sort order indication */
    bool m_sortDescending;
};

/** Index column specification list */
using IndexColumnSpecificationList = std::vector<IndexColumnSpecification>;

}  // namespace siodb::iomgr::dbengine
