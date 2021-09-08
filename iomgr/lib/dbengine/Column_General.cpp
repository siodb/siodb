// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Column.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ColumnDataBlock.h"
#include "ColumnDefinitionConstraintList.h"
#include "IndexColumn.h"
#include "LobChunkHeader.h"
#include "ThrowDatabaseError.h"
#include "lob/ColumnBlobStream.h"
#include "lob/ColumnClobStream.h"
#include "uli/UInt64UniqueLinearIndex.h"

// Common project headers
#include <siodb/common/config/SiodbDataFileDefs.h>
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/Base128VariantEncoding.h>
#include <siodb/common/utils/ByteOrder.h>
#include <siodb/common/utils/FSUtils.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>
#include <siodb/iomgr/shared/dbengine/parser/expr/ConstantExpression.h>

// Boost headers
#include <boost/format.hpp>

namespace siodb::iomgr::dbengine {

const std::array<std::uint32_t, ColumnDataType_MAX> Column::s_minRequiredBlockFreeSpaces {
        1,  // COLUMN_DATA_TYPE_BOOL
        1,  // COLUMN_DATA_TYPE_INT8
        1,  // COLUMN_DATA_TYPE_UINT8
        2,  // COLUMN_DATA_TYPE_INT16
        2,  // COLUMN_DATA_TYPE_UINT16
        4,  // COLUMN_DATA_TYPE_INT32
        4,  // COLUMN_DATA_TYPE_UINT32
        8,  // COLUMN_DATA_TYPE_INT64
        8,  // COLUMN_DATA_TYPE_UINT64
        4,  // COLUMN_DATA_TYPE_FLOAT
        8,  // COLUMN_DATA_TYPE_DOUBLE
        10,  // COLUMN_DATA_TYPE_TEXT (LOB header size)
        10,  // COLUMN_DATA_TYPE_BINARY (LOB header size)
        RawDateTime::kMaxSerializedSize  // COLUMN_DATA_TYPE_TIMESTAMP
        // TODO: Support more data types in this mapping
        // COLUMN_DATA_TYPE_DATE
        // COLUMN_DATA_TYPE_TIME
        // COLUMN_DATA_TYPE_TIME_WITH_TZ
        // COLUMN_DATA_TYPE_TIMESTAMP_WITH_TZ
        // COLUMN_DATA_TYPE_DATE_INTERVAL
        // COLUMN_DATA_TYPE_TIME_INTERVAL
        // COLUMN_DATA_TYPE_STRUCT
        // COLUMN_DATA_TYPE_XML
        // COLUMN_DATA_TYPE_JSON
        // COLUMN_DATA_TYPE_UUID
};

const std::unordered_set<std::string> Column::s_wellKnownIgnorableFiles {
        kInitializationFlagFile,
        Column::kMainIndexIdFile,
        Column::kTridCounterFile,
};

Column::Column(Table& table, ColumnSpecification&& spec, std::uint64_t firstUserTrid)
    : m_table(table)
    , m_name(validateColumnName(std::move(spec.m_name)))
    , m_description(std::move(spec.m_description))
    , m_dataType(validateColumnDataType(spec.m_dataType))
    , m_id(getDatabase().generateNextColumnId(m_table.isSystemTable()))
    , m_dataBlockDataAreaSize(spec.m_dataBlockDataAreaSize)
    , m_dataDir(ensureDataDir(true))
    , m_masterColumnData(maybeCreateMasterColumnData(true, firstUserTrid))
    , m_columnDefinitionCache(kColumnDefinitionCacheCapacity)
    , m_currentColumnDefinition(createColumnDefinitionUnlocked())
    , m_notNull(false)
    , m_blockRegistry(*this, true)
    , m_lastBlockId(m_blockRegistry.getLastBlockId())
    , m_blockCache(getDatabase().getInstance().getBlockCacheCapacity())
{
    if (isMasterColumn()) {
        if (!spec.m_constraints.empty()) {
            throwDatabaseError(
                    IOManagerMessageId::kErrorExplicitConstraintsForMasterColumnProhibited,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id);
        }

        createMasterColumnConstraints();
        m_currentColumnDefinition->markClosedForModification();

        createMasterColumnMainIndex();
        getDatabase().registerIndex(*m_masterColumnData->m_mainIndex);
    } else {
        for (auto& constraintSpec : spec.m_constraints) {
            BinaryValue serializedConstraintExpression;
            if (constraintSpec.m_expression) {
                serializedConstraintExpression.resize(
                        constraintSpec.m_expression->getSerializedSize());
                constraintSpec.m_expression->serializeUnchecked(
                        serializedConstraintExpression.data());
            }
            const auto constraintDefinition =
                    getDatabase().findOrCreateConstraintDefinition(m_table.isSystemTable(),
                            constraintSpec.m_type, serializedConstraintExpression, m_id);
            m_currentColumnDefinition->addConstraint(
                    m_table.createConstraint(std::move(constraintSpec.m_name), constraintDefinition,
                            this, std::move(constraintSpec.m_description)));
        }
        m_currentColumnDefinition->markClosedForModification();
    }

    m_notNull = m_currentColumnDefinition->isNotNull();

    createInitializationFlagFile();
}

Column::Column(Table& table, const ColumnRecord& columnRecord, std::uint64_t firstUserTrid)
    : m_table(validateTable(table, columnRecord))
    , m_name(validateColumnName(std::string(columnRecord.m_name)))
    , m_description(columnRecord.m_description)
    , m_dataType(validateColumnDataType(columnRecord.m_dataType))
    , m_id(columnRecord.m_id)
    , m_dataBlockDataAreaSize(columnRecord.m_dataBlockDataAreaSize)
    , m_dataDir(ensureDataDir())
    , m_masterColumnData(maybeCreateMasterColumnData(false, firstUserTrid))
    , m_columnDefinitionCache(kColumnDefinitionCacheCapacity)
    , m_currentColumnDefinition(findColumnDefinitionChecked(
              getDatabase().findLatestColumnDefinitionIdForColumn(m_table.getId(), m_id)))
    , m_notNull(m_currentColumnDefinition->isNotNull())
    , m_blockRegistry(*this)
    , m_lastBlockId(m_blockRegistry.getLastBlockId())
    , m_blockCache(table.getDatabase().getInstance().getBlockCacheCapacity())
{
    checkDataConsistency();
}

std::string Column::makeDisplayName() const
{
    std::ostringstream oss;
    oss << '\'' << getDatabaseName() << "'.'" << m_table.getName() << "'.'" << m_name << '\'';
    return oss.str();
}

std::string Column::makeDisplayCode() const
{
    std::ostringstream oss;
    oss << getDatabaseUuid() << '.' << m_table.getId() << '.' << m_id;
    return oss.str();
}

ColumnDefinitionPtr Column::findColumnDefinitionChecked(std::uint64_t columnDefinitionId)
{
    std::lock_guard lock(m_mutex);
    const auto cachedColumnDef = m_columnDefinitionCache.get(columnDefinitionId);
    if (cachedColumnDef) return *cachedColumnDef;
    return loadColumnDefinitionUnlocked(columnDefinitionId);
}

ColumnDefinitionPtr Column::getCurrentColumnDefinition() const
{
    std::lock_guard lock(m_mutex);
    return m_currentColumnDefinition;
}

ColumnDefinitionPtr Column::getPrevColumnDefinition() const
{
    std::lock_guard lock(m_mutex);
    return m_prevColumnDefinition;
}

void Column::eraseFromMasterColumnMainIndex(std::uint64_t trid)
{
    // Check that this is master column
    if (!isMasterColumn()) {
        throwDatabaseError(IOManagerMessageId::kErrorNotMasterColumn, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id);
    }

    std::lock_guard lock(m_mutex);

    // Update main index
    std::uint8_t indexKey[8];
    ::pbeEncodeUInt64(trid, indexKey);
    m_masterColumnData->m_mainIndex->erase(indexKey);
}

void Column::rollbackToAddress(
        const ColumnDataAddress& addr, const std::uint64_t firstAvailableBlockId)
{
    std::lock_guard lock(m_mutex);

    // Check first available data block
    if (m_availableDataBlocks.count(firstAvailableBlockId) == 0) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidCurrentDataBlock, getDatabaseName(),
                m_table.getName(), m_name, firstAvailableBlockId, getDatabaseUuid(),
                m_table.getId(), m_id);
    }

    // Check rollback block ID
    if (addr.getBlockId() > firstAvailableBlockId) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidRollbackDataBlockPosition,
                getDatabaseName(), m_table.getName(), m_name, addr.getBlockId(), getDatabaseUuid(),
                m_table.getId(), m_id, addr.getOffset(), firstAvailableBlockId);
    }

    // Check that target block really exists
    auto block = loadBlock(addr.getBlockId());
    if (!block) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockDoesNotExist, getDatabaseName(),
                m_table.getName(), m_name, addr.getBlockId(), getDatabaseUuid(), m_table.getId(),
                m_id);
    }

    // Validate offset
    if (addr.getOffset() >= m_dataBlockDataAreaSize) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDataBlockPosition, getDatabaseName(),
                m_table.getName(), getName(), addr.getBlockId(), getDatabaseUuid(), m_table.getId(),
                m_id, addr.getOffset());
    }

    // Walk through blocks
    auto currentBlockId = firstAvailableBlockId;
    while (currentBlockId != addr.getBlockId()) {
        // Load block
        block = loadBlock(currentBlockId);
        if (!block) {
            throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockDoesNotExist,
                    getDatabaseName(), m_table.getName(), m_name, addr.getBlockId(),
                    getDatabaseUuid(), m_table.getId(), m_id);
        }

        // Adjust block metadata
        block->setNextDataPos(0);
        block->resetFillTimestamp();
        block->writeHeader();

        // Update block free space info
        updateAvailableBlock(*block);

        // Move to next block
        currentBlockId = block->getPrevBlockId();
        if (currentBlockId == 0) {
            throwDatabaseError(IOManagerMessageId::kErrorUnreachableRollbackDataBlockPosition,
                    getDatabaseName(), m_table.getName(), m_name, addr.getBlockId(),
                    getDatabaseUuid(), m_table.getId(), m_id);
        }
    }

    // Adjust block metadata
    block->setNextDataPos(addr.getOffset());
    if (block->getId() != firstAvailableBlockId) {
        block->resetFillTimestamp();
        block->writeHeader();
    }

    updateAvailableBlock(*block);
}

std::uint32_t Column::loadLobChunkHeader(
        std::uint64_t blockId, std::uint32_t offset, LobChunkHeader& header)
{
    std::lock_guard lock(m_mutex);
    auto block = findExistingBlock(blockId);
    return loadLobChunkHeaderUnlocked(*block, offset, header);
}

void Column::readData(
        std::uint64_t blockId, std::uint32_t offset, void* buffer, std::size_t bufferSize)
{
    std::lock_guard lock(m_mutex);
    auto block = findExistingBlock(blockId);
    block->readData(buffer, bufferSize, offset);
}

std::uint64_t Column::generateNextUserTrid()
{
    if (!m_masterColumnData) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotGenerateUserTridUsingNonMasterColumn,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id);
    }

    if (m_masterColumnData->m_tridCounters->m_lastUserTrid
            == std::numeric_limits<std::uint64_t>::max()) {
        throwDatabaseError(
                IOManagerMessageId::kErrorUserTridExhausted, getDatabaseName(), m_table.getName());
    }

    return ++m_masterColumnData->m_tridCounters->m_lastUserTrid;
}

std::uint64_t Column::generateNextSystemTrid()
{
    if (!m_masterColumnData) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotGenerateSystemTridUsingNonMasterColumn,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id);
    }

    if (m_masterColumnData->m_tridCounters->m_lastSystemTrid
            == m_masterColumnData->m_firstUserTrid - 1) {
        throwDatabaseError(
                IOManagerMessageId::kErrorSystemTridExhausted, getDatabaseName(), m_name);
    }

    return ++m_masterColumnData->m_tridCounters->m_lastSystemTrid;
}

void Column::setLastSystemTrid(std::uint64_t lastSystemTrid)
{
    if (lastSystemTrid >= m_masterColumnData->m_firstUserTrid) {
        throwDatabaseError(
                IOManagerMessageId::kErrorSystemTridExhausted, getDatabaseName(), m_name);
    }
    m_masterColumnData->m_tridCounters->m_lastSystemTrid = lastSystemTrid;
}

void Column::setLastUserTrid(std::uint64_t lastUserTrid)
{
    if (lastUserTrid <= m_masterColumnData->m_tridCounters->m_lastUserTrid) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidLastUserTrid, getDatabaseName(),
                getTableName(), getDatabaseUuid(), getTableId(), lastUserTrid);
    }
    m_masterColumnData->m_tridCounters->m_lastUserTrid = lastUserTrid;
}

int Column::createTridCountersFile(std::uint64_t firstUserTrid)
{
    const auto tridCounterFilePath = utils::constructPath(m_dataDir, kTridCounterFile);
    FDGuard fd(::open(tridCounterFilePath.c_str(), O_CREAT | O_RDWR | O_DSYNC | O_CLOEXEC,
            kDataFileCreationMode));
    if (!fd.isValidFd()) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateTridCountersFile,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, errorCode, std::strerror(errorCode));
    }
    TridCounters data(firstUserTrid);
    writeFullTridCounters(fd.getFD(), data);
    return fd.release();
}

int Column::openTridCountersFile()
{
    const auto tridCounterFilePath = utils::constructPath(m_dataDir, kTridCounterFile);
    FDGuard fd(::open(tridCounterFilePath.c_str(), O_RDWR | O_DSYNC | O_CLOEXEC));
    if (!fd.isValidFd()) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenTridCounterFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode));
    }
    TridCounters data(0);
    const auto n = ::readExact(fd.getFD(), &data, sizeof(data), kIgnoreSignals);
    if (n != sizeof(data)) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadTridCounterFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode), sizeof(data), n);
    }
    if (data.m_marker != TridCounters::kMarker) {
        if (boost::endian::endian_reverse(data.m_marker) != TridCounters::kMarker) {
            throwDatabaseError(IOManagerMessageId::kErrorTridCounterFileCorrupted,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id);
        }
        const auto tridCounterMigrationFilePath =
                tridCounterFilePath + kTridCounterMigrationFileExt;
        if (::rename(tridCounterFilePath.c_str(), tridCounterMigrationFilePath.c_str())) {
            const auto errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotRenameTridCounterFile,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id, errorCode, std::strerror(errorCode));
        }
        utils::reverseByteOrder(data.m_lastUserTrid);
        utils::reverseByteOrder(data.m_lastSystemTrid);
        writeFullTridCounters(fd.getFD(), data);
        if (::rename(tridCounterMigrationFilePath.c_str(), tridCounterFilePath.c_str())) {
            const auto errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotRenameTridCounterFile,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id, errorCode, std::strerror(errorCode));
        }
    }
    return fd.release();
}

void Column::loadMasterColumnMainIndex()
{
    if (m_masterColumnData->m_mainIndex) {
        throwDatabaseError(IOManagerMessageId::kErrorMasterColumnMainIndexAlreadyExists,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id);
    }

    // Open file
    const auto mainIndexIdFilePath = utils::constructPath(m_dataDir, kMainIndexIdFile);
    FDGuard fd(::open(mainIndexIdFilePath.c_str(), O_RDWR | O_DSYNC | O_CLOEXEC | O_NOATIME,
            kDataFileCreationMode));
    if (!fd.isValidFd()) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenMainIndexIdFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode));
    }

    // Read data
    std::uint64_t indexId0 = 0;
    const auto n = ::readExact(fd.getFD(), &indexId0, sizeof(indexId0), kIgnoreSignals);
    if (n != sizeof(indexId0)) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadMainIndexIdFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode), sizeof(indexId0), n);
    }
    fd.reset();

    // Decode data
    std::uint64_t indexId = 0;
    ::pbeDecodeUInt64(reinterpret_cast<const std::uint8_t*>(&indexId0), &indexId);

    // Create index
    const auto indexRecord = getDatabase().findIndexRecord(indexId);
    m_masterColumnData->m_mainIndex = std::make_shared<UInt64UniqueLinearIndex>(
            m_table, indexRecord, kMasterColumnNameMainIndexValueSize);
}

// --- internals ---

Table& Column::validateTable(Table& table, const ColumnRecord& columnRecord)
{
    if (columnRecord.m_tableId == table.getId()) return table;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnTable, columnRecord.m_id,
            columnRecord.m_tableId, table.getDatabaseName(), table.getName(),
            table.getDatabaseUuid(), table.getId());
}

std::string&& Column::validateColumnName(std::string&& columnName) const
{
    if (isValidDatabaseObjectName(columnName)) return std::move(columnName);
    throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnNameInTableColumn, getDatabaseName(),
            m_table.getName(), columnName);
}

ColumnDataType Column::validateColumnDataType(ColumnDataType dataType) const
{
    if (dataType < 0 || dataType >= COLUMN_DATA_TYPE_MAX) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidColumnDataTypeInTableColumn,
                static_cast<int>(dataType), getDatabaseName(), m_table.getName(), m_name);
    }
    if (isMasterColumnName() && dataType != kMasterColumnDataType) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidMasterColumnDataType, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id);
    }
    return dataType;
}

void Column::checkDataConsistency()
{
    struct BlockInfo {
        std::uint64_t m_currentBlockId;
        std::uint64_t m_prevBlockId;
        ColumnDataBlockHeader::Digest m_prevBlockDigest;
    };

    std::stack<BlockInfo> stack;

    // Start with very first block of the column, physically present on disk.
    BlockInfo blockInfo;
    blockInfo.m_currentBlockId = findFirstBlock();
    if (blockInfo.m_currentBlockId != 0) {
        blockInfo.m_prevBlockId = 0;
        blockInfo.m_prevBlockDigest = ColumnDataBlockHeader::kInitialPrevBlockDigest;
        stack.push(blockInfo);
    }

    while (!stack.empty()) {
        // Get next block
        blockInfo = stack.top();
        stack.pop();

        while (true) {
            // Load block
            //DBG_LOG_DEBUG("Column::checkDataConsistency(): "
            //              << makeDisplayName() << ": Checking block " << blockInfo.m_currentBlockId);
            const auto currentBlock = findExistingBlock(blockInfo.m_currentBlockId);

            // Ensure previous block ID saved in block is correct
            if (currentBlock->getPrevBlockId() != blockInfo.m_prevBlockId) {
                throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockConsistencyMismatch,
                        getDatabaseName(), m_table.getName(), m_name, blockInfo.m_currentBlockId,
                        getDatabaseUuid(), m_table.getId(), m_id, "previous block ID mismatch");
            }

            // We can check only closed blocks
            if (currentBlock->getState() != ColumnDataBlockState::kClosed) break;

            // Check block digest based on data in block
            ColumnDataBlockHeader::Digest currentBlockDigest;
            currentBlock->computeDigest(blockInfo.m_prevBlockDigest, currentBlockDigest);
            if (currentBlock->getDigest() == currentBlockDigest) {
                LOG_DEBUG << "block digest mismatch";
                throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockConsistencyMismatch,
                        getDatabaseName(), m_table.getName(), getName(), blockInfo.m_currentBlockId,
                        getDatabaseUuid(), m_table.getId(), m_id, "block digest mismatch");
            }

            // Collect block into available block list, if it has enough free space
            if (currentBlock->getFreeDataSpace() >= s_minRequiredBlockFreeSpaces[m_dataType]) {
                m_availableDataBlocks.emplace(
                        currentBlock->getId(), currentBlock->getFreeDataSpace());
            }

            // Determine next blocks
            const auto nextBlockIds = m_blockRegistry.findNextBlockIds(blockInfo.m_currentBlockId);
            if (nextBlockIds.empty()) break;
            blockInfo.m_prevBlockId = blockInfo.m_currentBlockId;
            blockInfo.m_prevBlockDigest = currentBlockDigest;
            if (nextBlockIds.size() == 1) {
                blockInfo.m_currentBlockId = nextBlockIds.front();
                continue;
            }
            for (auto rit = nextBlockIds.rbegin(); rit != nextBlockIds.rend(); ++rit) {
                blockInfo.m_currentBlockId = *rit;
                stack.push(blockInfo);
            }
            break;
        }
    }
}

ColumnDefinitionPtr Column::createColumnDefinitionUnlocked()
{
    auto columnDefinition = std::make_shared<ColumnDefinition>(*this);
    m_columnDefinitionCache.emplace(columnDefinition->getId(), columnDefinition);
    getDatabase().registerColumnDefinition(*columnDefinition);
    return columnDefinition;
}

ColumnDefinitionPtr Column::createColumnDefinitionUnlocked(
        const ColumnDefinitionRecord& columnDefinitionRecord)
{
    auto columnDefinition = std::make_shared<ColumnDefinition>(*this, columnDefinitionRecord);
    m_columnDefinitionCache.emplace(columnDefinition->getId(), columnDefinition);
    return columnDefinition;
}

void Column::createMasterColumnMainIndex()
{
    LOG_DEBUG << "Creating master column index for the table " << getDatabaseName() << '.'
              << getTableName();

    auto indexName = composeMasterColumnMainIndexName();
    const IndexColumnSpecification indexColumnSpec(m_currentColumnDefinition, false);
    m_masterColumnData->m_mainIndex = std::make_shared<UInt64UniqueLinearIndex>(m_table,
            std::move(indexName), kMasterColumnNameMainIndexValueSize, indexColumnSpec,
            m_table.isSystemTable() ? kSystemTableDataFileDataAreaSize
                                    : kDefaultDataFileDataAreaSize,
            kMasterColumnMainIndexDescription);

    // Prepare main index ID file content
    std::uint64_t indexId = 0;
    ::pbeEncodeUInt64(
            m_masterColumnData->m_mainIndex->getId(), reinterpret_cast<std::uint8_t*>(&indexId));

    // Write index ID file
    const auto mainIndexIdFilePath = utils::constructPath(m_dataDir, kMainIndexIdFile);
    FDGuard fd(::open(mainIndexIdFilePath.c_str(),
            O_CREAT | O_RDWR | O_DSYNC | O_CLOEXEC | O_NOATIME, kDataFileCreationMode));
    if (!fd.isValidFd()) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateMainIndexIdFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode));
    }
    const auto n = ::writeExact(fd.getFD(), &indexId, sizeof(indexId), kIgnoreSignals);
    if (n != sizeof(indexId)) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteMainIndexIdFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode), sizeof(indexId), n);
    }
}

void Column::createMasterColumnConstraints()
{
    ConstraintDefinitionPtr constraintDefinition;
    if (m_table.isSystemTable()) {
        constraintDefinition = m_table.getDatabase().getSystemNotNullConstraintDefinition();
    } else {
        const requests::ConstantExpression expression(Variant::true_());
        BinaryValue serializedConstraintExpression(expression.getSerializedSize());
        expression.serializeUnchecked(serializedConstraintExpression.data());
        constraintDefinition = getDatabase().findOrCreateConstraintDefinition(
                false, ConstraintType::kNotNull, serializedConstraintExpression, m_id);
    }
    m_currentColumnDefinition->addConstraint(m_table.createConstraint(
            std::string(), constraintDefinition, this, kMasterColumnNotNullConstraintDescription));
}

void Column::writeFullTridCounters(int fd, const TridCounters& data)
{
    const auto n = ::pwriteExact(fd, &data, sizeof(data), 0, kIgnoreSignals);
    if (n != TridCounters::kDataSize) {
        const auto errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteTridCounterFile, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                std::strerror(errorCode), sizeof(data), n);
    }
}

std::string Column::composeMasterColumnMainIndexName() const
{
    std::ostringstream oss;
    oss << "$MCMI$" << m_table.getId() << '$' << m_id;
    return oss.str();
}

std::string Column::ensureDataDir(bool create) const
{
    // NOTE: use isMasterColumnName() here, since master column data may be N/A at the moment
    auto dataDir = utils::constructPath(m_table.getDataDir(),
            isMasterColumnName() ? kMasterColumnDataDirPrefix : kColumnDataDirPrefix, m_id);
    const auto initFlagFile = utils::constructPath(dataDir, kInitializationFlagFile);
    const auto initFlagFileExists = fs::exists(initFlagFile);
    if (create) {
        // Check initialization flag
        if (initFlagFileExists) {
            throwDatabaseError(IOManagerMessageId::kErrorColumnAlreadyExists, getDatabaseName(),
                    m_table.getName(), m_name);
        }

        // Create data directory
        try {
            const fs::path dataDirPath(dataDir);
            if (fs::exists(dataDirPath)) fs::remove_all(dataDirPath);
            fs::create_directories(dataDirPath);
        } catch (fs::filesystem_error& ex) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotCreateColumnDataDir, dataDir,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id, ex.code().value(), ex.code().message());
        }
    } else {
        // Check that data directory exists
        if (!boost::filesystem::exists(dataDir)) {
            throwDatabaseError(IOManagerMessageId::kErrorColumnDataFolderDoesNotExist,
                    getDatabaseName(), m_table.getName(), m_name, dataDir);
        }

        // Check initialization flag
        if (!initFlagFileExists) {
            throwDatabaseError(IOManagerMessageId::kErrorColumnInitFileDoesNotExist,
                    getDatabaseName(), m_table.getName(), m_name, initFlagFile);
        }
    }
    return dataDir;
}

void Column::createInitializationFlagFile() const
{
    const auto initFlagFile = utils::constructPath(m_dataDir, kInitializationFlagFile);
    std::ofstream ofs(initFlagFile);
    if (!ofs.is_open()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateColumnInitializationFlagFile,
                initFlagFile, getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                m_table.getId(), m_id, "create file failed");
    }
    ofs.exceptions(std::ios::badbit | std::ios::failbit);
    try {
        ofs << std::time(nullptr) << std::flush;
    } catch (std::exception& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateColumnInitializationFlagFile,
                initFlagFile, getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                m_table.getId(), m_id, "write failed");
    }
}

}  // namespace siodb::iomgr::dbengine
