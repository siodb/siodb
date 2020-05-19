// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../ConstraintState.h"

// CRT headers
#include <cstdint>

// STL headers
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
     */
    ConstraintRecord(std::uint64_t id, std::string&& name, ConstraintState state,
            std::uint32_t tableId, std::uint64_t columnId,
            std::uint64_t constraintDefinitionId) noexcept
        : m_id(id)
        , m_name(std::move(name))
        , m_state(state)
        , m_tableId(tableId)
        , m_columnId(columnId)
        , m_constraintDefinitionId(constraintDefinitionId)
    {
    }

    /**
     * Initializes object of class ConstraintRecord.
     * @param constraint A constraint.
     */
    explicit ConstraintRecord(const Constraint& constraint);

    /**
     * Returns buffer size required to serialize this object.
     * @return Number of bytes.
     */
    std::size_t getSerializedSize() const noexcept;

    /**
     * Serializes object into buffer. Assumes buffer is big enough.
     * @param buffer Output buffer.
     * @return Address of byte after last written byte.
     */
    std::uint8_t* serializeUnchecked(std::uint8_t* buffer) const noexcept;

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

    /** Structure name */
    static constexpr const char* kClassName = "ConstraintRecord";
};

}  // namespace siodb::iomgr::dbengine
