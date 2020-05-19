// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "TernaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/**
 * BETWEEN operator (<expr> BETWEEN  <expr> AND <expr>)
 * and ((<expr> NOT BETWEEN  <expr> AND <expr>))
 */
class BetweenOperator final : public TernaryOperator {
public:
    /**
     * Initializes object of class BetweenOperator.
     * @param left Left operand.
     * @param middle Middle operand.
     * @param right Right operand.
     * @param notBetween Indicates that this is NOT BETWEEN statement
     * @throw std::invalid_argument if any operand is nullptr
     */
    BetweenOperator(ExpressionPtr&& left, ExpressionPtr&& middle, ExpressionPtr&& right,
            bool notBetween) noexcept
        : TernaryOperator(ExpressionType::kBetweenPredicate, std::move(left), std::move(middle),
                std::move(right))
        , m_notBetween(notBetween)
    {
    }

    /**
     * Returns indication that this is NOT BETWEEN operator
     * @return true in case of NOT BETWEEN operator, false in case of BETWEEN
     */
    bool isNotBetween() const noexcept
    {
        return m_notBetween;
    }

    /**
     * Returns value type of expression.
     * @param context Evaluation context.
     * @return Evaluated expression value type.
     */
    VariantType getResultValueType(const Context& context) const override;

    /**
     * Returns type of generated column from this expression.
     * @param context Evaluation context.
     * @return Column data type.
     */
    ColumnDataType getColumnDataType(const Context& context) const override;

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
     * Checks if operands are numeric or  dates and valid.
     * @param context Evaluation context.
     * @std::runtime_error if operands aren't numeric or dates or not valid
     */
    void validate(const Context& context) const override;

    /**
     * Evaluates expression.
     * @param context Evaluation context
     * @return Resulting value.
     */
    Variant evaluate(Context& context) const override;

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
    /** Indicates NOT BETWEEN operator. */
    const bool m_notBetween;
};

}  // namespace siodb::iomgr::dbengine::requests
