/** Expression evaluation context */
// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/iomgr/shared/dbengine/ColumnDataType.h>
#include <siodb/iomgr/shared/dbengine/Variant.h>

namespace siodb::iomgr::dbengine::requests {

/** 
 * ExpressionEvaluationContext provides column data types and values for the expression evaluation.
 */
class ExpressionEvaluationContext {
public:
    /** De-initializes object of class ExpressionEvaluationContext. */
    virtual ~ExpressionEvaluationContext() = default;

    /**
     * Returns column value. May read value from the underlying source, if it is not yet cached.
     * @param tableIndex Table index.
     * @param columnIndex Column index.
     * @return Value of the specified column.
     */
    virtual const Variant& getColumnValue(std::size_t tableIndex, std::size_t columnIndex) = 0;

    /**
     * Returns column data type.
     * @param tableIndex Table index.
     * @param columnIndex Column index.
     * @return Column data type.
     */
    virtual ColumnDataType getColumnDataType(
            std::size_t tableIndex, std::size_t columnIndex) const = 0;
};

}  // namespace siodb::iomgr::dbengine::requests
