// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Expression.h"

namespace siodb::iomgr::dbengine::requests {

/**
 * In operator (<expr> [NOT] IN  <expr>).
 */
class InOperator final : public Expression {
public:
    /**
     * Initializes object of class InOperator.
     * @param value Value.
     * @param variants Allowed variants for value.
     * @param notIn Indicates that this is NOT IN statement.
     * @throw std::invalid_argument if value is nullptr.
     * @throw std::invalid_argument if variants is empty.
     */
    InOperator(ExpressionPtr&& value, std::vector<ExpressionPtr>&& variants, bool notIn) noexcept;

    /**
     * Returns indication that this is NOT IN operator.
     * @return true if this is NOT IN operator, false otherwise.
     */
    bool isNotIn() const noexcept
    {
        return m_notIn;
    }

    /**
     * Returns value.
     * @return Current value.
     */
    const auto& getValue() const noexcept
    {
        return *m_value;
    }

    /**
     * Returns value variants.
     * @return Value variants.
     */
    const auto& getVariants() const noexcept
    {
        return m_variants;
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
     * Returns memory size in bytes required to serialize this expression.
     * @return Memory size in bytes.
     */
    std::size_t getSerializedSize() const noexcept override;

    /** 
     * Checks that operands are valid.
     * @param context Evaluation context.
     */
    void validate(const ExpressionEvaluationContext& context) const;

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
    /** Value expression */
    const ExpressionPtr m_value;

    /** Allowed variants of value */
    const std::vector<ExpressionPtr> m_variants;

    /** NOT IN operator flag */
    const bool m_notIn;
};

}  // namespace siodb::iomgr::dbengine::requests
