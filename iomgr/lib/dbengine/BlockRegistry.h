// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnDataBlockState.h"

// Common project headers
#include <siodb/common/stl_ext/lru_cache.h>
#include <siodb/common/utils/FDGuard.h>

// STL headers
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace siodb::iomgr::dbengine {

class Column;

/** Registry of block files in a column */
class BlockRegistry {
public:
    /**
     * Initializes object of class BlockRegistry.
     * @param column Column object.
     * @param create Indicates that block registry must be created.
     */
    explicit BlockRegistry(const Column& column, bool create = false);

    /**
     * Returns last block ID in the registry.
     * @return Last block ID.
     */
    std::uint64_t getLastBlockId() const noexcept
    {
        return m_lastBlockId;
    }

    /**
     * Returns previous block ID for a given block.
     * @param blockId Block ID.
     * @return Parent block ID or zero if there is no parent block.
     */
    std::uint64_t findPrevBlockId(std::uint64_t blockId) const;

    /**
     * Populates list of next block IDs for a given block.
     * @param blockId Block ID.
     * @return List of next blocks.
     */
    std::vector<std::uint64_t> findNextBlockIds(std::uint64_t blockId) const;

    /**
     * Records new block and next block if applicable.
     * @param blockId Block ID.
     * @param parentBlockId Parent block ID.
     * @param state Block state.
     */
    void recordBlockAndNextBlock(std::uint64_t blockId, std::uint64_t parentBlockId,
            ColumnDataBlockState state = ColumnDataBlockState::kCreating);

    /**
     * Records new block.
     * @param blockId Block ID.
     * @param parentBlockId Parent block ID.
     * @param state Block state.
     */
    void recordBlock(std::uint64_t blockId, std::uint64_t parentBlockId,
            ColumnDataBlockState state = ColumnDataBlockState::kCreating);

    /**
     * Updated block state.
     * @param blockId Block ID.
     * @param state New block state.
     */
    void updateBlockState(std::uint64_t blockId, ColumnDataBlockState state) const;

    /**
     * Adds next block record.
     * @param blockId Block ID.
     * @param nextBlockId Next block ID.
     */
    void addNextBlock(std::uint64_t blockId, std::uint64_t nextBlockId);

private:
    /** Next block list record */
    struct NextBlockListRecord {
        /**
         * Serializes file header into buffer.
         * @param buffer A buffer.
         * @return Address of byte after a last written one.
         */
        std::uint8_t* serialize(std::uint8_t* buffer) const noexcept;

        /**
         * De-serializes file header from buffer.
         * @param buffer A buffer.
         * @return Address of byte after a last written one.
         */
        const std::uint8_t* deserialize(const std::uint8_t* buffer) noexcept;

        /** Block file ID, 0 means no next block */
        uint64_t m_blockId;

        /** Next block file ID location: offset in the data file */
        uint32_t m_nextBlockListFileOffset;

        /** Offset of block ID field */
        static constexpr std::uint32_t kBlockIdSerializedFieldOffset = 0;

        /** Offset of next block list file offset field */
        static constexpr std::uint32_t kNextBlockListFileOffsetSerializedFieldOffset =
                kBlockIdSerializedFieldOffset + sizeof(m_blockId);

        /** Serialized size */
        static constexpr std::size_t kSerializedSize =
                kNextBlockListFileOffsetSerializedFieldOffset + sizeof(m_nextBlockListFileOffset);
    };

    /** Block list record */
    struct BlockListRecord {
        /**
         * Serializes file header into buffer.
         * @param buffer A buffer.
         * @return Address of byte after a last written one.
         */
        std::uint8_t* serialize(std::uint8_t* buffer) const noexcept;

        /**
         * De-serializes file header from buffer.
         * @param buffer A buffer.
         * @return Address of byte after a last written one.
         */
        const std::uint8_t* deserialize(const std::uint8_t* buffer) noexcept;

        /** Block ID */
        std::uint64_t m_blockId;

        /** Block state */
        ColumnDataBlockState m_blockState;

        /** Previous block ID or 0 for the first block in a chain */
        std::uint64_t m_prevBlockId;

        /** First next block file ID location: offset in the data file */
        std::uint32_t m_firstNextBlockListFileOffset;

        /** Last next block file ID location: offset in the data file */
        std::uint32_t m_lastNextBlockListFileOffset;

        /** Offset of "active" field */
        static constexpr std::uint32_t kBlockPresentSerializedFieldOffset = 0;

        /** Offset of block state field */
        static constexpr std::uint32_t kBlockStateSerializedFieldOffset =
                kBlockPresentSerializedFieldOffset + 1;

        /** Offset of previous block ID field */
        static constexpr std::uint32_t kPrevBlockIdSerializedFieldOffset =
                kBlockStateSerializedFieldOffset + 1;

        /** Offset of first next block file offset field */
        static constexpr std::uint32_t kFirstNextBlockListFileOffsetSerializedFieldOffset =
                kPrevBlockIdSerializedFieldOffset + sizeof(m_prevBlockId);

        /** Offset of last next block file offset field */
        static constexpr std::uint32_t kLastNextBlockListFileOffsetSerializedFieldOffset =
                kFirstNextBlockListFileOffsetSerializedFieldOffset
                + sizeof(m_firstNextBlockListFileOffset);

        /** Serialized size */
        static constexpr std::size_t kSerializedSize =
                kLastNextBlockListFileOffsetSerializedFieldOffset
                + sizeof(m_lastNextBlockListFileOffset);
    };

private:
    /** Creates new data files */
    void createDataFiles();

    /** Opens existing data files */
    void openDataFiles();

    /**
     * Loads block list file record for the specified block.
     * @param blockId Block ID.
     * @param[out] record A record.
     */
    void loadRecord(std::uint64_t blockId, BlockListRecord& record) const;

    /**
     * Checks block record presence. Throws exception if block doesn't exist.
     * @param blockId Block ID.
     * @return Block record offset.
     */
    off_t checkBlockRecordPresent(std::uint64_t blockId) const;

    /**
     * Ensures that data directory exists and initialized if required.
     * @param dataDir Data directory.
     * @param create Indicates that data directoty must be created.
     * @return Data directory.
     */
    std::string&& ensureDataDir(std::string&& dataDir, bool create) const;

    /** Creates initialization flag file. */
    void createInitializationFlagFile() const;

private:
    /** Column object */
    const Column& m_column;

    /** Data directory */
    const std::string m_dataDir;

    /** Block list file */
    FDGuard m_blockListFile;

    /** Next block list file */
    FDGuard m_nextBlockListFile;

    /** Block list file size */
    std::uint64_t m_blockListFileSize;

    /** Next block list file size */
    std::uint64_t m_nextBlockListFileSize;

    /** Last block ID */
    std::uint64_t m_lastBlockId;

    /** Block registry subdirectory */
    static constexpr const char* kBlockRegistryDir = "breg";

    /** Block list data file cache capacity */
    static constexpr std::size_t kBlockListDataFileCacheSize = 64;

    /** Next block list data file cache capacity */
    static constexpr std::size_t kNextBlockListDataFileCacheSize = 64;

    /** Initialization flag file name */
    static constexpr const char* kInitializationFlagFile = "initialized";

    /** Block list data file name prefix */
    static constexpr const char* kBlockListFileName = "blist";

    /** Next block list data file name prefix */
    static constexpr const char* kNextBlockListFileName = "nblist";
};

}  // namespace siodb::iomgr::dbengine
