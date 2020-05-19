// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

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
        , m_isSortDescending(false)
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
        , m_isSortDescending(sortDescending)
    {
    }

    /**
     * Initializes object of class IndexColumnRecord.
     * @param indexColumn Source index column object.
     */
    explicit IndexColumnRecord(const IndexColumn& indexColumn) noexcept;

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

    /** Index column record ID */
    std::uint64_t m_id;

    /** Index ID */
    std::uint64_t m_indexId;

    /** Column definition ID */
    std::uint64_t m_columnDefinitionId;

    /** Indication of the descending sorting order */
    bool m_isSortDescending;

    /** Structure name */
    static constexpr const char* kClassName = "IndexColumnRecord";
};

}  // namespace siodb::iomgr::dbengine
