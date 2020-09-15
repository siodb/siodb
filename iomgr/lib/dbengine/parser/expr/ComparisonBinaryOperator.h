// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** A base class for binary comparison operators ==,!=,<,>,<=,> */
class ComparisonBinaryOperator : public BinaryOperator {
public:
    /**
     * Initializes object of class ComparisonBinaryOperator.
     * @param type Expression type.
     * @param left Left operand.
     * @param right Right operand.
     */
    ComparisonBinaryOperator(
            ExpressionType type, ExpressionPtr&& left, ExpressionPtr&& right) noexcept
        : BinaryOperator(type, std::move(left), std::move(right))
    {
    }

    /**
     * Returns value type of expression.
     * @param context Evaluation context.
     * @return Evaluated expression value type.
     */
    VariantType getResultValueType(const ExpressionEvaluationContext& context) const override final;

    /**
     * Returns type of generated column from this expression.
     * @param context Evaluation context.
     * @return Column data type.
     */
    ColumnDataType getColumnDataType(
            const ExpressionEvaluationContext& context) const override final;
};

}  // namespace siodb::iomgr::dbengine::requests
