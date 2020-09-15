// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BitwiseBinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** Bitwise OR operator (left | right) */
class BitwiseOrOperator final : public BitwiseBinaryOperator {
public:
    /**
     * Initializes object of class BitwiseOrOperator.
     * @param left Left operand.
     * @param right Right operand.
     * @throw std::invalid_argument if any operand is nullptr
     */
    BitwiseOrOperator(ExpressionPtr&& left, ExpressionPtr&& right) noexcept
        : BitwiseBinaryOperator(
                ExpressionType::kBitwiseOrOperator, std::move(left), std::move(right))
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
