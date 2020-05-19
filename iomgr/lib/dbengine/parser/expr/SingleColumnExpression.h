// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnExpressionBase.h"

namespace siodb::iomgr::dbengine::requests {

/** Column expression */
class SingleColumnExpression final : public ColumnExpressionBase {
public:
    /**
     * Initializes object of class SingleColumnExpression.
     * @param tableName Table name.
     * @param columnName Column name.
     */
    SingleColumnExpression(std::string&& tableName, std::string&& columnName) noexcept;

    /**
     * Returns table name.
     * @return Table name.
     */
    const std::string& getColumnName() const noexcept
    {
        return m_columnName;
    }

    /**
     * Returns dataset column index.
     * @return Dataset column index.
     */
    const auto& getDatasetColumnIndex() const noexcept
    {
        return m_datasetColumnIndex;
    }

    /**
     * Sets dataset column index.
     * @param datasetColumnIndex Column index
     */
    void setDatasetColumnIndex(std::size_t datasetColumnIndex) noexcept
    {
        m_datasetColumnIndex = datasetColumnIndex;
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
     * @param context Evaluation context.
     * @return Resulting value.
     * @throw DatabaseError in case of impossible evaluation.
     * @throw std::runtime_error in case of unitialized dataset and column indexes.
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

    /**
     * Dumps expression-specific part to a stream.
     * @param os Output stream.
     */
    void dumpImpl(std::ostream& os) const override final;

private:
    /**
     * Checks that @ref m_datasetTableIndex and @ref m_datasetColumnIndex values are set.
     * @throw std::runtime_error in case of unitialized dataset and column indices.
     */
    void checkHasTableAndColumnIndices() const;

private:
    /** Column name */
    const std::string m_columnName;

    /** Index of column in dataset, used to avoid searching table by name. */
    std::optional<std::size_t> m_datasetColumnIndex;
};

}  // namespace siodb::iomgr::dbengine::requests
