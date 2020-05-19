// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BitwiseUnaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** Unary complement operator (~X). */
class ComplementOperator final : public BitwiseUnaryOperator {
public:
    /**
     * Initializes object of class ComplementOperator.
     * @param operand Operand.
     * @throw std::invalid_argument if operand is nullptr
     */
    explicit ComplementOperator(ExpressionPtr&& operand) noexcept
        : BitwiseUnaryOperator(ExpressionType::kBitwiseComplementOperator, std::move(operand))
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
    Variant evaluate(Context& context) const override;

    /**
     * Creates deep copy of this expression.
     * @return New expression object.
     */
    Expression* clone() const override;
};

}  // namespace siodb::iomgr::dbengine::requests
