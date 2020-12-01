// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "LogicalBinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** Logical AND operator (left AND right) */
class LogicalAndOperator final : public LogicalBinaryOperator {
public:
    /**
     * Initializes object of class LogicalAndOperator.
     * @param left Left operand.
     * @param right Right operand.
     * @throw std::invalid_argument if any operand is nullptr
     */
    LogicalAndOperator(ExpressionPtr&& left, ExpressionPtr&& right) noexcept
        : LogicalBinaryOperator(
                ExpressionType::kLogicalAndOperator, std::move(left), std::move(right))
    {
    }

    /**
     * Returns expression text.
     * @return Expression text.
     */
    MutableOrConstantString getExpressionText() const override;

    /**
     * Evaluates expression.
     * @param context Evaluation context.
     * @return Resulting value.
     * @throw std::runtime_error If expression has non bool operand
     */
    Variant evaluate(ExpressionEvaluationContext& context) const override;

    /**
     * Creates deep copy of this expression.
     * @return New expression object.
     */
    Expression* clone() const override;
};

}  // namespace siodb::iomgr::dbengine::requests
