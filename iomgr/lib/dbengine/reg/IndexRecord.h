// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IndexColumnRegistry.h"
#include "../IndexType.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>

// STL headers
#include <string>

namespace siodb::iomgr::dbengine {

class Index;

struct IndexRecord {
    /** Initializes object of class IndexRecord */
    IndexRecord() noexcept
        : m_id(0)
        , m_type(static_cast<IndexType>(0))
        , m_tableId(0)
        , m_unique(true)
        , m_dataFileSize(kDefaultDataFileSize)
    {
    }

    /**
     * Initializes object of class IndexRecord.
     * @param id Index ID.
     * @param type Index type.
     * @param tableId Table ID.
     * @param unique Unique index flag.
     * @param name Index name.
     * @param columns Indexed columns.
     * @param dataFileSize Data file size.
     */
    IndexRecord(std::uint64_t id, IndexType type, std::uint64_t tableId, bool unique,
            std::string&& name, IndexColumnRegistry&& columns, std::uint32_t dataFileSize) noexcept
        : m_id(id)
        , m_type(type)
        , m_tableId(tableId)
        , m_unique(unique)
        , m_name(std::move(name))
        , m_columns(std::move(columns))
        , m_dataFileSize(dataFileSize)
    {
    }

    /**
     * Initializes object of class IndexRecord.
     * @param index Index object.
     */
    IndexRecord(const Index& index);

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

    /** Index ID */
    std::uint64_t m_id;

    /** Index type */
    IndexType m_type;

    /** Table Id */
    std::uint32_t m_tableId;

    /** Unique indication */
    bool m_unique;

    /** Index name */
    std::string m_name;

    /** List of index columns. */
    IndexColumnRegistry m_columns;

    /** Data file size */
    std::uint32_t m_dataFileSize;

    /** Structure name */
    static constexpr const char* kClassName = "IndexRecord";
};

}  // namespace siodb::iomgr::dbengine
