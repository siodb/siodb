// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../TableType.h"

// STL headers
#include <string>

namespace siodb::iomgr::dbengine {

class Table;

/** In-memory table registry record */
struct TableRecord {
    /** Initializes object of class TableRecord */
    TableRecord() noexcept
        : m_id(0)
        , m_type(TableType::kDisk)
        , m_firstUserTrid(0)
        , m_currentColumnSetId(0)
    {
    }

    /**
     * Initializes object of class TableRecord
     * @param id Table ID.
     * @param type Table type.
     * @param name Table name.
     * @param firstUserTrid First user range TRID.
     * @param currentColumnSetId Current column set ID.
     */
    TableRecord(std::uint32_t id, TableType type, std::string&& name, std::uint64_t firstUserTrid,
            std::uint64_t currentColumnSetId) noexcept
        : m_id(id)
        , m_type(type)
        , m_name(std::move(name))
        , m_firstUserTrid(firstUserTrid)
        , m_currentColumnSetId(currentColumnSetId)
    {
    }

    /**
     * Initializes object of class TableRecord.
     * @param table Table object.
     */
    TableRecord(const Table& table);

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

    /** Table ID */
    std::uint32_t m_id;

    /** Table type */
    TableType m_type;

    /** Table name */
    std::string m_name;

    /** First user range TRID */
    std::uint64_t m_firstUserTrid;

    /** Current column set ID */
    std::uint64_t m_currentColumnSetId;

    /** Structure name */
    static constexpr const char* kClassName = "TableRecord";
};

}  // namespace siodb::iomgr::dbengine
