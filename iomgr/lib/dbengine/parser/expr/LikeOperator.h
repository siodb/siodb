// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BinaryOperator.h"

namespace siodb::iomgr::dbengine::requests {

/** LIKE operator (expr LIKE expr) */
class LikeOperator final : public BinaryOperator {
public:
    /**
     * Initializes object of class LikeOperator.
     * @param left Left operand.
     * @param right Right operand.
     * @param notLike Indicates that this is NOT LIKE statement.
     * @throw std::invalid_argument if any operand is nullptr
     */
    LikeOperator(ExpressionPtr&& left, ExpressionPtr&& right, bool notLike) noexcept
        : BinaryOperator(ExpressionType::kLikePredicate, std::move(left), std::move(right))
        , m_notLike(notLike)
    {
    }

    /**
     * Returns indication that this is NOT LIKE operator
     * @return true if this is NOT LIKE operator, false otherwise
     */
    bool isNotLike() const noexcept
    {
        return m_notLike;
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
     * Checks if operands are strings and valid.
     * @param context Evaluation context.
     * @std::runtime_error if operands aren't strings or not valid
     */
    void validate(const Context& context) const override;

    /**
     * Evaluates expression.
     * @param context Evaluation context.
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
    /**
     * Matches string to pattern. Assumes pattern and string are UTF-8 strings.
     * @param str A string to match.
     * @param strEnd String end.
     * @param pattern A pattern.
     * @param patternEnd Pattern end.
     * @return true if a string matches to a pattern, false otherwise.
     */
    static bool matchPattern(
            const char* str, const char* strEnd, const char* pattern, const char* patternEnd);

private:
    /* Indicates NOT LIKE operator. */
    const bool m_notLike;
};

}  // namespace siodb::iomgr::dbengine::requests
