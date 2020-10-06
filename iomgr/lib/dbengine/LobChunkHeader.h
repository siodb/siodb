// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDataAddress.h"

namespace siodb::iomgr::dbengine {

/** Header of the LOB chunk in the data file */
struct LobChunkHeader {
    /** Initializes LobChunkHeader */
    LobChunkHeader() noexcept
        : m_remainingLobLength(0)
        , m_chunkLength(0)
        , m_nextChunkBlockId(0)
        , m_nextChunkOffset(0)
    {
    }

    /**
     * Initializes LobChunkHeader.
     * @param remainingLobLength Remaining length of LOB.
     * @param chunkLength Chunk length.
     */
    LobChunkHeader(std::uint32_t remainingLobLength, std::uint32_t chunkLength) noexcept
        : m_remainingLobLength(remainingLobLength)
        , m_chunkLength(chunkLength)
        , m_nextChunkBlockId(0)
        , m_nextChunkOffset(0)
    {
    }

    /**
     * Serializes this object into a memory buffer. Doesn't check buffer size.
     * @param buffer Buffer address.
     * @return Address after the last written byte. NOTE: if buffer is too small,
     *         this function may crash.
     */
    std::uint8_t* serialize(std::uint8_t* buffer) const noexcept;

    /**
     * De-serializes object from a memory buffer.
     * @param buffer Buffer address.
     * @return Address after the last read byte or nullptr if data can't be read
     *         (data size is too small or data corrupted).
     */
    const std::uint8_t* deserialize(const std::uint8_t* buffer) noexcept;

    /** Remaining length of LOB */
    std::uint32_t m_remainingLobLength;

    /** Chunk length */
    std::uint32_t m_chunkLength;

    /** Next chunk block ID */
    std::uint64_t m_nextChunkBlockId;

    /** Next chunk offset */
    std::uint32_t m_nextChunkOffset;

    /** Maximum serialized size */
    static constexpr std::size_t kSerializedSize =
            sizeof(m_remainingLobLength) + sizeof(m_chunkLength) + sizeof(m_nextChunkBlockId)
            + sizeof(m_nextChunkOffset);
};

}  // namespace siodb::iomgr::dbengine
