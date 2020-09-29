// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/utils/Uuid.h>
#include <siodb/iomgr/shared/dbengine/IndexType.h>

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/**
 * Full index identifier.
 * POD type.
 */
struct FullIndexId {
    /** Equality operator */
    bool operator==(const FullIndexId& other) const noexcept
    {
        return m_databaseUuid == other.m_databaseUuid && m_tableId == other.m_tableId
               && m_indexId == other.m_indexId;
    }

    /** Non-equality operator */
    bool operator!=(const FullIndexId& other) const noexcept
    {
        return !(*this == other);
    }

    /** Database UUID */
    Uuid m_databaseUuid;

    /** Table ID */
    std::uint32_t m_tableId;

    /** Index ID */
    std::uint64_t m_indexId;

    /** Serialized size */
    static constexpr const std::size_t kSerializedSize =
            sizeof(m_databaseUuid.data) + sizeof(m_tableId) + sizeof(m_indexId);
};

/** Header of the index file, common part */
struct IndexFileHeaderBase {
    /** Index information version */
    std::uint32_t m_version;

    /** Index type */
    const IndexType m_indexType;

    /** Full index ID */
    FullIndexId m_fullIndexId;

protected:
    /**
     * Initializes instance of class IndexFileHeaderBase.
     * @param indexType Index type.
     */
    IndexFileHeaderBase(IndexType indexType) noexcept
        : m_version(kCurrentVersion)
        , m_indexType(indexType)
        , m_fullIndexId {utils::getZeroUuid(), 0, 0}
    {
    }

    /**
     * Initializes instance of class IndexFileHeaderBase.
     * @param databaseUuid Database UUID.
     * @param tableId Table ID.
     * @param indexId Index ID.
     * @param indexType Index type.
     */
    IndexFileHeaderBase(const Uuid& databaseUuid, std::uint32_t tableId, std::uint64_t indexId,
            IndexType indexType) noexcept
        : m_version(kCurrentVersion)
        , m_indexType(indexType)
        , m_fullIndexId {databaseUuid, tableId, indexId}
    {
    }

    /** Equality operator */
    bool operator==(const IndexFileHeaderBase& other) const noexcept
    {
        return m_version == other.m_version && m_indexType == other.m_indexType
               && m_fullIndexId == other.m_fullIndexId;
    }

    /** Non-equality operator */
    bool operator!=(const IndexFileHeaderBase& other) const noexcept
    {
        return !(*this == other);
    }

    /**
     * Serializes this object to buffer.
     * @return Address after a last written byte.
     */
    std::uint8_t* serialize(std::uint8_t* buffer) const noexcept;

    /**
     * De-serializes this object from buffer.
     * @param buffer A buffer.
     * @return Address after a last read byte.
     */
    const std::uint8_t* deserialize(const std::uint8_t* buffer) noexcept;

    /** Serialized size */
    static constexpr std::size_t kSerializedSize = FullIndexId::kSerializedSize + 1;

    /** Current version of common part */
    static constexpr std::uint32_t kCurrentVersion = 1;
};

}  // namespace siodb::iomgr::dbengine
