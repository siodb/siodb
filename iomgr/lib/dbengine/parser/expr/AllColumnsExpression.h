// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnExpressionBase.h"

// STL headers
#include <optional>

namespace siodb::iomgr::dbengine::requests {

/** '*' column expression */
class AllColumnsExpression final : public ColumnExpressionBase {
public:
    /**
     * Initializes object of class AllColumnsExpression.
     * @param tableName Table name.
     */
    explicit AllColumnsExpression(std::string&& tableName) noexcept
        : ColumnExpressionBase(ExpressionType::kAllColumnsReference, std::move(tableName))
    {
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
     */
    void validate(const Context& context) const override;

    /**
     * Evaluates expression. Cannot be applied to "all columns" expression.
     * @param context Evaluation context.
     * @return Resulting value.
     * @throw std::runtime_error if evaluation error happens.
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
     * Dumps expression-specific part to a stream.
     * @param os Output stream.
     */
    void dumpImpl(std::ostream& os) const override final;
};

}  // namespace siodb::iomgr::dbengine::requests
