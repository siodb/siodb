// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include <siodb-generated/common/lib/siodb/common/proto/ColumnDataType.pb.h>
#include "../ColumnState.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>

namespace siodb::iomgr::dbengine {

class Column;

/** In-memory column registry record */
struct ColumnRecord {
    /** Initializes object of class ColumnRecord */
    ColumnRecord() noexcept
        : m_id(0)
        , m_dataType(COLUMN_DATA_TYPE_UNKNOWN)
        , m_tableId(0)
        , m_state(ColumnState::kCreating)
        , m_dataBlockDataAreaSize(kDefaultDataFileDataAreaSize)
    {
    }

    /**
     * Initializes object of class ColumnRecord.
     * @param columnId Column ID.
     * @param name Column name.
     * @param dataType Column data type.
     * @param tableId Table ID.
     * @param state Column state.
     * @param dataBlockDataAreaSize Column data file data area size.
     */
    ColumnRecord(std::uint64_t id, std::string&& name, ColumnDataType dataType,
            std::uint32_t tableId, ColumnState state, std::uint32_t dataBlockDataAreaSize) noexcept
        : m_id(id)
        , m_name(std::move(name))
        , m_dataType(dataType)
        , m_tableId(tableId)
        , m_state(state)
        , m_dataBlockDataAreaSize(dataBlockDataAreaSize)
    {
    }

    /**
     * Initializes object of class ColumnRecord.
     * @param column Column object.
     */
    explicit ColumnRecord(const Column& column);

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

    /** Column ID */
    std::uint64_t m_id;

    /** Column name */
    std::string m_name;

    /** Data type */
    ColumnDataType m_dataType;

    /** Table ID */
    std::uint32_t m_tableId;

    /** Column state */
    ColumnState m_state;

    /** Column data file data area size */
    std::uint32_t m_dataBlockDataAreaSize;

    /** Structure name */
    static constexpr const char* kClassName = "ColumnRecord";
};

}  // namespace siodb::iomgr::dbengine
