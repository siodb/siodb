// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "LogicalUnaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** Logical NOT operator */
class LogicalNotOperator final : public LogicalUnaryOperator {
public:
    /**
     * Initializes object of class LogicalNotOperator.
     * @param operand An operand
     * @throw std::invalid_argument if operand is nullptr
     */
    explicit LogicalNotOperator(ExpressionPtr&& operand) noexcept
        : LogicalUnaryOperator(ExpressionType::kLogicalNotOperator, std::move(operand))
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
