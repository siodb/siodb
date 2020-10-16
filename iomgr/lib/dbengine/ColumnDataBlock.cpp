// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDataBlock.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/config/SiodbDataFileDefs.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FDGuard.h>
#include <siodb/common/utils/FSUtils.h>
#include <siodb/common/utils/MessageCatalog.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// CRT headers
#include <cstdlib>
#include <cstring>

// STL headers
#include <sstream>

// System headers
#include <sys/stat.h>
#include <sys/types.h>

// OpenSSL
#include <openssl/sha.h>

namespace siodb::iomgr::dbengine {

const BinaryValue ColumnDataBlock::s_dataFileHeaderProto(kDataFileHeaderSize, 0);

ColumnDataBlock::ColumnDataBlock(
        Column& column, std::uint64_t prevBlockId, ColumnDataBlockState state)
    : m_column(column)
    , m_header(column.getDatabaseUuid(), column.getTableId(), column.getId(),
              m_column.generateNextBlockId(), column.getDataBlockDataAreaSize())
    , m_prevBlockId(prevBlockId)
    , m_dataFilePath(makeDataFilePath())
    , m_file(createDataFile())
    , m_state(state)
    , m_headerModified(false)
    , m_dataModified(false)
{
    loadHeader();
}

ColumnDataBlock::ColumnDataBlock(Column& column, std::uint64_t id)
    : m_column(column)
    , m_header(column.getDatabaseUuid(), column.getTableId(), column.getId(), id,
              column.getDataBlockDataAreaSize())
    , m_prevBlockId(column.findPrevBlockId(id))
    , m_dataFilePath(makeDataFilePath())
    , m_file(openDataFile())
    , m_state(ColumnDataBlockState::kCreating)
    , m_headerModified(false)
    , m_dataModified(false)
{
    loadHeader();
}

ColumnDataBlock::~ColumnDataBlock()
{
    //DBG_LOG_DEBUG("Deactivating ColumnDataBlock " << m_column.makeDisplayName());
    const bool headerModified = m_headerModified;
    if (m_headerModified) writeHeader();
    if (m_dataModified || headerModified) m_file->flush();
}

std::string ColumnDataBlock::makeDisplayName() const
{
    std::ostringstream oss;
    oss << '\'' << m_column.getDatabaseName() << "'.'" << m_column.getTableName() << "'.'"
        << m_column.getName() << "'.'" << getId() << '\'';
    return oss.str();
}

std::string ColumnDataBlock::makeDisplayCode() const
{
    std::ostringstream oss;
    oss << m_column.getDatabaseUuid() << '.' << m_column.getTableId() << '.' << m_column.getId()
        << '.' << getId();
    return oss.str();
}

void ColumnDataBlock::setNextDataPos(std::uint32_t pos)
{
    // Note: Allow setting position to block size, thus we get zero free space.
    if (pos > m_column.getDataBlockDataAreaSize()) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorInvalidPositionForColumnDataBlock,
                pos, m_column.getDataBlockDataAreaSize());
    }
    m_header.m_nextDataOffset = pos;
}

void ColumnDataBlock::readData(void* data, std::size_t length, std::uint32_t pos) const
{
    if (pos + length > m_column.getDataBlockDataAreaSize()) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotReadColumnDataBlockFile,
                pos, length, -1,
                "invalid offset or length, sum exceeds "
                        + std::to_string(m_column.getDataBlockDataAreaSize() - 1),
                0);
    }
    const auto readOffset = pos + m_header.m_dataAreaOffset;
    const auto n = m_file->read(static_cast<std::uint8_t*>(data), length, readOffset);
    if (n != length) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotReadColumnDataBlockFile,
                readOffset, length, m_file->getLastError(), std::strerror(m_file->getLastError()),
                n);
    }
}

void ColumnDataBlock::writeData(const void* data, std::size_t length, std::uint32_t pos)
{
    if (pos + length > m_column.getDataBlockDataAreaSize()) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotWriteColumnDataBlockFile,
                pos, length, -1,
                "invalid offset or length, sum exceeds "
                        + std::to_string(m_column.getDataBlockDataAreaSize() - 1),
                0);
    }
    const auto writeOffset = pos + m_header.m_dataAreaOffset;
    const auto n = m_file->write(static_cast<const std::uint8_t*>(data), length, writeOffset);
    if (n != length) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotWriteColumnDataBlockFile,
                writeOffset, length, m_file->getLastError(), std::strerror(m_file->getLastError()),
                n);
    }
    m_dataModified = true;
}

void ColumnDataBlock::finalize(const ColumnDataBlockHeader::Digest& prevBlockDigest)
{
    m_state = ColumnDataBlockState::kClosing;
    m_column.updateBlockState(getId(), m_state);
    m_header.m_fillTimestamp = std::time(nullptr);
    computeDigest(prevBlockDigest, m_header.m_digest);
    m_headerModified = true;
    writeHeader();
    m_state = ColumnDataBlockState::kClosed;
    m_column.updateBlockState(getId(), m_state);
}

void ColumnDataBlock::computeDigest(const ColumnDataBlockHeader::Digest& prevBlockDigest,
        ColumnDataBlockHeader::Digest& blockDigest) const
{
    // Serialize significant data from header
    std::uint8_t headerData[ColumnDataBlockHeader::kSerializedSize];
    const auto dataLength = m_header.m_nextDataOffset;
    std::uint8_t* p = headerData;
    p = pbeEncodeBinary(m_header.m_fullColumnDataBlockId.m_databaseUuid.data,
            m_header.m_fullColumnDataBlockId.m_databaseUuid.size(), p);
    p = pbeEncodeUInt32(m_header.m_fullColumnDataBlockId.m_tableId, p);
    p = pbeEncodeUInt32(m_header.m_fullColumnDataBlockId.m_columnId, p);
    p = pbeEncodeUInt64(m_header.m_fullColumnDataBlockId.m_blockId, p);
    p = pbeEncodeInt64(m_header.m_fillTimestamp, p);
    p = pbeEncodeUInt32(dataLength, p);

    // Compute digest
    ::SHA256_CTX ctx;
    ::SHA256_Init(&ctx);
    ::SHA256_Update(&ctx, prevBlockDigest.data(), prevBlockDigest.size());
    ::SHA256_Update(&ctx, headerData, p - headerData);
    if (dataLength > 0) {
        std::vector<std::uint8_t> buffer(dataLength);
        const auto n = m_file->read(buffer.data(), dataLength, m_header.m_dataAreaOffset);
        if (n != dataLength) {
            throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotReadColumnDataBlockFile,
                    m_header.m_dataAreaOffset, dataLength, m_file->getLastError(),
                    std::strerror(m_file->getLastError()), n);
        }
        ::SHA256_Update(&ctx, buffer.data(), dataLength);
    }
    ::SHA256_Final(blockDigest.data(), &ctx);
}

// ---- internals ----

io::FilePtr ColumnDataBlock::createDataFile() const
{
    LOG_DEBUG << "Creating ColumnDataBlock " << m_column.getDatabaseName() << '.'
              << m_column.getTableName() << '.' << m_column.getName() << '.' << getId();

    std::string tmpFilePath;

    // Create data file as temporary file
    constexpr int kBaseExtraOpenFlags = O_DSYNC;
    io::FilePtr file;
    try {
        try {
            file = m_column.getDatabase().createFile(m_column.getDataDir(),
                    kBaseExtraOpenFlags | O_TMPFILE, kDataFileCreationMode, getDataFileSize());
        } catch (std::system_error& ex) {
            if (ex.code().value() != ENOTSUP) throw;
            // O_TMPFILE not supported, fallback to the named temporary file
            tmpFilePath = m_dataFilePath + kTempFileExtension;
            file = m_column.getDatabase().createFile(
                    tmpFilePath, kBaseExtraOpenFlags, kDataFileCreationMode, getDataFileSize());
        }
    } catch (std::system_error& ex) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotCreateColumnDataBlockFile,
                m_dataFilePath, "Can't create new file", ex.code().value(),
                std::strerror(ex.code().value()));
    }

    // Prepare and write header
    const auto remainingHeaderSize =
            s_dataFileHeaderProto.size() - ColumnDataBlockHeader::kSerializedSize;
    std::uint8_t buffer[ColumnDataBlockHeader::kSerializedSize];
    m_header.serialize(buffer);
    auto n = file->write(buffer, sizeof(buffer), 0);
    if (n != sizeof(buffer)) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotWriteColumnDataBlockFile,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), getId(),
                file->getLastError(), sizeof(buffer), std::strerror(file->getLastError()), n);
    }

    // Write rest of header
    n = file->write(s_dataFileHeaderProto.data(), remainingHeaderSize, sizeof(buffer));
    if (n != remainingHeaderSize) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotWriteColumnDataBlockFile,
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), sizeof(buffer),
                remainingHeaderSize, file->getLastError(), std::strerror(file->getLastError()), n);
    }

    if (tmpFilePath.empty()) {
        // Link to the filesystem.
        const auto fdPath = "/proc/self/fd/" + std::to_string(file->getFD());
        if (::linkat(AT_FDCWD, fdPath.c_str(), AT_FDCWD, m_dataFilePath.c_str(), AT_SYMLINK_FOLLOW)
                < 0) {
            const int errorCode = errno;
            throwDatabaseErrorForThisObject(
                    IOManagerMessageId::kErrorCannotCreateColumnDataBlockFile, m_dataFilePath,
                    "Can't link new file to the filesystem", errorCode, std::strerror(errorCode));
        }
    } else {
        // Rename temporary file to the regular one
        if (::rename(tmpFilePath.c_str(), m_dataFilePath.c_str()) < 0) {
            const int errorCode = errno;
            throwDatabaseErrorForThisObject(
                    IOManagerMessageId::kErrorCannotCreateColumnDataBlockFile, m_dataFilePath,
                    "Can't rename temporary file to the regular one", errorCode,
                    std::strerror(errorCode));
        }
    }

    return file;
}

io::FilePtr ColumnDataBlock::openDataFile() const
{
    io::FilePtr file;
    try {
        file = m_column.getDatabase().openFile(m_dataFilePath, O_DSYNC);
    } catch (std::system_error& ex) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotOpenColumnDataBlockFile,
                m_dataFilePath, ex.code().value(), ex.what());
    }
    return file;
}

std::string ColumnDataBlock::makeDataFilePath() const
{
    return utils::constructPath(
            m_column.getDataDir(), kBlockFilePrefix, getId(), kDataFileExtension);
}

void ColumnDataBlock::loadHeader()
{
    // Read header
    std::uint8_t buffer[ColumnDataBlockHeader::kSerializedSize];
    const auto n = m_file->read(buffer, sizeof(buffer), 0);
    if (n != sizeof(buffer)) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotReadColumnDataBlockFile, 0,
                sizeof(buffer), m_file->getLastError(), std::strerror(m_file->getLastError()), n);
    }

    ColumnDataBlockHeader header;
    header.deserialize(buffer);

    // Validate header
    if (header.m_version > ColumnDataBlockHeader::kCurrentVersion
            || header.m_fullColumnDataBlockId != m_header.m_fullColumnDataBlockId) {
        throwDatabaseErrorForThisObject(
                IOManagerMessageId::kErrorInvalidDataFileHeader, header.m_version);
    }

    m_header = header;
}

void ColumnDataBlock::writeHeader() const
{
    uint8_t header[ColumnDataBlockHeader::kSerializedSize];
    m_header.serialize(header);
    const auto n = m_file->write(header, sizeof(header), 0);
    if (n != sizeof(header)) {
        throwDatabaseErrorForThisObject(IOManagerMessageId::kErrorCannotWriteColumnDataBlockFile, 0,
                sizeof(header), m_file->getLastError(), std::strerror(m_file->getLastError()), n);
    }
    m_headerModified = false;
}

template<class MessageId, class... Args>
[[noreturn]] void ColumnDataBlock::throwDatabaseErrorForThisObject(
        MessageId messageId, Args&&... args) const
{
    throwDatabaseError(messageId, m_column.getDatabaseName(), m_column.getTableName(),
            m_column.getName(), getId(), m_column.getDatabaseUuid(), m_column.getTableId(),
            m_column.getId(), std::forward<Args>(args)...);
}

}  // namespace siodb::iomgr::dbengine
