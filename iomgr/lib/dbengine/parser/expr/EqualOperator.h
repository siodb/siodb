// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ComparisonBinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** Equal operator (left = right) */
class EqualOperator final : public ComparisonBinaryOperator {
public:
    /**
     * Initializes object of class EqualOperator.
     * @param left Left operand.
     * @param right Right operand.
     * @throw std::invalid_argument if any operand is nullptr
     */
    EqualOperator(ExpressionPtr&& left, ExpressionPtr&& right) noexcept
        : ComparisonBinaryOperator(
                ExpressionType::kEqualPredicate, std::move(left), std::move(right))
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
