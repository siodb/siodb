// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/Uuid.h>

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

class ColumnSetColumn;

/** Column set column record */
struct ColumnSetColumnRecord {
    /** Initializes object of class ColumnSetColumnRecord */
    ColumnSetColumnRecord() noexcept
        : m_id(0)
        , m_columnSetId(0)
        , m_columnDefinitionId(0)
        , m_columnId(0)
    {
    }

    /**
     * Initializes object of class ColumnSetColumnRecord.
     * @param id Column set element ID.
     * @param columnSetId Column set ID.
     * @param columnDefinitionId Column definition ID.
     * @param columnId Column ID.
     */
    ColumnSetColumnRecord(std::uint64_t id, std::uint64_t columnSetId,
            std::uint64_t columnDefinitionId, std::uint64_t columnId) noexcept
        : m_id(id)
        , m_columnSetId(columnSetId)
        , m_columnDefinitionId(columnDefinitionId)
        , m_columnId(columnId)
    {
    }

    /**
     * Initializes object of class ColumnSetColumnRecord.
     * @param columnSetColumn Column set column object.
     */
    explicit ColumnSetColumnRecord(const ColumnSetColumn& columnSetColumn) noexcept;

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const ColumnSetColumnRecord& other) const noexcept
    {
        return m_id == other.m_id && m_columnSetId == other.m_columnSetId
               && m_columnDefinitionId == other.m_columnDefinitionId
               && m_columnId == other.m_columnId;
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

    /** Column set element ID */
    std::uint64_t m_id;

    /** Column set ID */
    std::uint64_t m_columnSetId;

    /** Column definition ID */
    std::uint64_t m_columnDefinitionId;

    /** Column ID (cached from column definition) */
    std::uint64_t m_columnId;

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "ColumnSetColumnRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
