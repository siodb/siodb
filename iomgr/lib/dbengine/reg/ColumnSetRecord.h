// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnSetColumnRegistry.h"

namespace siodb::iomgr::dbengine {

class ColumnSet;

/** In-memory column set registry record */
struct ColumnSetRecord {
    /** Initializes object of class ColumnSetRecord */
    ColumnSetRecord() noexcept
        : m_id(0)
        , m_tableId(0)
    {
    }

    /**
     * Initializes object of class ColumnSetRecord.
     * @param id Column set ID.
     * @param tableId Table ID.
     */
    ColumnSetRecord(std::uint64_t id, std::uint32_t tableId) noexcept
        : m_id(id)
        , m_tableId(tableId)
    {
    }

    /**
     * Initializes object of class ColumnSetRecord.
     * @param id Column set ID.
     * @param tableId Table ID.
     * @param column List of columns.
     */
    ColumnSetRecord(
            std::uint64_t id, std::uint32_t tableId, ColumnSetColumnRegistry&& columns) noexcept
        : m_id(id)
        , m_tableId(tableId)
        , m_columns(std::move(columns))
    {
    }

    /**
     * Initializes object of class ColumnSetRecord.
     * @param columnSetColumn Column set object.
     */
    explicit ColumnSetRecord(const ColumnSet& columnSet);

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const ColumnSetRecord& other) const noexcept
    {
        return m_id == other.m_id && m_tableId == other.m_tableId && m_columns == other.m_columns;
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

    /** Column set ID */
    std::uint64_t m_id;

    /** Table ID */
    std::uint32_t m_tableId;

    /** Column set columns */
    ColumnSetColumnRegistry m_columns;

    /** Structure UUID */
    static const Uuid kClassUuid;

    /** Structure name */
    static constexpr const char* kClassName = "ColumnSetRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
