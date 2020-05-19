// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../Column.h"
#include "../LobChunkHeader.h"

namespace siodb::iomgr::dbengine {

/** Column-based LOB stream. */
class ColumnLobStream : virtual public LobStream {
protected:
    /**
     * Initializes object of class ColumnLobStream.
     * @param column Column object.
     * @param addr Clob address.
     * @param holdSource Flag indicates that data source must be hold by this object.
     */
    ColumnLobStream(Column& column, const ColumnDataAddress& addr, bool holdSource);

    /**
     * Reads data from stream up to given data size.
     * @param buffer Destinaton buffer.
     * @param bufferSize Buffer size.
     * @return -1 if error occurred, 0 if EOF reached, otherwise data size read actually.
     */
    std::ptrdiff_t readInternal(void* buffer, std::size_t bufferSize);

    /** Performs rewind operation. */
    void doRewind();

protected:
    /**
     * Underlying database. In the most cases must make sure there's reference to it
     * to avoid evicting from cache, but in some cases we must not make such refernce
     * because shared pointer would not exist yet.
     */
    const DatabasePtr m_databaseHolder;

    /**
     * Underlying table. In the most cases must make sure there's reference to it
     * to avoid evicting from cache, but in some cases we must not make such refernce because
     * shared pointer would not exist yet.
     */
    const TablePtr m_tableHolder;

    /**
     * Underlying table. In the most cases must make sure there's reference to it
     * to avoid unexpected column destruction, but in some cases we must not make such refernce
     * because shared pointer would not exist yet.
     */
    const ColumnPtr m_columnHolder;

    /** Simple reference to column which will be actually used to access it. */
    Column& m_column;

    /** Starting address of CLOB */
    const ColumnDataAddress m_startingAddress;

    /** Current chunk header */
    LobChunkHeader m_chunkHeader;

    /** Current offset in block */
    std::uint32_t m_offsetInChunk;

    /** Current block */
    std::uint64_t m_blockId;

    /** Current offset in block */
    std::uint32_t m_offsetInBlock;
};

}  // namespace siodb::iomgr::dbengine
