// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnConstraintSpecification.h"
#include "SimpleColumnSpecification.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>

namespace siodb::iomgr::dbengine {

/** Column specification */
struct ColumnSpecification {
    /** Initializes new object of class ColumnSpecification. */
    ColumnSpecification() noexcept
        : m_dataType(COLUMN_DATA_TYPE_UNKNOWN)
        , m_dataBlockDataAreaSize(kDefaultDataFileDataAreaSize)
    {
    }

    /**
     * Initializes new object of class ColumnSpecification.
     * @param name Column name.
     * @param dataType Column data type.
     * @param dataBlockDataAreaSize Data block data area size.
     * @param constraints Column constraints
     */
    ColumnSpecification(const std::string& name, ColumnDataType dataType,
            std::uint32_t dataBlockDataAreaSize,
            const ColumnConstraintSpecificationList& constraints = {}) noexcept
        : m_name(name)
        , m_dataType(dataType)
        , m_dataBlockDataAreaSize(dataBlockDataAreaSize)
        , m_constraints(constraints)
    {
    }

    /**
     * Initializes new object of class ColumnSpecification.
     * @param name Column name.
     * @param dataType Column data type.
     * @param dataBlockDataAreaSize Data block data area size.
     * @param constraints Column constraints
     */
    ColumnSpecification(std::string&& name, ColumnDataType dataType,
            std::uint32_t dataBlockDataAreaSize,
            ColumnConstraintSpecificationList&& constraints = {}) noexcept
        : m_name(std::move(name))
        , m_dataType(dataType)
        , m_dataBlockDataAreaSize(dataBlockDataAreaSize)
        , m_constraints(std::move(constraints))
    {
    }

    /**
     * Initializes new object of class ColumnSpecification.
     * @param src Source simple column info.
     */
    ColumnSpecification(const SimpleColumnSpecification& src);

    /** Column name */
    std::string m_name;

    /** Column data type */
    ColumnDataType m_dataType;

    /** Data block data area size */
    std::uint32_t m_dataBlockDataAreaSize;

    /** Column constraints */
    ColumnConstraintSpecificationList m_constraints;
};

}  // namespace siodb::iomgr::dbengine
