// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Expression.h"

// Common project headers
#include <siodb/common/utils/Base128VariantEncoding.h>

// STL headers
#include <optional>

namespace siodb::iomgr::dbengine::requests {

/** A base class for any column expression */
class ColumnExpressionBase : public Expression {
protected:
    /**
     * Initializes object of class AllColumnsExpression.
     * @param tableName Table name.
     */
    ColumnExpressionBase(ExpressionType type, std::string&& tableName) noexcept
        : Expression(type)
        , m_tableName(std::move(tableName))
    {
    }

public:
    /**
     * Returns table name.
     * @return Table name.
     */
    const std::string& getTableName() const noexcept
    {
        return m_tableName;
    }

    /**
     * Returns dataset table index.
     * @return Dataset table index.
     */
    const auto& getDatasetTableIndices() const noexcept
    {
        return m_datasetTableIndices;
    }

    /**
     * Sets dataset table index.
     * @param datasetTableIndices List of table indices.
     */
    void setDatasetTableIndices(std::vector<std::size_t>&& datasetTableIndices) noexcept
    {
        m_datasetTableIndices = std::move(datasetTableIndices);
    }

    /**
     * Sets dataset table index.
     * @param datasetTableIndex Table index.
     */
    void setSingleDatasetTableIndex(std::size_t datasetTableIndex);

protected:
    /**
     * Returns memory size in bytes required to serialize common part of this expression.
     * @return Memory size in bytes.
     */
    std::size_t getCommonSerializedSize() const noexcept
    {
        return getExpressionTypeSerializedSize(m_type) + ::getSerializedSize(m_tableName);
    }

    /**
     * Serializes common part of this expression, doesn't check memory buffer size.
     * @param buffer Memory buffer address.
     * @return Address after a last written byte.
     * @throw std::runtime_error if serialization failed.
     */
    std::uint8_t* serializeCommonUnchecked(std::uint8_t* buffer) const
    {
        buffer = serializeExpressionTypeUnchecked(m_type, buffer);
        return ::serializeUnchecked(m_tableName, buffer);
    }

    /**
     * Compares structure of this expression with another one for equality.
     * @param other Other expression. Guaranteed to be of the same type as this one.
     * @return true if expressions structurally equal, false otherwise.
     */
    bool isEqualTo(const Expression& other) const noexcept override;

protected:
    /** Table name */
    const std::string m_tableName;

    /** Indices of columns in the dataset, stored to avoid searching columns by name */
    std::vector<std::size_t> m_datasetTableIndices;
};

}  // namespace siodb::iomgr::dbengine::requests
