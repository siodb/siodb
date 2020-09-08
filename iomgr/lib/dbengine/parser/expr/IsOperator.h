// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ComparisonBinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** IS operator (left IS [NOT] right) */
class IsOperator final : public ComparisonBinaryOperator {
public:
    /**
     * Initializes object of class IsOperator.
     * @param left Left operand.
     * @param right Right operand.
     * @param isNot Indicates that operator is 'IS NOT'.
     * @throw std::invalid_argument if any operand is nullptr
     */
    IsOperator(ExpressionPtr&& left, ExpressionPtr&& right, bool isNot) noexcept
        : ComparisonBinaryOperator(ExpressionType::kIsPredicate, std::move(left), std::move(right))
        , m_isNot(isNot)
    {
    }

    /**
     * Returns indication that this is IS NOT operator
     * @return true if this is IS NOT operator, false otherwise
     */
    bool isNot() const noexcept
    {
        return m_isNot;
    }

    /**
     * Returns expression text.
     * @return Expression text.
     */
    MutableOrConstantString getExpressionText() const override;

    /**
     * Returns memory size in bytes required to serialize this expression.
     * @return Memory size in bytes.
     */
    std::size_t getSerializedSize() const noexcept override;

    /**
     * Evaluates expression.
     * @param context Evaluation context.
     * @return Resulting value.
     */
    Variant evaluate(ExpressionEvaluationContext& context) const override;

    /**
     * Serializes this expression, doesn't check memory buffer size.
     * @param buffer Memory buffer address.
     * @return Address after a last written byte.
     * @throw std::runtime_error if serialization failed.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer) const override;

    /**
     * Creates deep copy of this expression.
     * @return New expression object.
     */
    Expression* clone() const override;

protected:
    /**
     * Compares structure of this expression with another one for equality.
     * @param other Other expression. Guaranteed to be of the same type as this one.
     * @return true if expressions structurally equal, false otherwise.
     */
    bool isEqualTo(const Expression& other) const noexcept override;

private:
    /** Indicates IS NOT operator. */
    const bool m_isNot;
};

}  // namespace siodb::iomgr::dbengine::requests
