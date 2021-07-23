// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BlockRegistry.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "Column.h"
#include "DBEngineDebug.h"
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/config/SiodbDataFileDefs.h>
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/DebugUtils.h>
#include <siodb/common/utils/FSUtils.h>
#include <siodb/common/utils/HelperMacros.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// STL headers
#include <fstream>

// Boost headers
#include <boost/algorithm/string/predicate.hpp>

// System headers
#include <sys/stat.h>
#include <sys/types.h>

namespace siodb::iomgr::dbengine {

BlockRegistry::BlockRegistry(const Column& column, bool create)
    : m_column(column)
    , m_dataDir(
              ensureDataDir(utils::constructPath(m_column.getDataDir(), kBlockRegistryDir), create))
    , m_blockListFileSize(0)
    , m_nextBlockListFileSize(0)
    , m_lastBlockId(0)
{
    DBG_LOG_DEBUG((create ? "Creating" : "Loading")
                  << " BlockRegistry " << m_column.makeDisplayName() << " in " << m_dataDir);
    if (create)
        createDataFiles();
    else
        openDataFiles();
}

std::uint64_t BlockRegistry::findPrevBlockId(std::uint64_t blockId) const
{
    BREG_DBG_LOG_DEBUG("BlockRegistry::findPrevBlockId(): " << m_column.makeDisplayName()
                                                            << ": blockId=" << blockId);

    // Obtain block record location
    const auto blockRecordOffset = checkBlockRecordPresent(blockId);

    // Read previous block ID
    std::uint8_t buffer[sizeof(std::uint64_t)];
    const auto readOffset = blockRecordOffset + BlockListRecord::kPrevBlockIdSerializedFieldOffset;
    const auto n = ::preadExact(
            m_blockListFile.getFD(), buffer, sizeof(buffer), readOffset, kIgnoreSignals);
    if (n != sizeof(buffer)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadBlockListDataFile, __func__,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), readOffset,
                sizeof(buffer), errorCode, std::strerror(errorCode), n);
    }
    std::uint64_t prevBlockId = 0;
    ::pbeDecodeUInt64(buffer, &prevBlockId);
    return prevBlockId;
}

std::vector<std::uint64_t> BlockRegistry::findNextBlockIds(std::uint64_t blockId) const
{
    std::vector<std::uint64_t> nextBlocks;

    // Read first next block list record address
    NextBlockListRecord nextBlockRecord;
    {
        BlockListRecord blockRecord;
        loadRecord(blockId, blockRecord);
        nextBlockRecord.m_nextBlockListFileOffset = blockRecord.m_firstNextBlockListFileOffset;
    }

    while (nextBlockRecord.m_nextBlockListFileOffset != 0) {
        // Read next block list record
        std::uint8_t buffer[NextBlockListRecord::kSerializedSize];
        const auto n = ::preadExact(m_nextBlockListFile.getFD(), buffer, sizeof(buffer),
                nextBlockRecord.m_nextBlockListFileOffset, kIgnoreSignals);
        if (n != sizeof(buffer)) {
            const int errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotReadNextBlockListDataFile, __func__,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                    nextBlockRecord.m_nextBlockListFileOffset, sizeof(buffer), errorCode,
                    std::strerror(errorCode), n);
        }
        nextBlockRecord.deserialize(buffer);
        // Save block ID
        nextBlocks.push_back(nextBlockRecord.m_blockId);
    }

    return nextBlocks;
}

void BlockRegistry::recordBlockAndNextBlock(
        std::uint64_t blockId, std::uint64_t parentBlockId, ColumnDataBlockState state)
{
    recordBlock(blockId, parentBlockId, state);
    if (parentBlockId != 0) addNextBlock(blockId, parentBlockId);
}

void BlockRegistry::recordBlock(
        std::uint64_t blockId, std::uint64_t parentBlockId, ColumnDataBlockState state)
{
    // Obtain block record location
    const auto blockRecordOffset = blockId * BlockListRecord::kSerializedSize;

    //BREG_DBG_LOG_DEBUG(
    LOG_DEBUG << "BlockRegistry::recordBlock(): " << m_column.makeDisplayName() << '.' << blockId
              << ", parent " << parentBlockId << ", state " << static_cast<int>(state)
              << ", offset " << blockRecordOffset;

    // Prepare record
    BlockListRecord blockRecord;
    std::memset(&blockRecord, 0, sizeof(blockRecord));
    blockRecord.m_blockId = blockId;
    blockRecord.m_blockState = state;
    blockRecord.m_prevBlockId = parentBlockId;
    std::uint8_t buffer[BlockListRecord::kSerializedSize];
    blockRecord.serialize(buffer);

    //LOG_DEBUG << "BlockRegistry: addBlock #" << blockId << ": Block dump:\n"
    //          << utils::dumpMemoryToString(buffer, sizeof(buffer));

    const auto n = ::pwriteExact(
            m_blockListFile.getFD(), buffer, sizeof(buffer), blockRecordOffset, kIgnoreSignals);
    if (n != sizeof(buffer)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteBlockListDataFile, __func__,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                blockRecordOffset, sizeof(buffer), errorCode, std::strerror(errorCode), n);
    }

    if (blockId > m_lastBlockId) m_lastBlockId = blockId;
}

void BlockRegistry::updateBlockState(std::uint64_t blockId, ColumnDataBlockState state) const
{
    BREG_DBG_LOG_DEBUG("BlockRegistry::updateBlockState(): " << m_column.makeDisplayName() << '.'
                                                             << blockId << ", new state "
                                                             << static_cast<int>(state));

    const auto blockRecordOffset = checkBlockRecordPresent(blockId);

    // Write new block state
    std::uint8_t buffer[1] = {static_cast<std::uint8_t>(state)};
    const auto writeOffset = blockRecordOffset + BlockListRecord::kBlockStateSerializedFieldOffset;
    const auto n = ::pwriteExact(
            m_blockListFile.getFD(), buffer, sizeof(buffer), writeOffset, kIgnoreSignals);
    if (n != sizeof(buffer)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteBlockListDataFile, __func__,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), writeOffset,
                sizeof(buffer), errorCode, std::strerror(errorCode), n);
    }
}

void BlockRegistry::addNextBlock(std::uint64_t blockId, std::uint64_t nextBlockId)
{
    BREG_DBG_LOG_DEBUG("BlockRegistry: Recording NEXT block: "
                       << m_column.makeDisplayName() << '.' << blockId << ", next " << nextBlockId);

    BlockListRecord blockRecord;
    loadRecord(blockId, blockRecord);

    const auto newRecordLocation = m_nextBlockListFileSize;

    // Prepare next block list record
    NextBlockListRecord record;
    record.m_blockId = nextBlockId;
    record.m_nextBlockListFileOffset = 0;
    std::uint8_t buffer[NextBlockListRecord::kSerializedSize];
    record.serialize(buffer);

    // Write record
    auto n = ::pwriteExact(
            m_nextBlockListFile.getFD(), buffer, sizeof(buffer), newRecordLocation, kIgnoreSignals);
    if (n != sizeof(buffer)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteNextBlockListDataFile, __func__,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                newRecordLocation, sizeof(buffer), errorCode, std::strerror(errorCode), n);
    }

    struct LastRecordUpdate {
        LastRecordUpdate(const Column& column, int fd, std::uint64_t offset, std::uint8_t* buffer,
                std::uint64_t blockId)
            : m_column(column)
            , m_fd(fd)
            , m_offset(offset)
            , m_buffer(buffer)
            , m_blockId(blockId)
            , m_updateSize(sizeof(NextBlockListRecord::m_nextBlockListFileOffset))
            , m_committed(false)
        {
            // Update last record
            const auto writeOffset =
                    offset + NextBlockListRecord::kNextBlockListFileOffsetSerializedFieldOffset;
            const auto n = ::pwriteExact(m_fd,
                    m_buffer + NextBlockListRecord::kNextBlockListFileOffsetSerializedFieldOffset,
                    m_updateSize, writeOffset, kIgnoreSignals);
            if (n != m_updateSize) {
                const int errorCode = errno;
                throwDatabaseError(IOManagerMessageId::kErrorCannotWriteNextBlockListDataFile,
                        __func__, m_column.getDatabaseName(), m_column.getDatabaseUuid(),
                        m_column.getTableId(), m_column.getId(), writeOffset, m_updateSize,
                        errorCode, std::strerror(errorCode), n);
            }
        }

        ~LastRecordUpdate() noexcept(false)
        {
            if (m_committed) return;

            // Prepare record for rollback
            NextBlockListRecord lastRecord;
            lastRecord.m_blockId = m_blockId;
            lastRecord.m_nextBlockListFileOffset = 0;
            lastRecord.serialize(m_buffer);

            // Update last record
            const auto writeOffset =
                    m_offset + NextBlockListRecord::kNextBlockListFileOffsetSerializedFieldOffset;
            const auto n = ::pwriteExact(m_fd,
                    m_buffer + NextBlockListRecord::kNextBlockListFileOffsetSerializedFieldOffset,
                    m_updateSize, writeOffset, kIgnoreSignals);
            if (n != m_updateSize) {
                if (std::uncaught_exceptions() == 0) {
                    const int errorCode = errno;
                    throwDatabaseError(IOManagerMessageId::kErrorCannotWriteNextBlockListDataFile,
                            __func__, m_column.getDatabaseName(), m_column.getDatabaseUuid(),
                            m_column.getTableId(), m_column.getId(), writeOffset, m_updateSize,
                            errorCode, std::strerror(errorCode), n);
                }
            }
        }

        void commit() noexcept
        {
            m_committed = true;
        }

        const Column& m_column;
        const int m_fd;
        const std::uint64_t m_offset;
        std::uint8_t* const m_buffer;
        const std::uint64_t m_blockId;
        const std::size_t m_updateSize;
        bool m_committed;
    };

    std::unique_ptr<LastRecordUpdate> lastRecordUpdate;
    if (blockRecord.m_lastNextBlockListFileOffset == 0) {
        // This is first next block, fill block record
        blockRecord.m_firstNextBlockListFileOffset = newRecordLocation;
    } else {
        // This is some subsequent one, therefore also update last record in the chain.
        // Load old last next block record
        n = ::preadExact(m_nextBlockListFile.getFD(), buffer, NextBlockListRecord::kSerializedSize,
                blockRecord.m_lastNextBlockListFileOffset, kIgnoreSignals);
        LOG_DEBUG << __func__ << ": Read next block list file " << m_column.getDatabaseUuid() << '.'
                  << m_column.getTableId() << '.' << m_column.getId() << ": blockId " << blockId
                  << " offset " << blockRecord.m_lastNextBlockListFileOffset << " length "
                  << NextBlockListRecord::kSerializedSize << " result " << n << std::endl;
        if (n != NextBlockListRecord::kSerializedSize) {
            const int errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotReadNextBlockListDataFile, __func__,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                    blockRecord.m_lastNextBlockListFileOffset, NextBlockListRecord::kSerializedSize,
                    errorCode, std::strerror(errorCode), n);
        }

        // Prepare record
        NextBlockListRecord lastRecord;
        lastRecord.deserialize(buffer);
        lastRecord.m_nextBlockListFileOffset = newRecordLocation;
        lastRecord.serialize(buffer);

        // Update record
        lastRecordUpdate = std::make_unique<LastRecordUpdate>(m_column, m_nextBlockListFile.getFD(),
                blockRecord.m_lastNextBlockListFileOffset, buffer, lastRecord.m_blockId);
    }

    // Update block record
    blockRecord.m_lastNextBlockListFileOffset = newRecordLocation;
    std::uint8_t blockRecordBuffer[BlockListRecord::kSerializedSize];
    blockRecord.serialize(blockRecordBuffer);
    const auto blockRecordOffset = computeBlockRecordOffset(blockId);
    n = ::pwriteExact(m_blockListFile.getFD(), blockRecordBuffer, sizeof(blockRecordBuffer),
            blockRecordOffset, kIgnoreSignals);
    if (n != sizeof(blockRecordBuffer)) {
        // Save error code
        const int errorCode = errno;
        // Report error
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteBlockListDataFile, __func__,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                blockRecordOffset, sizeof(blockRecordBuffer), errorCode, std::strerror(errorCode),
                n);
    }

    // Update next data offset in the next record file
    m_nextBlockListFileSize += NextBlockListRecord::kSerializedSize;

    // Finally
    if (lastRecordUpdate) lastRecordUpdate->commit();
}

// --- internals ---

void BlockRegistry::createDataFiles()
{
    const auto blockListFilePath = utils::constructPath(
            m_dataDir, kBlockListFileName, m_column.getId(), kDataFileExtension);

    const auto nextBlockListFilePath = utils::constructPath(
            m_dataDir, kNextBlockListFileName, m_column.getId(), kDataFileExtension);

    FDGuard blockListFile(::open(blockListFilePath.c_str(), O_CREAT | O_CLOEXEC | O_DSYNC | O_RDWR,
            kDataFileCreationMode));
    if (!blockListFile.isValidFd()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateBlockListDataFile,
                blockListFilePath, m_column.getDatabaseName(), m_column.getTableName(),
                m_column.getName(), m_column.getDatabaseUuid(), m_column.getTableId(),
                m_column.getId(), errorCode, std::strerror(errorCode));
    }

    FDGuard nextBlockListFile(::open(nextBlockListFilePath.c_str(),
            O_CREAT | O_CLOEXEC | O_DSYNC | O_RDWR, kDataFileCreationMode));
    if (!blockListFile.isValidFd()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateNextBlockListDataFile,
                nextBlockListFilePath, m_column.getDatabaseName(), m_column.getTableName(),
                m_column.getName(), m_column.getDatabaseUuid(), m_column.getTableId(),
                m_column.getId(), errorCode, std::strerror(errorCode));
    }

    createInitializationFlagFile();

    m_blockListFile.swap(blockListFile);
    m_nextBlockListFile.swap(nextBlockListFile);

    BREG_DBG_LOG_DEBUG("BlockRegistry " << m_column.makeDisplayName() << ": data files created.");
}

void BlockRegistry::openDataFiles()
{
    const auto blockListFilePath = utils::constructPath(
            m_dataDir, kBlockListFileName, m_column.getId(), kDataFileExtension);

    const auto nextBlockListFilePath = utils::constructPath(
            m_dataDir, kNextBlockListFileName, m_column.getId(), kDataFileExtension);

    FDGuard blockListFile(
            ::open(blockListFilePath.c_str(), O_CLOEXEC | O_DSYNC | O_RDWR, kDataFileCreationMode));
    if (!blockListFile.isValidFd()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenBlockListDataFile, blockListFilePath,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), errorCode,
                std::strerror(errorCode));
    }

    const auto blockListFileSize = lseek(blockListFile.getFD(), 0, SEEK_END);
    if (blockListFileSize < 0) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotGetBlockListDataFileSize,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), errorCode,
                std::strerror(errorCode));
    }
    if (blockListFileSize % BlockListRecord::kSerializedSize != 0) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidBlockListDataFileSize,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                blockListFileSize);
    }

    FDGuard nextBlockListFile(::open(
            nextBlockListFilePath.c_str(), O_CLOEXEC | O_DSYNC | O_RDWR, kDataFileCreationMode));
    if (!blockListFile.isValidFd()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenNextBlockListDataFile,
                nextBlockListFilePath, m_column.getDatabaseName(), m_column.getTableName(),
                m_column.getName(), m_column.getDatabaseUuid(), m_column.getTableId(),
                m_column.getId(), errorCode, std::strerror(errorCode));
    }

    const auto nextBlockListFileSize = lseek(nextBlockListFile.getFD(), 0, SEEK_END);
    if (nextBlockListFileSize < 0) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotGetNextBlockListDataFileSize,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), errorCode,
                std::strerror(errorCode));
    }
    if (nextBlockListFileSize % NextBlockListRecord::kSerializedSize != 0) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidNextBlockListDataFileSize,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                nextBlockListFileSize);
    }

    m_blockListFile.swap(blockListFile);
    m_nextBlockListFile.swap(nextBlockListFile);
    m_blockListFileSize = blockListFileSize;
    m_nextBlockListFileSize = nextBlockListFileSize;
    m_lastBlockId = blockListFileSize / BlockListRecord::kSerializedSize;
    if (m_lastBlockId > 0) --m_lastBlockId;

    BREG_DBG_LOG_DEBUG("BlockRegistry " << m_column.makeDisplayName() << ": data files opened.");

    // Log this always
    LOG_DEBUG << "BlockRegistry " << m_column.makeDisplayName()
              << ": lastBlockId=" << m_lastBlockId;
}

void BlockRegistry::loadRecord(std::uint64_t blockId, BlockListRecord& record) const
{
    BREG_DBG_LOG_DEBUG("BlockRegistry::loadBlockRecord(): " << m_column.makeDisplayName()
                                                            << ": blockId=" << blockId);

    // Obtain block record location
    const auto blockRecordOffset = checkBlockRecordPresent(blockId);

    // Load block record
    std::uint8_t buffer[BlockListRecord::kSerializedSize];
    const auto n = ::preadExact(
            m_blockListFile.getFD(), buffer, sizeof(buffer), blockRecordOffset, kIgnoreSignals);
    if (n != sizeof(buffer)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadBlockListDataFile, __func__,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                blockRecordOffset, sizeof(buffer), errorCode, std::strerror(errorCode), n);
    }

    // Decode block record
    record.deserialize(buffer);
}

off_t BlockRegistry::checkBlockRecordPresent(std::uint64_t blockId) const
{
    BREG_DBG_LOG_DEBUG("BlockRegistry::checkBlockRecordPresent(): "
                       << m_column.makeDisplayName() << " lastBlockId=" << m_lastBlockId
                       << " checking blockId " << blockId);

    // Check block ID
    if (blockId > m_lastBlockId) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockDoesNotExist,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), blockId,
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId());
    }

    // Obtain block record location
    const auto blockRecordOffset = computeBlockRecordOffset(blockId);

    // Read block presence flag
    std::uint8_t buffer[1];
    const auto n = ::preadExact(m_blockListFile.getFD(), buffer, sizeof(buffer),
            blockRecordOffset + BlockListRecord::kBlockStateSerializedFieldOffset, kIgnoreSignals);
    if (n != sizeof(buffer)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadBlockListDataFile, __func__,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                blockRecordOffset, sizeof(buffer), errorCode, std::strerror(errorCode), n);
    }

    // Check block presence
    if (*buffer == 0) {
        throwDatabaseError(IOManagerMessageId::kErrorColumnDataBlockDoesNotExist,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), blockId,
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId());
    }

    return blockRecordOffset;
}

std::string&& BlockRegistry::ensureDataDir(std::string&& dataDir, bool create) const
{
    const auto initFlagFile = utils::constructPath(dataDir, kInitializationFlagFile);
    const bool initFlagFileExists = fs::exists(initFlagFile);
    if (create) {
        if (initFlagFileExists) {
            throwDatabaseError(IOManagerMessageId::kErrorBlockRegistryDirAlreadyExists,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId());
        }
        try {
            fs::path dataDirPath(dataDir);
            if (fs::exists(dataDirPath)) fs::remove_all(dataDirPath);
            fs::create_directories(dataDirPath);
        } catch (fs::filesystem_error& ex) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotCreateBlockRegistryDir,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                    ex.code().value(), ex.code().message());
        }
    } else {
        if (!initFlagFileExists) {
            throwDatabaseError(IOManagerMessageId::kErrorBlockRegistryDirNotExists,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId());
        }
    }
    return std::move(dataDir);
}

void BlockRegistry::createInitializationFlagFile() const
{
    const auto initFlagFile = utils::constructPath(m_dataDir, kInitializationFlagFile);
    FDGuard fd(::open(initFlagFile.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC | O_NOATIME,
            kDataFileCreationMode));
    if (!fd.isValidFd()) {
        const int errorCode = errno;
        std::ostringstream err;
        err << "Can't create block registry initialization flag file " << initFlagFile << ": "
            << std::strerror(errno);
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateBlockRegistryDir,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), errorCode,
                err.str());
    }
    const auto timeStr = std::to_string(std::time(nullptr));
    if (::writeExact(fd.getFD(), timeStr.c_str(), timeStr.length(), kIgnoreSignals)
            != timeStr.length()) {
        const int errorCode = errno;
        std::ostringstream err;
        err << "Can't write to the block registry initialization flag file " << initFlagFile << ": "
            << std::strerror(errno);
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateBlockRegistryDir,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), errorCode,
                err.str());
    }
}

//----------------- struct BlockRegistry::NextBlockListRecord -------------------------------------

std::uint8_t* BlockRegistry::NextBlockListRecord::serialize(std::uint8_t* buffer) const noexcept
{
    buffer = ::pbeEncodeUInt64(m_blockId, buffer);
    buffer = ::pbeEncodeUInt64(m_nextBlockListFileOffset, buffer);
    return buffer;
}

const std::uint8_t* BlockRegistry::NextBlockListRecord::deserialize(
        const std::uint8_t* buffer) noexcept
{
    buffer = ::pbeDecodeUInt64(buffer, &m_blockId);
    buffer = ::pbeDecodeUInt64(buffer, &m_nextBlockListFileOffset);
    return buffer;
}

//----------------- struct BlockRegistry::BlockListRecord -----------------------------------------

std::uint8_t* BlockRegistry::BlockListRecord::serialize(std::uint8_t* buffer) const noexcept
{
    *buffer++ = static_cast<std::uint8_t>(m_blockState);
    buffer = ::pbeEncodeUInt64(m_prevBlockId, buffer);
    buffer = ::pbeEncodeUInt64(m_firstNextBlockListFileOffset, buffer);
    buffer = ::pbeEncodeUInt64(m_lastNextBlockListFileOffset, buffer);
    return buffer;
}

const std::uint8_t* BlockRegistry::BlockListRecord::deserialize(const std::uint8_t* buffer) noexcept
{
    m_blockState = static_cast<ColumnDataBlockState>(*buffer++);
    buffer = ::pbeDecodeUInt64(buffer, &m_prevBlockId);
    buffer = ::pbeDecodeUInt64(buffer, &m_firstNextBlockListFileOffset);
    buffer = ::pbeDecodeUInt64(buffer, &m_lastNextBlockListFileOffset);
    return buffer;
}

}  // namespace siodb::iomgr::dbengine
