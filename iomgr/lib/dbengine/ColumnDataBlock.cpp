// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "ColumnDataBlock.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_ext/string_builder.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FdGuard.h>
#include <siodb/common/utils/FsUtils.h>
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

void ColumnDataBlock::readData(void* data, std::size_t length, std::uint32_t pos) const
{
    if (pos + length > m_column.getDataBlockDataAreaSize()) {
        std::ostringstream err;
        throw std::runtime_error(stdext::string_builder()
                                 << makeDisplayName() << ": Invalid offset or length: " << pos
                                 << ", " << length);
    }
    const auto readOffset = pos + m_header.m_dataAreaOffset;
    if (m_file->read(static_cast<std::uint8_t*>(data), length, readOffset) != length) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadColumnDataBlockFile,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), getId(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), readOffset,
                length, m_file->getLastError(), std::strerror(m_file->getLastError()));
    }
}

void ColumnDataBlock::writeData(const void* data, std::size_t length, std::uint32_t pos)
{
    if (pos + length > m_column.getDataBlockDataAreaSize()) {
        std::ostringstream err;
        throw std::runtime_error(stdext::string_builder()
                                 << makeDisplayName() << ": Invalid offset or length: " << pos
                                 << ", " << length);
    }
    const auto writeOffset = pos + m_header.m_dataAreaOffset;
    if (m_file->write(static_cast<const std::uint8_t*>(data), length, writeOffset) != length) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteColumnDataBlockFile,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), getId(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), writeOffset,
                length, m_file->getLastError(), std::strerror(m_file->getLastError()));
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
        if (m_file->read(buffer.data(), dataLength, m_header.m_dataAreaOffset) != dataLength) {
            throwDatabaseError(IOManagerMessageId::kErrorCannotReadColumnDataBlockFile,
                    m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(),
                    getId(), m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                    m_header.m_dataAreaOffset, dataLength, m_file->getLastError(),
                    std::strerror(m_file->getLastError()));
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
            // O_TMPFILE not supported, fallback to named temporary file
            tmpFilePath = m_dataFilePath + kTempFileExtension;
            file = m_column.getDatabase().createFile(
                    tmpFilePath, kBaseExtraOpenFlags, kDataFileCreationMode, getDataFileSize());
        }
    } catch (std::system_error& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateNewColumnDataBlockFile,
                m_dataFilePath, m_column.getDatabaseName(), m_column.getTableName(),
                m_column.getName(), getId(), m_column.getDatabaseUuid(), m_column.getTableId(),
                m_column.getId(), "Can't create new file", ex.code().value(),
                std::strerror(ex.code().value()));
    }

    // Prepare and write header
    const auto remainingHeaderSize =
            s_dataFileHeaderProto.size() - ColumnDataBlockHeader::kSerializedSize;
    std::uint8_t buffer[ColumnDataBlockHeader::kSerializedSize];
    m_header.serialize(buffer);
    if (file->write(buffer, sizeof(buffer), 0) != sizeof(buffer)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateNewColumnDataBlockFile,
                m_dataFilePath, m_column.getDatabaseName(), m_column.getTableName(),
                m_column.getName(), getId(), m_column.getDatabaseUuid(), m_column.getTableId(),
                m_column.getId(), "Can't write header part 1", file->getLastError(),
                std::strerror(file->getLastError()));
    }

    // Write rest of header
    if (file->write(s_dataFileHeaderProto.data(), remainingHeaderSize, sizeof(buffer))
            != remainingHeaderSize) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateNewColumnDataBlockFile,
                m_dataFilePath, m_column.getDatabaseName(), m_column.getTableName(),
                m_column.getName(), getId(), m_column.getDatabaseUuid(), m_column.getTableId(),
                m_column.getId(), "Can't write header part 2", file->getLastError(),
                std::strerror(file->getLastError()));
    }

    if (tmpFilePath.empty()) {
        // Link to the filesystem.
        const auto fdPath = "/proc/self/fd/" + std::to_string(file->getFD());
        if (::linkat(AT_FDCWD, fdPath.c_str(), AT_FDCWD, m_dataFilePath.c_str(), AT_SYMLINK_FOLLOW)
                < 0) {
            const int errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotCreateNewColumnDataBlockFile,
                    m_dataFilePath, m_column.getDatabaseName(), m_column.getTableName(),
                    m_column.getName(), getId(), m_column.getDatabaseUuid(), m_column.getTableId(),
                    m_column.getId(), "Can't link new file to the filesystem", errorCode,
                    std::strerror(errorCode));
        }
    } else {
        // Rename temporary file to the regular one
        if (::rename(tmpFilePath.c_str(), m_dataFilePath.c_str()) < 0) {
            const int errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotCreateNewColumnDataBlockFile,
                    m_dataFilePath, m_column.getDatabaseName(), m_column.getTableName(),
                    m_column.getName(), getId(), m_column.getDatabaseUuid(), m_column.getTableId(),
                    m_column.getId(), "Can't rename temporary file to the regular one", errorCode,
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
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenColumnDataBlockFile, m_dataFilePath,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), getId(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId());
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
    auto readBytes = m_file->read(buffer, sizeof(buffer), 0);
    if (readBytes == 0) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadColumnDataBlockFile,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), getId(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), 0,
                sizeof(buffer), m_file->getLastError(), std::strerror(m_file->getLastError()));
    } else if (readBytes != sizeof(buffer)) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDataFileHeaderSize,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), getId(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId());
    }

    ColumnDataBlockHeader header;
    header.deserialize(buffer);

    // Validate header
    if (header.m_version > ColumnDataBlockHeader::kCurrentVersion
            || header.m_fullColumnDataBlockId != m_header.m_fullColumnDataBlockId) {
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDataFileHeader,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), getId(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(),
                header.m_version);
    }

    m_header = header;
}

void ColumnDataBlock::writeHeader() const
{
    uint8_t header[ColumnDataBlockHeader::kSerializedSize];
    m_header.serialize(header);
    if (m_file->write(header, sizeof(header), 0) != sizeof(header)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteColumnDataBlockFile,
                m_column.getDatabaseName(), m_column.getTableName(), m_column.getName(), getId(),
                m_column.getDatabaseUuid(), m_column.getTableId(), m_column.getId(), 0,
                sizeof(header), m_file->getLastError(), std::strerror(m_file->getLastError()));
    }
    m_headerModified = false;
}

}  // namespace siodb::iomgr::dbengine
