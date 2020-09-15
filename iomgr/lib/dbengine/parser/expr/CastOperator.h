// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {
/**
 * Cast operator CAST(expr AS type).
 */
class CastOperator final : public BinaryOperator {
public:
    /**
     * Initializes object of class CastOperator.
     * @param left Left operand.
     * @param right Right operand.
     * @throw std::invalid_argument if any operand is nullptr
     */
    CastOperator(ExpressionPtr&& left, ExpressionPtr&& right) noexcept
        : BinaryOperator(ExpressionType::kCastOperator, std::move(left), std::move(right))
    {
        checkRightIsStringConstant();
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
     * Checks if operands are numeric or strings and valid.
     * @param context Evaluation context.
     * @std::runtime_error if operands aren't numeric or strings or not valid
     */
    void validate(const ExpressionEvaluationContext& context) const override;

    /**
     * Evaluates expression.
     * @param context Evaluation context
     * @return Resulting value.
     */
    Variant evaluate(ExpressionEvaluationContext& context) const override;

    /**
     * Creates deep copy of this expression.
     * @return New expression object.
     */
    Expression* clone() const override;

private:
    /**
     * Checks that right operand is string constant.
     * @throw std::invalid_argument if right operand is not a string constant.
     */
    void checkRightIsStringConstant() const;
};

}  // namespace siodb::iomgr::dbengine::requests
