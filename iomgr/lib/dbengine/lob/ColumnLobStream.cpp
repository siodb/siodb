// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnLobStream.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>

namespace siodb::iomgr::dbengine {

ColumnLobStream::ColumnLobStream(Column& column, const ColumnDataAddress& addr, bool holdSource)
    : LobStream(0)
    , m_databaseHolder(holdSource ? column.getDatabase().shared_from_this() : nullptr)
    , m_tableHolder(holdSource ? column.getTable().shared_from_this() : nullptr)
    , m_columnHolder(holdSource ? column.shared_from_this() : nullptr)
    , m_column(column)
    , m_startingAddress(addr)
    , m_offsetInChunk(0)
    , m_blockId(m_startingAddress.getBlockId())
    , m_offsetInBlock(m_column.loadLobChunkHeader(
              m_startingAddress.getBlockId(), m_startingAddress.getOffset(), m_chunkHeader))
{
    m_size = m_chunkHeader.m_remainingLobLength;
}

std::ptrdiff_t ColumnLobStream::readInternal(void* buffer, std::size_t bufferSize)
{
    const std::uint32_t totalBytesToRead =
            std::min(bufferSize, static_cast<std::size_t>(getRemainingSize()));
    if (totalBytesToRead == 0) return 0;
    auto remainingBytes = totalBytesToRead;
    std::uint32_t bufferOffset = 0;

    while (true) {
        // Read whatever is possible to read
        const auto availableInChunk = m_chunkHeader.m_chunkLength - m_offsetInChunk;
        if (availableInChunk > 0) {
            const auto bytesToRead = std::min(availableInChunk, remainingBytes);
            m_column.readData(m_blockId, m_offsetInBlock,
                    static_cast<std::uint8_t*>(buffer) + bufferOffset, bytesToRead);
            m_offsetInBlock += bytesToRead;
            m_offsetInChunk += bytesToRead;
            m_pos += bytesToRead;
            remainingBytes -= bytesToRead;
            if (remainingBytes == 0) return totalBytesToRead;
            bufferOffset += bytesToRead;
        }

        // Move to a next chunk
        assert(getRemainingSize() > 0);
        if (m_chunkHeader.m_nextChunkBlockId == 0) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeader,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), m_blockId,
                    m_offsetInBlock, "chunk chain unexpectedly terminated");
        }
        m_blockId = m_chunkHeader.m_nextChunkBlockId;
        m_offsetInBlock = m_chunkHeader.m_nextChunkOffset;

        // Load next chunk header and validate it.
        // Be exception-safe in regard to internal state: commit chunk header
        // and related counters only after all checks passed.
        LobChunkHeader chunkHeader;
        const auto newOffsetInBlock =
                m_column.loadLobChunkHeader(m_blockId, m_offsetInBlock, chunkHeader);
        if (chunkHeader.m_remainingLobLength != getRemainingSize()) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeader,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), m_blockId,
                    m_offsetInBlock, "subsequent chunk has unexpected remaining length");
        }
        if (chunkHeader.m_chunkLength == 0) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeader,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), m_blockId,
                    m_offsetInBlock, "subsequent chunk length is zero");
        }
        if (chunkHeader.m_nextChunkBlockId >= m_column.getLastBlockId()) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeader,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), m_blockId,
                    m_offsetInBlock, "invalid next chunk block ID in the subsequent chunk header");
        }
        if (chunkHeader.m_nextChunkOffset >= m_column.getDataBlockDataAreaSize()) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidLobChunkHeader,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), m_blockId,
                    m_offsetInBlock, "invalid next chunk offset in the subsequent chunk header");
        }
        m_chunkHeader = chunkHeader;
        m_offsetInBlock = newOffsetInBlock;
        m_offsetInChunk = 0;
    }
}

void ColumnLobStream::doRewind()
{
    m_offsetInChunk = 0;
    m_blockId = m_startingAddress.getBlockId();
    m_offsetInBlock = m_column.loadLobChunkHeader(
            m_startingAddress.getBlockId(), m_startingAddress.getOffset(), m_chunkHeader);
}

}  // namespace siodb::iomgr::dbengine
