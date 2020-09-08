// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "expr/ExpressionEvaluationContext.h"

namespace siodb::iomgr::dbengine::requests {

/**
 * Empty context used for expression evaluation
 */
class EmptyExpressionEvaluationContext : public ExpressionEvaluationContext {
    /**
     * Throws exception on call, invalid method for this class.
     * @param tableIndex Table index.
     * @param columnIndex Column index.
     * @return Never returns.
     * @throw DatabaseError if method is called.
     */
    const siodb::iomgr::dbengine::Variant& getColumnValue(
            std::size_t tableIndex, std::size_t columnIndex) override;

    /**
     * Throws exception on call, invalid method for this class.
     * @param tableIndex Table index.
     * @param columnIndex Column index.
     * @return Never returns.
     * @throw DatabaseError if method is called.
     */
    siodb::ColumnDataType getColumnDataType(
            std::size_t tableIndex, std::size_t columnIndex) const override;
};

}  // namespace siodb::iomgr::dbengine::requests
