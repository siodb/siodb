// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ExpressionType.h"
#include "../../ColumnDataType.h"
#include "../../Variant.h"

// Common project headers
#include <siodb/common/utils/MutableOrConstantString.h>

// Protobuf message headers
#include <siodb/common/proto/ColumnDataType.pb.h>

namespace siodb::iomgr::dbengine::requests {

/** Base class for all expressions. */
class Expression {
protected:
    /** 
     * Initializes object of class Expression.
     * @param type Expression type.
     */
    explicit Expression(ExpressionType type) noexcept
        : m_type(type)
    {
    }

public:
    /** Expression evaluation context */
    class Context {
    public:
        /** De-initializes object of class Context. */
        virtual ~Context() = default;

        /**
         * Returns column value. May read value from the underlying source, 
         * if it is not yet cached.
         * @param tableIndex Table index.
         * @param columnIndex Column index.
         * @return Value of the specified column.
         */
        virtual const Variant& getColumnValue(std::size_t tableIndex, std::size_t columnIndex) = 0;
        /**
         * Returns column data type.
         * @param tableIndex Table index.
         * @param columnIndex Column index.
         * @return Column data type.
         */
        virtual ColumnDataType getColumnDataType(
                std::size_t tableIndex, std::size_t columnIndex) const = 0;
    };

    /** De-initializes object of class Expression. */
    virtual ~Expression() = default;

    /**
     * Returns expression type.
     * @return Expression type.
     */
    ExpressionType getType() const noexcept
    {
        return m_type;
    }

    /**
     * Returns indication that expression is constant.
     * @return true if expression type is constant, false otherwise.
     */
    virtual bool isConstant() const noexcept;

    /**
     * Returns indication that expression is unary operator.
     * @return true if expression type is unary operator, false otherwise.
     */
    virtual bool isUnaryOperator() const noexcept;

    /**
     * Returns indication that expression is binary operator.
     * @return true if expression type is binary operator, false otherwise.
     */
    virtual bool isBinaryOperator() const noexcept;

    /**
     * Returns indication that expression is ternary operator.
     * @return true if expression type is ternary operator, false otherwise.
     */
    virtual bool isTernaryOperator() const noexcept;

    /**
     * Returns indication that expression result value type can be DateTime.
     * @param context Evaluation context.
     * @return true if expression result value type is DateTime, false otherwise
     */
    virtual bool canCastAsDateTime(const Context& context) const noexcept;

    /**
     * Returns value type of expression.
     * @param context Evaluation context.
     * @return Result value type.
     */
    virtual VariantType getResultValueType(const Context& context) const = 0;

    /**
     * Returns type of generated column from this expression.
     * @param context Evaluation context.
     * @return Column data type.
     */
    virtual ColumnDataType getColumnDataType(const Context& context) const = 0;

    /**
     * Returns expression text.
     * @return Expression text.
     */
    virtual MutableOrConstantString getExpressionText() const = 0;

    /**
     * Returns memory size in bytes required to serialize this expression.
     * @return Memory size in bytes.
     */
    virtual std::size_t getSerializedSize() const noexcept = 0;

    /**
     * Compares structure of this expression with another one for equality.
     * @param other Other expression.
     * @return true if expressions structurally equal, false otherwise.
     */
    bool operator==(const Expression& other) const noexcept
    {
        return m_type == other.m_type && isEqualTo(other);
    }

    /**
     * Compares structure of this expression with another one for non-equality.
     * @param other Other expression.
     * @return true if expressions structurally non-equal, false otherwise.
     */
    bool operator!=(const Expression& other) const noexcept
    {
        return m_type != other.m_type || !isEqualTo(other);
    }

    /**
     * Checks if expression is valid.
     * @param context Evaluation context.
     */
    virtual void validate(const Context& context) const = 0;

    /**
     * Evaluates expression.
     * @param context Evaluation context.
     * @return Resulting value.
     */
    virtual Variant evaluate(Context& context) const = 0;

    /**
     * Serializes this expression, doesn't check memory buffer size.
     * @param buffer Memory buffer address.
     * @return Address after a last written byte.
     * @throw std::runtime_error if serialization failed.
     */
    virtual std::uint8_t* serializeUnchecked(std::uint8_t* buffer) const = 0;

    /**
     * Deserializes expression.
     * @param buffer Memory buffer address.
     * @param length Data length.
     * @param[out] result Resulting expression.
     * @return Number of consumed bytes.
     * @throw std::runtime_error if deserialization failed.
     */
    static std::size_t deserialize(
            const std::uint8_t* buffer, std::size_t length, std::unique_ptr<Expression>& result);

    /**
     * Creates deep copy of this expression.
     * @return New expression object.
     */
    virtual Expression* clone() const = 0;

    /**
     * Dumps expression to a stream.
     * @param os Output stream.
     */
    void dump(std::ostream& os) const;

protected:
    /**
     * Compares structure of this expression with another one for equality.
     * @param other Other expression. Guaranteed to be of the same type as this one.
     * @return true if expressions structurally equal, false otherwise.
     */
    virtual bool isEqualTo(const Expression& other) const noexcept = 0;

    /**
     * Dumps expression-specific part to a stream.
     * @param os Output stream.
     */
    virtual void dumpImpl(std::ostream& os) const = 0;

protected:
    /** Expression type */
    const ExpressionType m_type;
};

/** Expression unique pointer shorthand */
using ExpressionPtr = std::unique_ptr<Expression>;

/** Constant Expression unique pointer shorthand */
using ConstExpressionPtr = std::unique_ptr<const Expression>;

/**
 * Outputs expression to the stream.
 * @param os Output stream.
 * @param expr An expression.
 * @return Output stream.
 */
inline std::ostream& operator<<(std::ostream& os, const Expression& expr)
{
    expr.dump(os);
    return os;
}

}  // namespace siodb::iomgr::dbengine::requests
