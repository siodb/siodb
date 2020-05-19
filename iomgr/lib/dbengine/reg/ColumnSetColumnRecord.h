// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

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

    /** Column set element ID */
    std::uint64_t m_id;

    /** Column set ID */
    std::uint64_t m_columnSetId;

    /** Column definition ID */
    std::uint64_t m_columnDefinitionId;

    /** Column ID (cached from column definition) */
    std::uint64_t m_columnId;

    /** Structure name */
    static constexpr const char* kClassName = "ColumnSetColumnRecord";
};

}  // namespace siodb::iomgr::dbengine
