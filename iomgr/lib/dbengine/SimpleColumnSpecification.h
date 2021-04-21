// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include <siodb-generated/common/lib/siodb/common/proto/ColumnDataType.pb.h>

// Common project headers
#include <siodb/iomgr/shared/dbengine/Variant.h>

// STL headers
#include <optional>

namespace siodb::iomgr::dbengine {

/** Simple column definition. Used in tests and for creating demo tables. */
struct SimpleColumnSpecification {
    /** Initializes new object of class SimpleColumnSpecification. */
    SimpleColumnSpecification() noexcept
        : m_dataType(COLUMN_DATA_TYPE_UNKNOWN)
    {
    }

    /**
     * Initializes new object of class SimpleColumnSpecification.
     * @param name Column name.
     * @param dataType Column data type.
     * @param notNull NOT NULL constraint indication.
     * @param defaultValue Default value.
     */
    SimpleColumnSpecification(const std::string& name, ColumnDataType dataType,
            std::optional<bool> notNull = std::nullopt, const Variant& defaultValue = {})
        : m_name(std::string(name))
        , m_dataType(dataType)
        , m_notNull(notNull)
        , m_defaultValue(defaultValue)
    {
    }

    /**
     * Initializes new object of class SimpleColumnSpecification.
     * @param name Column name.
     * @param dataType Column data type.
     * @param notNull NOT NULL constraint indication.
     * @param defaultValue Default value.
     */
    SimpleColumnSpecification(std::string&& name, ColumnDataType dataType,
            std::optional<bool> notNull = std::nullopt, Variant&& defaultValue = {}) noexcept
        : m_name(std::move(name))
        , m_dataType(dataType)
        , m_notNull(notNull)
        , m_defaultValue(std::move(defaultValue))
    {
    }

    /** Column name */
    std::string m_name;

    /** Column data type */
    ColumnDataType m_dataType;

    /** NOT NULL constraint indication. */
    std::optional<bool> m_notNull;

    /** Default value constraint. */
    Variant m_defaultValue;
};

}  // namespace siodb::iomgr::dbengine
