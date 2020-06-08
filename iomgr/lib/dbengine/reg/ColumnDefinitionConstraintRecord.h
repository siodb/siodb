// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/Uuid.h>

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

class ColumnDefinitionConstraint;

/** Column definition constraint record */
struct ColumnDefinitionConstraintRecord {
    /** Initializes object of class ColumnDefinitionConstraintRecord */
    ColumnDefinitionConstraintRecord() noexcept
        : m_id(0)
        , m_columnDefinitionId(0)
        , m_constraintId(0)
    {
    }

    /**
     * Initializes object of class ColumnDefinitionConstraintRecord.
     * @param id Column set element ID.
     * @param constraintId Constraint ID.
     */
    ColumnDefinitionConstraintRecord(
            std::uint64_t id, std::uint64_t columnDefinitionId, std::uint64_t constraintId) noexcept
        : m_id(id)
        , m_columnDefinitionId(columnDefinitionId)
        , m_constraintId(constraintId)
    {
    }

    /**
     * Initializes object of class ColumnDefinitionConstraintRecord.
     * @param columnDefinitionConstraint Column definition constraint.
     */
    explicit ColumnDefinitionConstraintRecord(
            const ColumnDefinitionConstraint& columnDefinitionConstraint) noexcept;

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const ColumnDefinitionConstraintRecord& other) const noexcept
    {
        return m_id == other.m_id && m_columnDefinitionId == other.m_columnDefinitionId
               && m_constraintId == other.m_constraintId;
    }

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

    /** Column definition constraint record ID */
    std::uint64_t m_id;

    /** Column definition ID */
    std::uint64_t m_columnDefinitionId;

    /** Constraint ID */
    std::uint64_t m_constraintId;

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "ColumnDefinitionConstraintRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
