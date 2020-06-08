// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDefinitionConstraintRegistry.h"

// Common project headers
#include <siodb/common/utils/Uuid.h>

namespace siodb::iomgr::dbengine {

class ColumnDefinition;

/** In-memory column definition registry record */
struct ColumnDefinitionRecord {
    /** Initializes object of class ColumnDefinitionRecord */
    ColumnDefinitionRecord() noexcept
        : m_id(0)
        , m_columnId(0)
    {
    }

    /**
     * Initializes object of class ColumnDefinitionRecord.
     * @param id Column definition ID.
     * @param columnId Column ID.
     */
    ColumnDefinitionRecord(std::uint64_t id, std::uint64_t columnId) noexcept
        : m_id(id)
        , m_columnId(columnId)
    {
    }

    /**
     * Initializes object of class ColumnDefinitionRecord.
     * @param id Column definition ID.
     * @param columnId Column ID.
     * @param cointraints Constraints for this column definition.
     */
    ColumnDefinitionRecord(std::uint64_t id, std::uint64_t columnId,
            ColumnDefinitionConstraintRegistry&& constraints) noexcept
        : m_id(id)
        , m_columnId(columnId)
        , m_constraints(std::move(constraints))
    {
    }

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const ColumnDefinitionRecord& other) const noexcept
    {
        return m_id == other.m_id && m_columnId == other.m_columnId
               && m_constraints == other.m_constraints;
    }

    /**
     * Initializes object of class ColumnDefinitionRecord.
     * @param columnDefinition Column definition.
     */
    explicit ColumnDefinitionRecord(const ColumnDefinition& columnDefinition);

    /**
     * Returns buffer size required to serialize this object.
     * @param version Target version.
     * @return Number of bytes.
     */
    std::size_t getSerializedSize(unsigned version = kClassVersion) const noexcept;

    /**
     * Serializes object into buffer. Assumes buffer is big enough.
     * @param buffer Output buffer.
     * @param version Target version.
     * @return Address of byte after last written byte.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer, unsigned version = kClassVersion) const
            noexcept;

    /**
     * Deserializes object from buffer.
     * @param buffer Input buffer.
     * @param length Length of data in the buffer.
     * @return Number of bytes consumed.
     */
    std::size_t deserialize(const std::uint8_t* buffer, std::size_t length);

    /** Column set ID */
    std::uint64_t m_id;

    /** Table ID */
    std::uint64_t m_columnId;

    /** Column definition constraints */
    ColumnDefinitionConstraintRegistry m_constraints;

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "ColumnDefinitionRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
