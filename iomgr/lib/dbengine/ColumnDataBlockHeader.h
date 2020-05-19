// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDataBlockState.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/utils/Uuid.h>

// CRT headers
#include <cstdint>

// STL headers
#include <array>

namespace siodb::iomgr::dbengine {

/**
 * Full column block identifier.
 * POD type.
 */
struct FullColumnDataBlockId {
    /** Equality operator */
    bool operator==(const FullColumnDataBlockId& other) const noexcept
    {
        return m_databaseUuid == other.m_databaseUuid && m_tableId == other.m_tableId
               && m_columnId == other.m_columnId && m_blockId == other.m_blockId;
    }

    /** Non-equality operator */
    bool operator!=(const FullColumnDataBlockId& other) const noexcept
    {
        return m_databaseUuid != other.m_databaseUuid || m_tableId != other.m_tableId
               || m_columnId != other.m_columnId || m_blockId != other.m_blockId;
    }

    /** Database UUID */
    Uuid m_databaseUuid;

    /** Table ID */
    std::uint32_t m_tableId;

    /** Column ID */
    std::uint64_t m_columnId;

    /** Block ID */
    std::uint64_t m_blockId;

    /** Serialized size */
    static constexpr const std::size_t kSerializedSize = sizeof(m_databaseUuid.data)
                                                         + sizeof(m_tableId) + sizeof(m_columnId)
                                                         + sizeof(m_blockId);
};

/**
 * Persistent information about column block.
 * POD type.
 */
struct ColumnDataBlockHeader {
    /** Digest length */
    static constexpr const unsigned kDigestLength = 64;

    /** Digest type */
    using Digest = std::array<std::uint8_t, kDigestLength>;

    /** Initializes ColumnDataBlockHeader */
    ColumnDataBlockHeader()
        : m_version(kCurrentVersion)
        , m_fullColumnDataBlockId {utils::getZeroUuid(), 0, 0, 0}
        , m_dataAreaOffset(kDefaultDataAreaOffset)
        , m_dataAreaSize(kDefaultDataFileDataAreaSize)
        , m_nextDataOffset(0)
        , m_commitedDataOffset(0)
        , m_fillTimestamp(0)
        , m_digest {0}
    {
    }

    /**
     * Initializes ColumnDataBlockHeader.
     * @param databaseUuid Database UUID.
     * @param tableId Table ID.
     * @param columnId Column ID.
     * @param blockId Column block ID in this column.
     * @param dataAreaSize Data area size.
     */
    ColumnDataBlockHeader(const Uuid& databaseUuid, std::uint32_t tableId, std::uint64_t columnId,
            std::uint64_t blockId, std::uint32_t dataAreaSize)
        : m_version(kCurrentVersion)
        , m_fullColumnDataBlockId {databaseUuid, tableId, columnId, blockId}
        , m_dataAreaOffset(kDefaultDataAreaOffset)
        , m_dataAreaSize(dataAreaSize)
        , m_nextDataOffset(0)
        , m_commitedDataOffset(0)
        , m_fillTimestamp(0)
        , m_digest {0}
    {
    }

    /**
     * Serializes this object into memory buffer.
     * @param buffer Memory buffer address.
     * @return Address of byte after last written byte.
     */
    std::uint8_t* serialize(std::uint8_t* buffer) const noexcept;

    /**
     * De-serializes data from memory buffer into this object.
     * @param buffer Memory buffer address.
     * @return Address of byte after last read byte on success, nullptr otherwise.
     */
    const std::uint8_t* deserialize(const std::uint8_t* buffer) noexcept;

    /** Column block info version */
    std::uint32_t m_version;

    /** Full block identifier */
    FullColumnDataBlockId m_fullColumnDataBlockId;

    /** Previous block ID */
    std::uint64_t m_prevBlockId;

    /** Offset of the data area start */
    std::uint32_t m_dataAreaOffset;

    /** Size of the data area */
    std::uint32_t m_dataAreaSize;

    /** Offset of next data record in the file relative to data area start */
    std::uint32_t m_nextDataOffset;

    /** Offset of committed data */
    std::uint32_t m_commitedDataOffset;

    /**
     * Fill timestamp (when block became full).
     * Nonzero value indicates that block is full.
     */
    std::uint64_t m_fillTimestamp;

    /** Previous block digest (when it became full) */
    Digest m_prevBlockDigest;

    /** Block digest (when it became full) */
    Digest m_digest;

    /** Current column block info version */
    static constexpr const std::uint32_t kCurrentVersion = 1;

    /** Serialized size */
    static constexpr const std::size_t kSerializedSize =
            sizeof(m_version) + FullColumnDataBlockId::kSerializedSize + sizeof(m_prevBlockId)
            + sizeof(m_dataAreaOffset) + sizeof(m_dataAreaSize) + sizeof(m_nextDataOffset)
            + sizeof(m_commitedDataOffset) + sizeof(m_fillTimestamp) + sizeof(m_digest);

    /** Standard data area offset for the current data file format version */
    static constexpr std::size_t kDefaultDataAreaOffset = kDataFileHeaderSize;

    /** Previous block digest for initial block */
    static constexpr Digest kInitialPrevBlockDigest {0};
};

}  // namespace siodb::iomgr::dbengine
