// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/Uuid.h>
#include <siodb/iomgr/shared/dbengine/TableType.h>

// STL headers
#include <optional>
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
     * @param description Table description.
     */
    TableRecord(std::uint32_t id, TableType type, std::string&& name, std::uint64_t firstUserTrid,
            std::uint64_t currentColumnSetId, std::optional<std::string>&& description) noexcept
        : m_id(id)
        , m_type(type)
        , m_name(std::move(name))
        , m_firstUserTrid(firstUserTrid)
        , m_currentColumnSetId(currentColumnSetId)
        , m_description(std::move(description))
    {
    }

    /**
     * Initializes object of class TableRecord.
     * @param table Table object.
     */
    TableRecord(const Table& table);

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const TableRecord& other) const noexcept
    {
        return m_id == other.m_id && m_type == other.m_type && m_name == other.m_name
               && m_firstUserTrid == other.m_firstUserTrid
               && m_currentColumnSetId == other.m_currentColumnSetId
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

    /** Table description */
    std::optional<std::string> m_description;

    /** Structure UUID */
    static const Uuid s_classUuid;

    /** Structure name */
    static constexpr const char* kClassName = "TableRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
