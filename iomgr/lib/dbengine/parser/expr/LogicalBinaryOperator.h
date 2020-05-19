// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** A base class for binary logical operators AND, OR */
class LogicalBinaryOperator : public BinaryOperator {
protected:
    /**
     * Initializes object of class LogicalBinaryOperator.
     * @param type Expression type.
     * @param left Left operand.
     * @param right Right operand.
     * @throw std::invalid_argument if left is nullptr
     */
    LogicalBinaryOperator(ExpressionType type, ExpressionPtr&& left, ExpressionPtr&& right) noexcept
        : BinaryOperator(type, std::move(left), std::move(right))
    {
    }

public:
    /**
     * Returns value type of expression.
     * @param context Evaluation context.
     * @return Evaluated expression value type.
     */
    VariantType getResultValueType(const Context& context) const override;

    /**
     * Returns type of generated column from this expression.
     * @param context Evaluation context.
     * @return Column data type.
     */
    ColumnDataType getColumnDataType(const Context& context) const override final;

    /**
     * Checks if operands are bool and valid.
     * @param context Evaluation context.
     * @std::runtime_error if operands result aren't bool or not valid
     */
    void validate(const Context& context) const override final;
};

}  // namespace siodb::iomgr::dbengine::requests
