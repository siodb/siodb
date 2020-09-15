// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Expression.h"

namespace siodb::iomgr::dbengine::requests {

/** Base class for the all unary (single operand) operators */
class UnaryOperator : public Expression {
protected:
    /**
     * Initializes object of class UnaryOperator.
     * @param type Expression type.
     * @param operand Operand.
     */
    UnaryOperator(ExpressionType type, ExpressionPtr&& operand) noexcept;

public:
    /**
     * Returns operand.
     * @return An operand.
     */
    const Expression& getOperand() const noexcept
    {
        return *m_operand;
    }

    /**
     * Returns indication that expression is unary operator.
     * @return true if expression type is unary operator, false otherwise.
     */
    bool isUnaryOperator() const noexcept override final;

    /**
     * Checks if operand is valid.
     * @param context Evaluation context.
     */
    void validate(const ExpressionEvaluationContext& context) const override;

    /**
     * Returns memory size in bytes required to serialize this expression.
     * @return Memory size in bytes.
     */
    std::size_t getSerializedSize() const noexcept override final;

    /**
     * Serializes this expression, doesn't check memory buffer size.
     * @param buffer Memory buffer address.
     * @return Address after a last written byte.
     * @throw std::runtime_error if serialization failed.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer) const override final;

protected:
    /**
     * Compares structure of this expression with another one for equality.
     * @param other Other expression. Guaranteed to be of the same type as this one.
     * @return true if expressions structurally equal, false otherwise.
     */
    bool isEqualTo(const Expression& other) const noexcept override final;

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

protected:
    /* Operand expression */
    const ExpressionPtr m_operand;
};

template<class Expr>
Expression* UnaryOperator::cloneImpl() const
{
    ExpressionPtr op(m_operand->clone());
    return new Expr(std::move(op));
}

}  // namespace siodb::iomgr::dbengine::requests
