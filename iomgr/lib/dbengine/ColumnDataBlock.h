// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Column.h"
#include "ColumnDataBlockHeader.h"
#include "ColumnDataBlockPtr.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/utils/FileDescriptorGuard.h>
#include <siodb/common/utils/HelperMacros.h>

namespace siodb::iomgr::dbengine {

/** Column data block */
class ColumnDataBlock {
public:
    /** Block file prefix */
    static constexpr const char* kBlockFilePrefix = "b";

public:
    /**
     * Initializes object of class ColumnDataBlock for a new block.
     * @param column Column information.
     * @param prevBlockId Previous block ID.
     * @param state Block state.
     */
    ColumnDataBlock(Column& column, std::uint64_t prevBlockId, ColumnDataBlockState state);

    /**
     * Initializes object of class ColumnDataBlock for an existing block.
     * @param column Column information.
     * @param id Column block ID in this column.
     */
    ColumnDataBlock(Column& column, std::uint64_t id);

    /** De-initializes object of class ColumnDataBlock */
    ~ColumnDataBlock();

    DECLARE_NONCOPYABLE(ColumnDataBlock);

    /**
     * Returns column object.
     * @return Column object.
     */
    Column& getColumn() const noexcept
    {
        return m_column;
    }

    /**
     * Returns block ID.
     * @return Block ID.
     */
    auto getId() const noexcept
    {
        return m_header.m_fullColumnDataBlockId.m_blockId;
    }

    /**
     * Returns previous block ID.
     * @return Previous block ID.
     */
    auto getPrevBlockId() const noexcept
    {
        return m_prevBlockId;
    }

    /**
     * Returns block state.
     * @return Block state.
     */
    auto getState() const noexcept
    {
        return m_state;
    }

    /**
     * Sets block state.
     * @param state A new block state.
     */
    void setState(ColumnDataBlockState state) noexcept
    {
        m_state = state;
    }

    /**
     * Returns column block digest.
     * @return Block digest from header.
     */
    const auto& getDigest() const noexcept
    {
        return m_header.m_digest;
    }

    /**
     * Returns data file path.
     * @return data file path.
     */
    const auto& getDataFilePath() const noexcept
    {
        return m_dataFilePath;
    }

    /**
     * Returns display name of the block.
     * @return Display name.
     */
    std::string getDisplayName() const;

    /**
     * Returns display code of the block.
     * @return Display code.
     */
    std::string getDisplayCode() const;

    /**
     * Returns indication that block is modified.
     * @return true if block is modified, false otherwise.
     */
    bool isModified() const noexcept
    {
        return m_headerModified || m_dataModified;
    }

    /**
     * Returns next data position.
     * @return Next data item position.
     */
    std::uint32_t getNextDataPos() const noexcept
    {
        return m_header.m_nextDataOffset;
    }

    /**
     * Returns amount of free data space available .
     * @return Next data item position.
     */
    std::uint32_t getFreeDataSpace() const noexcept
    {
        return m_column.getDataBlockDataAreaSize() - m_header.m_nextDataOffset;
    }

    /**
     * Sets next data position.
     * @param nextDataPos Next data item position.
     */
    void setNextDataPos(std::uint32_t nextDataPos) noexcept
    {
        m_header.m_nextDataOffset = nextDataPos;
    }

    std::uint32_t getDataFileSize() const noexcept
    {
        return m_column.getDataBlockDataAreaSize() + ColumnDataBlockHeader::kDefaultDataAreaOffset;
    }

    /**
     * Increase next data position by specified number of bytes.
     * @param n Number of bytes.
     * @return New next data item position.
     */
    std::uint32_t incNextDataPos(std::uint32_t n) noexcept
    {
        m_header.m_nextDataOffset += n;
        return m_header.m_nextDataOffset;
    }

    /** Resets fill timestemp to zero */
    void resetFillTimestamp() noexcept
    {
        m_header.m_fillTimestamp = 0;
    }

    /** Saves header */
    void saveHeader() const;

    /**
     * Reads data from the data file at a given position.
     * @param[out] data A data.
     * @param length Data length.
     * @param pos Data position.
     */
    void readData(void* data, std::size_t length, std::uint32_t pos) const;

    /**
     * Writes data to the data file at a given position.
     * @param data A data.
     * @param length Data length.
     * @param pos Data position.
     */
    void writeData(const void* data, std::size_t length, std::uint32_t pos);

    /**
     * Writes data to the data file at the current data position.
     * @param data A data.
     * @param length Data length.
     */
    void writeData(const void* data, std::size_t length)
    {
        writeData(data, length, m_header.m_nextDataOffset);
    }

    /**
     * Writes buffer to the data file at a given position.
     * @param buffer A buffer.
     * @param pos Data position.
     */
    void writeData(const BinaryValue& buffer, std::uint32_t pos)
    {
        writeData(buffer.data(), buffer.size(), pos);
    }

    /**
     * Writes buffer to the data file at a current position.
     * @param buffer A buffer.
     */
    void writeData(const BinaryValue& buffer)
    {
        writeData(buffer.data(), buffer.size(), m_header.m_nextDataOffset);
    }

    /**
     * Finalizes block - put fill timestamp and adds data digest.
     * @param prevBlockDigest Digest of a previous block.
     */
    void finalize(const ColumnDataBlockHeader::Digest& prevBlockDigest);

    /**
     * Computes block digest. Assumes block has data.
     * @param prevBlockDigest Digest of a previous block.
     * @param[out] blockDigest Computed current block digest.
     */
    void computeDigest(const ColumnDataBlockHeader::Digest& prevBlockDigest,
            ColumnDataBlockHeader::Digest& blockDigest) const;

private:
    /**
     * Creates new data file for the specified column block.
     * Fails if the file already exists.
     * @param columnDataBlock Column block info.
     * @return File object.
     * @throw DatabaseError if operation fails for any reason
     */
    io::FilePtr createDataFile() const;

    /**
     * Opens data file for the specified column block. Fails if the file doesn't exist.
     * @param columnDataBlock Column block info.
     * @return File object.
     * @throw DatabaseError if operation fails for any reason
     */
    io::FilePtr openDataFile() const;

    /**
     * Constructs data file path.
     * @return Data file path.
     */
    std::string makeDataFilePath() const;

    /** Loads header */
    void loadHeader();

private:
    /** Column to which this data block belongs */
    Column& m_column;

    /** Block header */
    ColumnDataBlockHeader m_header;

    /** Cached previous block ID */
    const std::uint64_t m_prevBlockId;

    /** Column block data file path */
    const std::string m_dataFilePath;

    /** Block file */
    io::FilePtr m_file;

    /** Column block state */
    ColumnDataBlockState m_state;

    /** Indicates that header of the block is been modified */
    mutable bool m_headerModified;

    /** Indicates that data of the block is been modified */
    bool m_dataModified;

    /** Data file header prototype */
    static const BinaryValue m_dataFileHeaderProto;
};

}  // namespace siodb::iomgr::dbengine
