// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ArithmeticUnaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** Unary plus operator (+X). */
class UnaryPlusOperator final : public ArithmeticUnaryOperator {
public:
    /**
     * Initializes object of class UnaryPlusOperator.
     * @param operand Operand.
     * @throw std::invalid_argument if operand is nullptr
     */
    explicit UnaryPlusOperator(ExpressionPtr&& operand) noexcept
        : ArithmeticUnaryOperator(ExpressionType::kUnaryPlusOperator, std::move(operand))
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
     */
    Variant evaluate(ExpressionEvaluationContext& context) const override;

    /**
     * Creates deep copy of this expression.
     * @return New expression object.
     */
    Expression* clone() const override;
};

}  // namespace siodb::iomgr::dbengine::requests
