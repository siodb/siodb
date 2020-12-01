// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "UnaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** A base class for logical unary operator NOT */
class LogicalUnaryOperator : public UnaryOperator {
protected:
    /**
     * Initializes object of class ArithmeticUnaryOperator.
     * @param type Expression type
     * @param operand Operand
     * @throw std::invalid_argument if operand is nullptr
     */
    LogicalUnaryOperator(ExpressionType type, ExpressionPtr&& operand) noexcept
        : UnaryOperator(type, std::move(operand))
    {
    }

public:
    /**
     * Returns value type of expression.
     * @param context Evaluation context.
     * @return Evaluated expression value type.
     */
    VariantType getResultValueType(const ExpressionEvaluationContext& context) const override;

    /**
     * Returns type of generated column from this expression.
     * @param context Evaluation context.
     * @return Column data type.
     */
    ColumnDataType getColumnDataType(
            const ExpressionEvaluationContext& context) const override final;

    /**
     * Checks if operand is bool and valid.
     * @param context Evaluation context.
     * @std::runtime_error if operand isn't bool or not valid
     */
    void validate(const ExpressionEvaluationContext& context) const override;
};

}  // namespace siodb::iomgr::dbengine::requests
