// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/Uuid.h>

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

class IndexColumn;

/** Index column registry record */
struct IndexColumnRecord {
    /** Initilizes object of class IndexColumnRecord */
    IndexColumnRecord() noexcept
        : m_id(0)
        , m_indexId(0)
        , m_columnDefinitionId(0)
        , m_sortDescending(false)
    {
    }

    /**
     * Initilizes object of class IndexColumnRecord.
     * @param id Index column record ID.
     * @param indexId Index ID.
     * @param columnDefinitionId Column definition ID.
     * @param sortDescending Indication of the descending sorting order.
     */
    IndexColumnRecord(std::uint64_t id, std::uint64_t indexId, std::uint64_t columnDefinitionId,
            bool sortDescending) noexcept
        : m_id(id)
        , m_indexId(indexId)
        , m_columnDefinitionId(columnDefinitionId)
        , m_sortDescending(sortDescending)
    {
    }

    /**
     * Initializes object of class IndexColumnRecord.
     * @param indexColumn Source index column object.
     */
    explicit IndexColumnRecord(const IndexColumn& indexColumn) noexcept;

    /**
     * Equality comparison operator.
     * @param other Other object.
     * @return true if this and other objects are equal, false otherwise.
     */
    bool operator==(const IndexColumnRecord& other) const noexcept
    {
        return m_id == other.m_id && m_indexId == other.m_indexId
               && m_columnDefinitionId == other.m_columnDefinitionId
               && m_sortDescending == other.m_sortDescending;
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

    /** Index column record ID */
    std::uint64_t m_id;

    /** Index ID */
    std::uint64_t m_indexId;

    /** Column definition ID */
    std::uint64_t m_columnDefinitionId;

    /** Indication of the descending sorting order */
    bool m_sortDescending;

    /** Structure UUID */
    static const Uuid kClassUuid;

    /** Structure name */
    static constexpr const char* kClassName = "IndexColumnRecord";

    /** Structure version */
    static constexpr std::uint32_t kClassVersion = 0;
};

}  // namespace siodb::iomgr::dbengine
