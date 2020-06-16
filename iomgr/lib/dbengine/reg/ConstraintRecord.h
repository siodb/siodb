// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/Uuid.h>
#include <siodb/iomgr/shared/dbengine/ConstraintState.h>

// CRT headers
#include <cstdint>

// STL headers
#include <optional>
#include <string>

namespace siodb::iomgr::dbengine {

class Constraint;

/** In-memory constraint registry record */
struct ConstraintRecord {
    /** Initializes object of class ConstraintRecord */
    ConstraintRecord() noexcept
        : m_id(0)
        , m_state(ConstraintState::kCreating)
        , m_tableId(0)
        , m_columnId(0)
        , m_constraintDefinitionId(0)
    {
    }

    /**
     * Initializes object of class ConstraintRecord
     * @param id Constraint ID.
     * @param name Constraint name.
     * @param state Constraint state.
     * @param tableId Table ID to which this constraint belongs.
     * @param columnId Column ID to which this constraint belongs (zero if table constraint).
     * @param constraintDefinitionId Constraint definition ID.
     * @param description Constraint description.
     */
    ConstraintRecord(std::uint64_t id, std::string&& name, ConstraintState state,
            std::uint32_t tableId, std::uint64_t columnId, std::uint64_t constraintDefinitionId,
            std::optional<std::string>&& description) noexcept
        : m_id(id)
        , m_name(std::move(name))
        , m_state(state)
        , m_tableId(tableId)
        , m_columnId(columnId)
        , m_constraintDefinitionId(constraintDefinitionId)
        , m_description(std::move(description))
    {
    }

    /**
     * Initializes object of class ConstraintRecord.
     * @param constraint A constraint.
     */
    explicit ConstraintRecord(const Constraint& constraint);

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const ConstraintRecord& other) const noexcept
    {
        return m_id == other.m_id && m_name == other.m_name && m_state == other.m_state
               && m_tableId == other.m_tableId && m_columnId == other.m_columnId
               && m_constraintDefinitionId == other.m_constraintDefinitionId
               && m_description == other.m_description;
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

    /** Constraint ID */
    std::uint64_t m_id;

    /** Constraint name */
    std::string m_name;

    /** Constraint state */
    ConstraintState m_state;

    /** Table to which this constraint belongs */
    std::uint32_t m_tableId;

    /** Column to which this constraint belongs, zero if table constraint */
    std::uint64_t m_columnId;

    /** Constraint definition */
    std::uint64_t m_constraintDefinitionId;

    /** Constraint description */
    std::optional<std::string> m_description;

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "ConstraintRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
