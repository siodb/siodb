// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "BlockRegistry.h"
#include "ColumnDataBlockCache.h"
#include "ColumnDefinitionCache.h"
#include "ColumnPtr.h"
#include "IndexPtr.h"
#include "MasterColumnRecord.h"
#include "Table.h"

// Common project headers
#include <siodb/common/proto/ColumnDataType.pb.h>

// STL headers
#include <array>
#include <map>
#include <unordered_map>

namespace siodb::iomgr::dbengine {

class ColumnDefinition;
class LobChunkHeader;

/** Database table column */
class Column : public std::enable_shared_from_this<Column> {
public:
    /** Master column data type */
    static constexpr const auto kMasterColumnDataType = COLUMN_DATA_TYPE_UINT64;

    /** Master column index description */
    static constexpr const char* kMasterColumnMainIndexDescription =
            "Indexes row identifiers contained in the master column";

    /** Master column NOT NULL constraint description */
    static constexpr const char* kMasterColumnNotNullConstraintDescription =
            "Restricts master column to non-null values";

public:
    /**
     * Initializes object of class Column for a new column.
     * @param table Table object.
     * @param spec Column specification.
     * @param firstUserTrid First user range TRID (effective only for the master columns).
     */
    Column(Table& table, ColumnSpecification&& spec, std::uint64_t firstUserTrid);

    /**
     * Initializes object of class Column for an existing column.
     * @param table Table object.
     * @param columnRecord Column registry record.
     * @param firstUserTrid First user range TRID (effective only for the master columns).
     */
    Column(Table& table, const ColumnRecord& columnRecord, std::uint64_t firstUserTrid);

    DECLARE_NONCOPYABLE(Column);

    /**
     * Returns database object.
     * @return Database object.
     */
    Database& getDatabase() const noexcept
    {
        return m_table.getDatabase();
    }

    /**
     * Returns database UUID.
     * @return Database UUID.
     */
    const Uuid& getDatabaseUuid() const noexcept
    {
        return m_table.getDatabaseUuid();
    }

    /**
     * Returns database name.
     * @return Database name.
     */
    const std::string& getDatabaseName() const noexcept
    {
        return m_table.getDatabaseName();
    }

    /**
     * Returns table object.
     * @return Table object.
     */
    Table& getTable() const noexcept
    {
        return m_table;
    }

    /**
     * Returns table ID.
     * @return Table ID.
     */
    std::uint32_t getTableId() const noexcept
    {
        return m_table.getId();
    }

    /**
     * Returns table name.
     * @return Table name.
     */
    const std::string& getTableName() const noexcept
    {
        return m_table.getName();
    }

    /**
     * Returns column ID.
     * @return Column ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns data block data area size.
     * @return Data area size in bytes.
     */
    auto getDataBlockDataAreaSize() const noexcept
    {
        return m_dataBlockDataAreaSize;
    }

    /**
     * Returns column name.
     * @return Column name.
     */
    const auto& getName() const noexcept
    {
        return m_name;
    }

    /**
     * Returns column description.
     * @return Column description.
     */
    const auto& getDescription() const noexcept
    {
        return m_description;
    }

    /**
     * Returns data type ID.
     * @return Data type ID.
     */
    auto getDataType() const noexcept
    {
        return m_dataType;
    }

    /**
     * Returns column state.
     * @return Column state
     */
    auto getState() const noexcept
    {
        return m_state;
    }

    /**
     * Returns column data directory path.
     * @return data directory path.
     */
    const auto& getDataDir() const noexcept
    {
        return m_dataDir;
    }

    /**
     * Returns display name of the column.
     * @return Display name.
     */
    std::string makeDisplayName() const;

    /**
     * Returns display code of the column.
     * @return Display code.
     */
    std::string makeDisplayCode() const;

    /**
     * Returns first user range TRID.
     * Behavior is undefined if called on the non-master column.
     * @return First user range TRID.
     */
    std::uint64_t getFirstUserTrid() const noexcept
    {
        return m_masterColumnData->m_firstUserTrid;
    }

    /**
     * Returns indication of master column.
     * @return true if this is master column, false otherwise.
     */
    bool isMasterColumn() const noexcept
    {
        return static_cast<bool>(m_masterColumnData);
    }

    /**
     * Returns master column main index if present.
     * @return Master column main index object or nullptr, if this is not master column.
     */
    IndexPtr getMasterColumnMainIndex() const noexcept
    {
        return m_masterColumnData->m_mainIndex;
    }

    /**
     * Returns column definition with given ID.
     * @param columnDefinition Column definition ID.
     * @return Column definition.
     * @throw DatabaseError if column definition doesn't exist
     */
    ColumnDefinitionPtr findColumnDefinitionChecked(std::uint64_t columnDefinitionId);

    /**
     * Returns current column definition.
     * @return Column definition.
     */
    ColumnDefinitionPtr getCurrentColumnDefinition() const;

    /**
     * Returns previous column definition.
     * @return Column definition.
     */
    ColumnDefinitionPtr getPrevColumnDefinition() const;

    /**
     * Returns current position of this column in the table.
     * @return Current postion of this column in table.
     * @throw DatabaseError, if column is not a member of table.
     */
    std::uint32_t getCurrentPosition() const
    {
        return m_table.getColumnCurrentPosition(m_id);
    }

    /**
     * Returns last block ID in the block registry.
     * @return Previous block ID in the block registry.
     */
    std::uint64_t getLastBlockId() const noexcept
    {
        return m_blockRegistry.getLastBlockId();
    }

    /**
     * Returns indication that column doesn't allow NULL values.
     * @return true is column doesn't allow NULL values, false otherwise.
     */
    bool isNotNull() const noexcept
    {
        // NOTE: This is cached value.
        return m_notNull;
    }

    /**
     * Creates new column data block.
     * @param prevBlockId Previous Block ID.
     * @param state Initial block state.
     * @return Column data block object.
     */
    ColumnDataBlockPtr createBlock(std::uint64_t prevBlockId,
            ColumnDataBlockState state = ColumnDataBlockState::kCreating);

    /**
     * Returns ID of the previous block in the chain for the given block
     * based on the information in the block registry.
     * @param blockId A block identifier.
     * @return Previous block ID in the chain.
     * @throw DatabaseError if given block doesn't exist in the block registry.
     */
    std::uint64_t findPrevBlockId(std::uint64_t blockId) const;

    /**
     * Update state of the the given block in the block registry.
     * @param blockId A block identifier.
     * @param state Block state.
     * @throw DatabaseError if given block doesn't exist in the block registry.
     */
    void updateBlockState(std::uint64_t blockId, ColumnDataBlockState state) const;

    /**
     * Read data from the data file.
     * @param addr Data address.
     * @param value Resulting value.
     * @param lobStreamsMustHoldSource Flag indicates that data source must be hold by any
     *                                 underlying LOB stream objects.
     */
    void readRecord(
            const ColumnDataAddress& addr, Variant& value, bool lobStreamsMustHoldSource = false);

    /**
     * Read master column record from the data file.
     * @param addr Data address.
     * @param record Resulting value.
     */
    void readMasterColumnRecord(const ColumnDataAddress& addr, MasterColumnRecord& record);

    /**
     * Adds new data to a column.
     * @param value A value to put. May be altered by this function.
     * @return Pair containing data address and next data address
     */
    std::pair<ColumnDataAddress, ColumnDataAddress> writeRecord(Variant&& value);

    /**
     * Adds new data to a master column.
     * @param record Master column record.
     * @return Pair containing data address and next data address
     */
    std::pair<ColumnDataAddress, ColumnDataAddress> writeMasterColumnRecord(
            const MasterColumnRecord& record);

    /**
     * Erases TRID in the master column record main index
     * @param trid Table row ID
     */
    void eraseFromMasterColumnRecordMainIndex(std::uint64_t trid);

    /**
     * Rolls back to the given data address.
     * @param addr Data address.
     * @param firstAvailableBlockId First available block ID in the chain.
     */
    void rollbackToAddress(
            const ColumnDataAddress& addr, const std::uint64_t firstAvailableBlockId);

    /**
     * Loads LOB chunk header.
     * @param blockId Data block ID.
     * @param offset Offset in the block.
     * @param[out] header Chunk header.
     * @return Offset after header.
     */
    std::uint32_t loadLobChunkHeader(
            std::uint64_t blockId, std::uint32_t offset, LobChunkHeader& header);

    /**
     * Reads data from block.
     * @param blockId Block ID.
     * @param offset Offset in block.
     * @param[out] buffer Output buffer.
     * @param bufferSize Size of buffer.
     */
    void readData(
            std::uint64_t blockId, std::uint32_t offset, void* buffer, std::size_t bufferSize);

    /**
     * Generates next TRID from the user TRID range.
     * @return Next user record TRID.
     * @throw DatabaseError if this is not master column
     *         or generated ID is out of allowed range
     */
    std::uint64_t generateNextUserTrid();

    /**
     * Generates next TRID from the system TRID range.
     * @return Next system objet record TRID.
     * @throw DatabaseError if this is not master column
     *         or generated ID is out of allowed range
     */
    std::uint64_t generateNextSystemTrid();

    /**
     * Sets last system TRID value. Used to adjust TRID counter when initializing database.
     * @param lastSystemTrid Last system TRID value.
     */
    void setLastSystemTrid(std::uint64_t lastSystemTrid);

    /**
     * Sets last system TRID value. Used by ALTER TABLE SET NEXT_TRID.
     * @param lastSystemTrid Last system TRID value.
     * @throw DatabaseError if new value is less or equal to the current value.
     */
    void setLastUserTrid(std::uint64_t lastUserTrid);

    /**
     * Generates next block ID.
     * @return Next block ID.
     */
    std::uint64_t generateNextBlockId() noexcept
    {
        return ++m_lastBlockId;
    }

    /**
     * Returns last system TRID. Assumes column is master column.
     * @return Last system TRID.
     */
    std::uint64_t getLastSystemTrid() const noexcept
    {
        return m_masterColumnData->m_tridCounters->m_lastSystemTrid;
    }

    /**
     * Returns last user TRID. Assumes column is master column.
     * @return Last user TRID.
     */
    std::uint64_t getLastUserTrid() const noexcept
    {
        return m_masterColumnData->m_tridCounters->m_lastUserTrid;
    }

    /**
     * Creates new TRID counter file.
     * @param firstUserTrid First user TRID.
     * @return File descriptor.
     */
    int createTridCountersFile(std::uint64_t firstUserTrid);

    /**
     * Loads TRID counter from the existing file.
     * @return File descriptor.
     */
    int openTridCountersFile();

    /** Loads master column main index */
    void loadMasterColumnMainIndex();

private:
    /** Data of the TRID counters */
    struct TridCounters {
        /**
         * Initializes obejct of class TridCounters.
         * @param firstUserTrid First user range TRID.
         */
        explicit TridCounters(std::uint64_t firstUserTrid) noexcept
            : m_marker(kMarker)
            , m_lastUserTrid(firstUserTrid > 0 ? firstUserTrid - 1 : 0)
            , m_lastSystemTrid(firstUserTrid < 2 ? std::numeric_limits<std::uint64_t>::max() : 0)
        {
        }

        /** Endianness marker */
        std::uint64_t m_marker;

        /** User TRID counter */
        std::atomic<std::uint64_t> m_lastUserTrid;

        /** System TRID counter */
        std::atomic<std::uint64_t> m_lastSystemTrid;

        /** TRID counter file marker value */
        static constexpr std::uint64_t kMarker = 0x1234567890abcdef;

        /** Counters data size */
        static constexpr std::uint64_t kDataSize = 24;
    };

    /** Master column specific data. */
    struct MasterColumnData {
        /**
         * Initializes object of class MasterColumnData.
         * @param parent Parent column.
         * @param createCounters Indicates that counters must be created
         * @param firstUserTrid First user range TRID.
         */
        MasterColumnData(Column& parent, bool createCounters, std::uint64_t firstUserTrid);

        /** First user range TRID */
        const std::uint64_t m_firstUserTrid;

        /** Master column main index */
        IndexPtr m_mainIndex;

        /** Memory mapped file that holds counters */
        MemoryMappedFile m_file;

        /** TRID counters */
        TridCounters* const m_tridCounters;
    };

private:
    /**
     * Returns indication that column name is master column name.
     * @return true if column name matches to master column name, false otherwise.
     */
    bool isMasterColumnName() const noexcept
    {
        return m_name == kMasterColumnName;
    }

    /**
     * Validates table.
     * @param table Table to which this column is supposed to belong to.
     * @param columnRecord Column registry record.
     * @return The same table, if it is valid.
     * @throw DatabaseError if table has different ID.
     */
    static Table& validateTable(Table& table, const ColumnRecord& columnRecord);

    /**
     * Validates column name. Requires table reference initialized.
     * @param columnName Column name.
     * @return The same column name.
     * @throw DatabaseError if column name is invalid.
     */
    std::string&& validateColumnName(std::string&& columnName) const;

    /**
     * Validates column data type. Requires table name initialized.
     * @param dataType Column data type.
     * @return The same column data type if it is valid.
     * @throw DatabaseError if column data type is invalid or not appropriate.
     */
    ColumnDataType validateColumnDataType(ColumnDataType dataType) const;

    /**
     * Checks column data consistency.
     * Throws exception if error found.
     */
    void checkDataConsistency();

    /**
     * Creates new column definition. Does not acquire access synchronization lock for the column
     * definition cache.
     * @return New column definition object.
     */
    ColumnDefinitionPtr createColumnDefinitionUnlocked();

    /**
     * Creates column definition from registry record. Does not acquire access synchronization lock
     * for the column definition cache.
     * @param columnDefinitionRecord Column definition registry record.
     * @return Column definition object.
     */
    ColumnDefinitionPtr createColumnDefinitionUnlocked(
            const ColumnDefinitionRecord& columnDefinitionRecord);

    /**
     * Loads existing column definition.
     * @param columnDefinitionId Column definition ID.
     * @return Column definition object.
     * @throw DatabaseError if column definition doesn't exist.
     */
    ColumnDefinitionPtr loadColumnDefinitionUnlocked(std::uint64_t columnDefinitionId)
    {
        return createColumnDefinitionUnlocked(
                m_table.getDatabase().findColumnDefinitionRecord(columnDefinitionId));
    }

    /**
     * Obtains existing column data block.
     * @param blockId Block ID.
     * @return Column data block object.
     */
    ColumnDataBlockPtr loadBlock(std::uint64_t blockId);

    /**
     * Selects available block or creates new one that can store at least given amount of bytes.
     * @param requiredLength Required data length.
     * @return Data block that can store at least requiredLength bytes of data.
     */
    ColumnDataBlockPtr selectAvailableBlock(std::size_t requiredLength);

    /**
     * Updates block information in available block.
     * @param block A block.
     */
    void updateAvailableBlock(const ColumnDataBlock& block);

    /**
     * Creates new block or gets next available block.
     * @param block Current block.
     * @param requiredFreeSpace Required free space in block, must be nonzero and not exceed
     *                          max data size in the block.
     * @return New block.
     */
    ColumnDataBlockPtr createOrGetNextBlock(ColumnDataBlock& block, std::size_t requiredFreeSpace);

    /**
     * Gets existing block into memory and returns cached object.
     * @param blockId Block ID.
     * @return Block object if block exists.
     * @throw DatabaseError if block doesn't exist.
     */
    ColumnDataBlockPtr findExistingBlock(std::uint64_t blockId);

    /**
     * Finds first block on disk.
     * @return First block ID or 0 if there are no blocks.
     */
    std::uint64_t findFirstBlock() const;

    /**
     * Stores binary data in the buffer.
     * Assumes column is already locked.
     * @param src Data buffer
     * @param length Data length
     * @param block Starting block.
     * @return Pair containing data address and next data address
     */
    std::pair<ColumnDataAddress, ColumnDataAddress> writeBuffer(
            const void* src, std::uint32_t length, ColumnDataBlockPtr block);

    /**
     * Stores some LOB data.
     * Assumes column is already locked.
     * @param lob LOB data stream
     * @param block Starting block.
     * @return Pair containing data address and next data address
     */
    std::pair<ColumnDataAddress, ColumnDataAddress> writeLob(
            LobStream& lob, ColumnDataBlockPtr block);

    /**
     * Loads TEXT data.
     * @param addr Data address.
     * @param[out] value Output value.
     * @param lobStreamsMustHoldSource Flag indicates that data source must be hold by any
     *                   underlying LOB stream objects.
     */
    void loadText(const ColumnDataAddress& addr, Variant& value, bool lobStreamsMustHoldSource);

    /**
     * Loads BINARY data.
     * @param addr Data address.
     * @param[out] value Output value.
     * @param lobStreamsMustHoldSource Flag indicates that data source must be hold by any
     *                   underlying LOB stream objects.
     */
    void loadBinary(const ColumnDataAddress& addr, Variant& value, bool lobStreamsMustHoldSource);

    /**
     * Loads LOB chunk header.
     * @param block Data block.
     * @param offset Offset in the block.
     * @param[out] header Chunk header.
     * @return Offset after header.
     */
    std::uint32_t loadLobChunkHeaderUnlocked(
            ColumnDataBlock& block, std::uint32_t offset, LobChunkHeader& header);

    /** Creates master column main index */
    void createMasterColumnMainIndex();

    /** Creates master column constraints */
    void createMasterColumnConstraints();

    /**
     * Writes full content of the TRID counter file.
     * @param fd File descriptor.
     * @param data TRID counters data.
     */
    void writeFullTridCounters(int fd, const TridCounters& data);

    /**
     * Constructs master column main index name.
     * @return Master column main index name.
     */
    std::string composeMasterColumnMainIndexName() const;

    /**
     * Ensures that data directory exists and initialized if required.
     * @param create Indicates that data directoty must be created.
     * @return Data directory.
     */
    std::string ensureDataDir(bool create = false) const;

    /** Creates initialization flag file. */
    void createInitializationFlagFile() const;

    /**
     * Creates master column data if applicable.
     * @param create Indicates that master column data must be created.
     * @param firstUserTrid First user range TRID.
     * @return Master column data if this is master column, nullptr otherwise.
     */
    std::unique_ptr<MasterColumnData> maybeCreateMasterColumnData(
            bool create, std::uint64_t firstUserTrid);

private:
    /** Table to which this column belongs */
    Table& m_table;

    /** Column name */
    std::string m_name;

    /** Column name */
    std::optional<std::string> m_description;

    /** Data type */
    const ColumnDataType m_dataType;

    /** Current state */
    ColumnState m_state;

    /** Column ID */
    const std::uint64_t m_id;

    /** Data block data size */
    const std::uint32_t m_dataBlockDataAreaSize;

    /** Persistent info access synchronizarion object */
    mutable std::recursive_mutex m_mutex;

    /** Column data directory */
    const std::string m_dataDir;

    /** Master column specific data */
    const std::unique_ptr<MasterColumnData> m_masterColumnData;

    /** Column definition cache */
    ColumnDefinitionCache m_columnDefinitionCache;

    /** Previous column definition */
    ColumnDefinitionPtr m_prevColumnDefinition;

    /** Current column definition */
    ColumnDefinitionPtr m_currentColumnDefinition;

    /**
     * Indicates that column doesn't allow NULL values.
     * This is cached value derived from m_currentColumnDefinition, and therefore
     * must be updated together with it.
     */
    bool m_notNull;

    /**
     * Map of available data blocks. Must be ordered map.
     * Key is block ID, value is free space in the block.
     */
    std::map<std::uint64_t, std::uint32_t> m_availableDataBlocks;

    /** Block registry */
    BlockRegistry m_blockRegistry;

    /** Last block ID */
    std::atomic<std::uint64_t> m_lastBlockId;

    /** Cached blocks */
    ColumnDataBlockCache m_blockCache;

    /** Minimum required block free spaces for various column data type */
    static const std::array<std::uint32_t, ColumnDataType_MAX> s_minRequiredBlockFreeSpaces;

    /** Well known ignorable files during consistency check */
    static const std::unordered_set<std::string> s_wellKnownIgnorableFiles;

    /** Normal column directory prefix */
    static constexpr const char* kColumnDataDirPrefix = "c";

    /** Master column directory prefix */
    static constexpr const char* kMasterColumnDataDirPrefix = "mc";

    /** Main index ID file name */
    static constexpr const char* kMainIndexIdFile = "main_index_id";

    /** TRID counter file name */
    static constexpr const char* kTridCounterFile = "trid";

    /** TRID counter migration file extension */
    static constexpr const char* kTridCounterMigrationFileExt = ".mig";

    /** Master column main index key size */
    static constexpr std::size_t kMasterColumnNameMainIndexKeySize = 8;

    /** Master column main index value size (block ID + offset) */
    static constexpr std::size_t kMasterColumnNameMainIndexValueSize = 12;

    /** Small LOB size limit */
    static constexpr std::size_t kSmallLobSizeLimit = 0x100000;

    /** Chunk free space threshold for storing LOB piece */
    static constexpr std::size_t kBlockFreeSpaceThresholdForLob = 0x100;

    /** Marker offest in the TRID counter file */
    static constexpr int kTridCounterFileMarkerOffset = 0;

    /** User TRID counter offest in the TRID counter file */
    static constexpr int kTridCounterFileUserTridOffset =
            kTridCounterFileMarkerOffset + sizeof(TridCounters::kMarker);

    /** System TRID counter offest in the TRID counter file */
    static const int kTridCounterFileSystemTridOffset = kTridCounterFileUserTridOffset + 8;

    /** Column definition cache capacity */
    static constexpr std::size_t kColumnDefinitionCacheCapacity = 10;
};

}  // namespace siodb::iomgr::dbengine
