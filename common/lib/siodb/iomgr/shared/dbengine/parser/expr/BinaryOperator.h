// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Expression.h"

namespace siodb::iomgr::dbengine::requests {

/** Base class for the all binary (two operand) operators. */
class BinaryOperator : public Expression {
protected:
    /**
     * Initializes object of class BinaryOperator.
     * @param type Expression type.
     * @param left Left operand.
     * @param right Right operand.
     * @throw std::invalid_argument if any operand is nullptr.
     */
    BinaryOperator(ExpressionType type, ExpressionPtr&& left, ExpressionPtr&& right) noexcept;

public:
    /**
     * Returns left operand.
     * @return Left operand.
     */
    const Expression& getLeftOperand() const noexcept
    {
        return *m_left;
    }

    /**
     * Returns right operand.
     * @return Right operand.
     */
    const Expression& getRightOperand() const noexcept
    {
        return *m_right;
    }

    /**
     * Returns indication that expression is binary operator.
     * @return true if expression type is binary operator, false otherwise.
     */
    bool isBinaryOperator() const noexcept override final;

    /**
     * Returns memory size in bytes required to serialize this expression.
     * @return Memory size in bytes.
     */
    std::size_t getSerializedSize() const noexcept override;

    /**
     * Checks that operands are valid.
     * @param context Evaluation context.
     */
    void validate(const ExpressionEvaluationContext& context) const override;

    /**
     * Serializes this expression, doesn't check memory buffer size.
     * @param buffer Memory buffer address.
     * @return Address after a last written byte.
     * @throw std::runtime_error if serialization failed.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer) const override;

protected:
    /**
     * Compares structure of this expression with another one for equality.
     * @param other Other expression. Guaranteed to be of the same type as this one.
     * @return true if expressions structurally equal, false otherwise.
     */
    bool isEqualTo(const Expression& other) const noexcept override;

    /**
     * Creates deep copy of this expression.
     * @tparam Expr Expression class.
     * @return New expression object.
     */
    template<class Expr>
    Expression* cloneImpl() const;

    /**
     * Dumps expression-specific part to a stream.
     * @param os Output stream.
     */
    void dumpImpl(std::ostream& os) const override final;

    /**
     * Returns indication that some operand is null.
     * @param context Evaluation context.
     * @return true if any operand is null., false otherwise.
     */
    bool hasNullOperand(const ExpressionEvaluationContext& context) const
    {
        return isNullType(m_left->getResultValueType(context))
               || isNullType(m_right->getResultValueType(context));
    }

protected:
    /** Left operand */
    const ExpressionPtr m_left;

    /** Right operand */
    const ExpressionPtr m_right;
};

template<class Expr>
Expression* BinaryOperator::cloneImpl() const
{
    ExpressionPtr left(m_left->clone()), right(m_right->clone());
    return new Expr(std::move(left), std::move(right));
}

}  // namespace siodb::iomgr::dbengine::requests
