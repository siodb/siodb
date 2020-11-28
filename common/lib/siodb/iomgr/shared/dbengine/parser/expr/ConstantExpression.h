// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Expression.h"

namespace siodb::iomgr::dbengine::requests {

/** Constant value expression. */
class ConstantExpression final : public Expression {
public:
    /** Initializes object of class ConstantExpression. */
    ConstantExpression() noexcept
        : Expression(ExpressionType::kConstant)
    {
    }

    /**
     * Initializes object of class ConstantExpression.
     * @param value Constant value.
     */
    explicit ConstantExpression(Variant&& value) noexcept
        : Expression(ExpressionType::kConstant)
        , m_value(std::move(value))
    {
    }

    /**
     * Creates new constant expression object with null value.
     * @return New constant expression object.
     */
    static std::unique_ptr<Expression> create()
    {
        return std::make_unique<ConstantExpression>();
    }

    /**
     * Creates new constant expression object with given value.
     * @tparam Value Value type.
     * @param value A value.
     * @return New constant expression object.
     */
    template<class Value>
    static std::unique_ptr<Expression> create(const Value& value)
    {
        return std::make_unique<ConstantExpression>(Variant(value));
    }

    /**
     * Creates new constant expression object.
     * @tparam Value Value type.
     * @param value A value.
     * @return New constant expression object.
     */
    template<class Value>
    static std::unique_ptr<Expression> create(Value&& value)
    {
        return std::make_unique<ConstantExpression>(Variant(std::move(value)));
    }

    /**
     * Returns underlying constant value.
     * @return Underlying constant value.
     */
    const Variant& getValue() const noexcept
    {
        return m_value;
    }

    /**
     * Returns indication that expression is constant.
     * @return true if expression type is constant, false otherwise.
     */
    bool isConstant() const noexcept override;

    /**
     * Returns indication that expression result value type can be DateTime.
     * @param context ExpressionEvaluationContext
     * @return true if expression result value type is DateTime, false otherwise
     */
    bool canCastAsDateTime(const ExpressionEvaluationContext& context) const noexcept override;

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
     * @return Operator text.
     */
    MutableOrConstantString getExpressionText() const override;

    /**
     * Returns memory size in bytes required to serialize this expression.
     * @return Memory size in bytes.
     */
    std::size_t getSerializedSize() const noexcept override;

    /**
     * Checks if operands are numeric or  dates and valid.
     * @param context ExpressionEvaluationContext
     * @std::runtime_error if operands aren't numeric or dates or not valid
     */
    void validate(const ExpressionEvaluationContext& context) const override;

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

    /**
     * Dumps expression-specific part to a stream.
     * @param os Output stream.
     */
    void dumpImpl(std::ostream& os) const override final;

private:
    /** Constant value */
    const Variant m_value;
};

}  // namespace siodb::iomgr::dbengine::requests
