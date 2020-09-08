// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/**
 * Concatenation operator (left || right).
 */
class ConcatenationOperator final : public BinaryOperator {
public:
    /**
     * Initializes object of class ConcatenationOperator.
     * @param left Left operand.
     * @param right Right operand.
     * @throw std::invalid_argument if any operand is nullptr
     */
    ConcatenationOperator(ExpressionPtr&& left, ExpressionPtr&& right) noexcept
        : BinaryOperator(ExpressionType::kConcatenateOperator, std::move(left), std::move(right))
    {
    }

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
    ColumnDataType getColumnDataType(const ExpressionEvaluationContext& context) const override;

    /**
     * Returns expression text.
     * @return Expression text.
     */
    MutableOrConstantString getExpressionText() const override;

    /**
     * Evaluates expression. Operands are converted to strings
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
