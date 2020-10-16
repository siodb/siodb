// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UniqueLinearIndex.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "FileData.h"
#include "../Debug.h"
#include "../IndexColumn.h"
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/config/Config.h>
#include <siodb/common/config/SiodbDataFileDefs.h>
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FSUtils.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// System headers
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace siodb::iomgr::dbengine {

UniqueLinearIndex::UniqueLinearIndex(Table& table, IndexType type, std::string&& name,
        const IndexKeyTraits& keyTraits, std::size_t valueSize, KeyCompareFunction keyCompare,
        const IndexColumnSpecification& columnSpec, std::uint32_t dataFileSize,
        std::optional<std::string>&& description)
    : Index(table, type, std::move(name), keyTraits, valueSize, keyCompare, true,
            IndexColumnSpecificationList {columnSpec}, std::move(description))
    , m_dataFileSize(validateIndexFileSize(dataFileSize))
    , m_validatedKeySize(validateKeySize())
    , m_isSignedKey(validateKeyType(keyTraits))
    , m_sortDescending(columnSpec.m_sortDescending)
    , m_recordSize(computeIndexRecordSize())
    , m_numberOfRecordsPerFile(computeNumberOfRecordsPerFile())
    , m_minPossibleKey(keyTraits.getMinKey())
    , m_maxPossibleKey(keyTraits.getMaxKey())
    , m_maxPossibleFileId(computeMaxPossibleFileId())
    , m_fileCache(*this, kFileCacheCapacity)
    , m_minKey(m_maxPossibleKey)
    , m_maxKey(m_minPossibleKey)
    , m_minNumericKey(0)
    , m_maxNumericKey(0)
{
    createInitializationFlagFile();
}

UniqueLinearIndex::UniqueLinearIndex(Table& table, const IndexRecord& indexRecord,
        const IndexKeyTraits& keyTraits, std::size_t valueSize, KeyCompareFunction keyCompare)
    : Index(table, indexRecord, keyTraits, valueSize, keyCompare)
    , m_dataFileSize(validateIndexFileSize(indexRecord.m_dataFileSize))
    , m_validatedKeySize(validateKeySize())
    , m_isSignedKey(validateKeyType(keyTraits))
    , m_sortDescending(m_columns.at(0)->isDescendingSortOrder())
    , m_recordSize(computeIndexRecordSize())
    , m_numberOfRecordsPerFile(computeNumberOfRecordsPerFile())
    , m_minPossibleKey(keyTraits.getMinKey())
    , m_maxPossibleKey(keyTraits.getMaxKey())
    , m_maxPossibleFileId(computeMaxPossibleFileId())
    , m_fileIds(scanFiles())
    , m_fileCache(*this, kFileCacheCapacity)
    , m_minKey(findLeadingKey())
    , m_maxKey(findTrailingKey())
    , m_minNumericKey(0)
    , m_maxNumericKey(0)
{
    if (m_keyCompare(m_minKey.data(), m_maxKey.data()) <= 0) {
        m_minNumericKey = decodeKey(m_minKey.data());
        m_maxNumericKey = decodeKey(m_maxKey.data());
        if (m_minNumericKey > m_maxNumericKey) std::swap(m_minNumericKey, m_maxNumericKey);
    }

    // Log this always
    LOG_DEBUG << "Index " << makeDisplayName() << ": fileCount=" << m_fileIds.size()
              << ", minKey=" << decodeKey(m_minKey.data())
              << ", maxKey=" << decodeKey(m_maxKey.data());
}

std::uint32_t UniqueLinearIndex::getDataFileSize() const noexcept
{
    return m_dataFileSize;
}

bool UniqueLinearIndex::insert(const void* key, const void* value)
{
    const auto numericKey = decodeKey(key);
    const auto fileId = getFileIdForKey(numericKey);
    auto file = findFile(fileId);
    if (!file) file = makeFile(fileId);
    const auto offset = file->getRecordOffsetInMemory(numericKey);
    const auto record = file->getBuffer() + offset;
    const bool keyAbsent = (*record == kValueStateFree);

    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": INSERT key=" << numericKey << " (fileId "
                               << fileId << ", offset " << offset << ", key "
                               << (keyAbsent ? "doesn't exists" : "exists") << ')');

    //if (getTableName() == "SYS_COLUMN_DEF_CONSTRAINTS" && numericKey == 3) {
    //    DBG_LOG_DEBUG("Insert #" << numericKey);
    //}

    if (keyAbsent) {
        // Update data
        file->update(offset + 1, value, m_valueSize);
        std::uint8_t state = kValueStateExists1;
        file->update(offset, &state, 1);

        // Update min and max keys
        if (m_keyCompare(m_maxKey.data(), m_minKey.data()) < 0) {
            // First record in the index
            std::memcpy(m_minKey.data(), key, m_keySize);
            std::memcpy(m_maxKey.data(), key, m_keySize);
            m_minNumericKey = numericKey;
            m_maxNumericKey = numericKey;
        } else {
            // There are some records in the index
            if (m_keyCompare(key, m_minKey.data()) < 0)
                std::memcpy(m_minKey.data(), key, m_keySize);
            if (m_keyCompare(key, m_maxKey.data()) > 0)
                std::memcpy(m_maxKey.data(), key, m_keySize);
            if (numericKey < m_minNumericKey) m_minNumericKey = numericKey;
            if (numericKey > m_maxNumericKey) m_maxNumericKey = numericKey;
        }
    }
    return keyAbsent;
}

std::uint64_t UniqueLinearIndex::erase(const void* key)
{
    // Find record
    const auto numericKey = decodeKey(key);
    auto file = findFile(getFileIdForKey(numericKey));
    if (!file) return 0;
    const auto offset = file->getRecordOffsetInMemory(numericKey);
    const auto record = file->getBuffer() + offset;
    const bool keyExists = *record != kValueStateFree;

    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": DELETE key=" << numericKey << " (fileId "
                               << fileId << ", offset " << offset << ", key "
                               << (keyExists ? "doesn't exist" : "exists") << ')');

    if (!keyExists) return 0;

    // Mark record as free
    std::uint8_t state = kValueStateFree;
    file->update(offset, &state, 1);

    updateMinAndMaxKeysAfterRemoval(key);

    return 1;
}

std::uint64_t UniqueLinearIndex::update(const void* key, const void* value)
{
    const auto numericKey = decodeKey(key);
    auto file = findFile(getFileIdForKey(numericKey));
    if (!file) return 0;
    const auto offset = file->getRecordOffsetInMemory(numericKey);
    const auto record = file->getBuffer() + offset;
    const bool keyExists = *record != kValueStateFree;

    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": UPDATE key=" << numericKey << " (fileId "
                               << fileId << ", offset " << offset << ", key "
                               << (keyExists ? "doesn't exist" : "exists") << ')');

    if (keyExists) {
        // Update data
        std::uint8_t state =
                (*record == kValueStateExists1) ? kValueStateExists2 : kValueStateExists1;
        file->update(offset + 1 + m_valueSize * (state - 1), value, m_valueSize);
        file->update(offset, &state, 1);
        return 1;
    }
    return 0;
}

void UniqueLinearIndex::flush()
{
    // Nothing to do here
}

std::uint64_t UniqueLinearIndex::find(const void* key, void* value, std::size_t count)
{
    if (count == 0) return 0;
    const auto numericKey = decodeKey(key);
    auto file = findFile(getFileIdForKey(numericKey));
    if (!file) return 0;
    const auto offset = file->getRecordOffsetInMemory(numericKey);
    const auto record = file->getBuffer() + offset;
    const bool keyExists = *record != kValueStateFree;

    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": GET key=" << numericKey << " (fileId "
                               << fileId << ", offset " << offset << ", key "
                               << (keyExists ? "doesn't exist" : "exists") << ')');

    if (keyExists) {
        if (SIODB_LIKELY(*record <= kValueStateExists2)) {
            ::memcpy(value, record + 1 + ((*record - 1) * m_valueSize), m_valueSize);
            return 1;
        }
        throwDatabaseError(IOManagerMessageId::kErrorUliCorrupted, getDatabaseName(),
                m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, numericKey);
    }
    return 0;
}

std::uint64_t UniqueLinearIndex::count(const void* key)
{
    const auto numericKey = decodeKey(key);
    auto file = findFile(getFileIdForKey(numericKey));
    if (!file) return 0;
    const auto offset = file->getRecordOffsetInMemory(numericKey);
    const auto record = file->getBuffer() + offset;
    const bool keyExists = *record != kValueStateFree;

    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": COUNT key=" << numericKey << " (fileId "
                               << fileId << ", offset " << offset << ", key "
                               << (keyExists ? "doesn't exist" : "exists") << ')');

    return keyExists ? 1 : 0;
}

bool UniqueLinearIndex::getMinKey(void* key)
{
    // Check that we have min and max keys
    if (m_keyCompare(m_minKey.data(), m_maxKey.data()) > 0) return false;
    // Copy min key
    std::memcpy(key, m_minKey.data(), m_keySize);
    return true;
}

bool UniqueLinearIndex::getMaxKey(void* key)
{
    // Check that we have min and max keys
    if (m_keyCompare(m_minKey.data(), m_maxKey.data()) > 0) return false;
    // Copy max key
    std::memcpy(key, m_maxKey.data(), m_keySize);
    return true;
}

bool UniqueLinearIndex::findFirstKey(void* key)
{
    return m_sortDescending ? findTrailingKey(key) : findLeadingKey(key);
}

bool UniqueLinearIndex::findLastKey(void* key)
{
    return m_sortDescending ? findLeadingKey(key) : findTrailingKey(key);
}

bool UniqueLinearIndex::findPreviousKey(const void* key, void* prevKey)
{
    return m_sortDescending ? findKeyAfter(key, prevKey) : findKeyBefore(key, prevKey);
}

bool UniqueLinearIndex::findNextKey(const void* key, void* nextKey)
{
    return m_sortDescending ? findKeyBefore(key, nextKey) : findKeyAfter(key, nextKey);
}

// ----- internals -----

std::uint32_t UniqueLinearIndex::validateIndexFileSize(std::uint32_t size)
{
    if (size < kMinDataFileSize)
        throw std::invalid_argument("UniqueLinearIndex: Index file size is too small");
    if (size > kMaxDataFileSize)
        throw std::invalid_argument("UniqueLinearIndex: Index file size is too large");
    return size;
}

io::FilePtr UniqueLinearIndex::createIndexFile(std::uint64_t fileId) const
{
    std::string tmpFilePath;
    const auto indexFilePath = makeIndexFilePath(fileId);

    // Create data file as temporary file
    constexpr int kBaseExtraOpenFlags = O_DSYNC;
    io::FilePtr file;
    try {
        try {
            file = m_table.getDatabase().createFile(m_dataDir, kBaseExtraOpenFlags | O_TMPFILE,
                    kDataFileCreationMode, m_dataFileSize);
        } catch (std::system_error& ex) {
            if (ex.code().value() != ENOTSUP) throw;
            // O_TMPFILE not supported, fallback to the named temporary file
            tmpFilePath = indexFilePath + kTempFileExtension;
            file = m_table.getDatabase().createFile(
                    tmpFilePath, kBaseExtraOpenFlags, kDataFileCreationMode, m_dataFileSize);
        }
    } catch (std::system_error& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateIndexFile, indexFilePath,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, ex.code().value(), std::strerror(ex.code().value()));
    }

    stdext::buffer<std::uint8_t> buffer(kIndexFileHeaderSize, 0);

    // Write header
    IndexFileHeader indexFileHeader(getDatabaseUuid(), getTableId(), m_id, m_type);
    indexFileHeader.serialize(buffer.data());
    auto n = file->write(buffer.data(), buffer.size(), 0);
    if (n != buffer.size()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteIndexFile, indexFilePath,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, 0, buffer.size(), file->getLastError(), std::strerror(file->getLastError()),
                n);
    }

    // Write initial data
    const off_t dataOffset = buffer.size();
    buffer.resize(m_dataFileSize - kIndexFileHeaderSize);
    buffer.fill(0);
    n = file->write(buffer.data(), buffer.size(), dataOffset);
    if (n != buffer.size()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteIndexFile, indexFilePath,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, dataOffset, buffer.size(), file->getLastError(),
                std::strerror(file->getLastError()), n);
    }

    if (tmpFilePath.empty()) {
        // Link to the filesystem
        const auto fdPath = "/proc/self/fd/" + std::to_string(file->getFD());
        if (::linkat(AT_FDCWD, fdPath.c_str(), AT_FDCWD, indexFilePath.c_str(), AT_SYMLINK_FOLLOW)
                < 0) {
            const int errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotLinkIndexFile, indexFilePath,
                    getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id, errorCode, std::strerror(errorCode));
        }
    } else {
        // Rename temporary file to the regular one
        if (::rename(tmpFilePath.c_str(), indexFilePath.c_str()) < 0) {
            const int errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotRenameIndexFile, tmpFilePath,
                    indexFilePath, getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                    m_table.getId(), m_id, errorCode, std::strerror(errorCode));
        }
    }

    return file;
}

io::FilePtr UniqueLinearIndex::openIndexFile(std::uint64_t fileId) const
{
    // Open file
    const auto indexFilePath = makeIndexFilePath(fileId);
    io::FilePtr file;
    try {
        file = getDatabase().openFile(indexFilePath, O_DSYNC);
    } catch (std::system_error& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenIndexFile, indexFilePath,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, ex.code().value(), std::strerror(ex.code().value()));
    }

    // Check file size
    struct stat st;
    if (!file->stat(st)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotStatIndexFile, getDatabaseName(),
                getTableName(), getName(), file->getLastError(),
                std::strerror(file->getLastError()));
    }
    const auto expectedFileSize = getDataFileSize();
    if (st.st_size != expectedFileSize) {
        std::ostringstream str;
        str << "invalid file size " << st.st_size << " bytes, expected " << expectedFileSize
            << " bytes";
        throwDatabaseError(IOManagerMessageId::kErrorIndexFileCorrupted, getDatabaseName(),
                getTableName(), m_name, getDatabaseUuid(), getTableId(), m_id, str.str());
    }

    // Check header
    stdext::buffer<std::uint8_t> buffer(kIndexFileHeaderSize);
    const auto n = file->read(buffer.data(), buffer.size(), 0);
    if (n != buffer.size()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadIndexFile, indexFilePath,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, 0, buffer.size(), file->getLastError(), std::strerror(file->getLastError()),
                n);
    }
    IndexFileHeader actualHeader(m_type);
    actualHeader.deserialize(buffer.data());
    const IndexFileHeader expectedHeader(getDatabaseUuid(), getTableId(), m_id, m_type);
    if (actualHeader != expectedHeader) {
        throwDatabaseError(IOManagerMessageId::kErrorIndexFileCorrupted, indexFilePath,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, "invalid header");
    }

    return file;
}

uli::FileDataPtr UniqueLinearIndex::findFileChecked(std::uint64_t fileId)
{
    auto file = findFile(fileId);
    if (file) return file;
    throwDatabaseError(IOManagerMessageId::kErrorUliMissingFileWhenExpected, getDatabaseName(),
            m_table.getName(), m_name, fileId, getDatabaseUuid(), m_table.getId(), m_id);
}

uli::FileDataPtr UniqueLinearIndex::findFile(std::uint64_t fileId)
{
    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": Getting file #" << fileId);

    if (fileId > m_maxPossibleFileId) throw std::out_of_range("Index file ID is out of range");

    if (m_fileIds.count(fileId) == 0) return nullptr;

    uli::FileDataPtr fileData;
    auto maybeFileData = m_fileCache.get(fileId);
    if (maybeFileData)
        fileData = *maybeFileData;
    else {
        fileData = std::make_shared<uli::FileData>(*this, fileId, openIndexFile(fileId));
        m_fileCache.emplace(fileId, fileData);
    }

    return fileData;
}

uli::FileDataPtr UniqueLinearIndex::makeFile(std::uint64_t fileId)
{
    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": Creating file " << fileId);
    auto indexFile = createIndexFile(fileId);
    auto fileData = std::make_shared<uli::FileData>(*this, fileId, std::move(indexFile));
    m_fileIds.insert(fileId);
    m_fileCache.emplace(fileId, fileData);
    return fileData;
}

std::uint64_t UniqueLinearIndex::decodeKey(const void* key) const noexcept
{
    if (m_isSignedKey) {
        switch (m_keySize) {
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
            case 1: return encodeSignedInt(*reinterpret_cast<const std::int8_t*>(key));
            case 2: return encodeSignedInt(*reinterpret_cast<const std::int16_t*>(key));
            case 4: return encodeSignedInt(*reinterpret_cast<const std::int32_t*>(key));
            case 8: return encodeSignedInt(*reinterpret_cast<const std::int64_t*>(key));
#else
            case 1: return encodeSignedInt(*reinterpret_cast<const std::uint8_t*>(key));
            case 2: {
                std::int16_t v = 0;
                ::pbeDecodeInt16(reinterpret_cast<const std::uint8_t*>(key), &v);
                return encodeSignedInt(v);
            }
            case 4: {
                std::int32_t v = 0;
                ::pbeDecodeInt32(reinterpret_cast<const std::uint8_t*>(key), &v);
                return encodeSignedInt(v);
            }
            case 8: {
                std::int64_t v = 0;
                ::pbeDecodeInt64(reinterpret_cast<const std::uint8_t*>(key), &v);
                return encodeSignedInt(v);
            }
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
            default: abort();
        }
    } else {
        switch (m_keySize) {
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
            case 1: return *reinterpret_cast<const std::uint8_t*>(key);
            case 2: return *reinterpret_cast<const std::uint16_t*>(key);
            case 4: return *reinterpret_cast<const std::uint32_t*>(key);
            case 8: return *reinterpret_cast<const std::uint64_t*>(key);
#else
            case 1: return *reinterpret_cast<const std::uint8_t*>(key);
            case 2: {
                std::uint16_t result = 0;
                ::pbeDecodeUInt16(reinterpret_cast<const std::uint8_t*>(key), &result);
                return result;
            }
            case 4: {
                std::uint32_t result = 0;
                ::pbeDecodeUInt32(reinterpret_cast<const std::uint8_t*>(key), &result);
                return result;
            }
            case 8: {
                std::uint64_t result = 0;
                ::pbeDecodeUInt64(reinterpret_cast<const std::uint8_t*>(key), &result);
                return result;
            }
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
            default: abort();
        }
    }
}

void UniqueLinearIndex::encodeKey(std::uint64_t numericKey, void* key) const noexcept
{
    if (m_isSignedKey) {
        switch (m_keySize) {
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
            case 1: {
                *reinterpret_cast<std::int8_t*>(key) = decodeSignedInt8(numericKey);
                return;
            }
            case 2: {
                *reinterpret_cast<std::int16_t*>(key) = decodeSignedInt16(numericKey);
                return;
            }
            case 4: {
                *reinterpret_cast<std::int32_t*>(key) = decodeSignedInt32(numericKey);
                return;
            }
            case 8: {
                *reinterpret_cast<std::int64_t*>(key) = decodeSignedInt64(numericKey);
                return;
            }
#else
            case 1: {
                *reinterpret_cast<std::int8_t*>(key) = decodeSignedInt8(numericKey);
                return;
            }
            case 2: {
                ::pbeEncodeInt16(
                        decodeSignedInt16(numericKey), reinterpret_cast<std::uint8_t*>(key));
                return;
            }
            case 4: {
                ::pbeEncodeInt32(
                        decodeSignedInt32(numericKey), reinterpret_cast<std::uint8_t*>(key));
                return;
            }
            case 8: {
                ::pbeEncodeInt64(
                        decodeSignedInt64(numericKey), reinterpret_cast<std::uint8_t*>(key));
                return;
            }
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
            default: abort();
        }
    } else {
        switch (m_keySize) {
#ifdef SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
            case 1: {
                *reinterpret_cast<std::uint8_t*>(key) = static_cast<std::uint8_t>(numericKey);
                return;
            }
            case 2: {
                *reinterpret_cast<std::uint16_t*>(key) = static_cast<std::uint16_t>(numericKey);
                return;
            }
            case 4: {
                *reinterpret_cast<std::uint32_t*>(key) = static_cast<std::uint32_t>(numericKey);
                return;
            }
            case 8: {
                *reinterpret_cast<std::uint64_t*>(key) = static_cast<std::uint64_t>(numericKey);
                return;
            }
#else
            case 1: {
                *reinterpret_cast<std::uint8_t*>(key) = static_cast<std::uint8_t>(numericKey);
                return;
            }
            case 2: {
                ::pbeEncodeUInt16(static_cast<std::uint16_t>(numericKey),
                        reinterpret_cast<std::uint8_t*>(key));
                return;
            }
            case 4: {
                ::pbeEncodeUInt32(static_cast<std::uint32_t>(numericKey),
                        reinterpret_cast<std::uint8_t*>(key));
                return;
            }
            case 8: {
                ::pbeEncodeUInt64(static_cast<std::uint64_t>(numericKey),
                        reinterpret_cast<std::uint8_t*>(key));
                return;
            }
#endif  // SIODB_PLATFORM_SUPPORTS_UNALIGNED_DATA_ACCESS
            default: abort();
        }
    }
}

std::size_t UniqueLinearIndex::validateKeySize() const
{
    switch (m_keySize) {
        case 1:
        case 2:
        case 4:
        case 8: return m_keySize;
        default: throw std::invalid_argument("Invalid key size for the linear index");
    }
}

bool UniqueLinearIndex::validateKeyType(const IndexKeyTraits& keyTraits)
{
    switch (keyTraits.getNumericKeyType()) {
        case NumericKeyType::kSignedInt: return true;
        case NumericKeyType::kUnsignedInt: return false;
        default: throw std::invalid_argument("Invalid key type for linear index");
    }
}

BinaryValue UniqueLinearIndex::findLeadingKey()
{
    BinaryValue result(m_keySize);
    if (!findLeadingKey(result.data())) result = m_maxPossibleKey;
    return result;
}

bool UniqueLinearIndex::findLeadingKey(void* key)
{
    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findLeadingKey");
    for (const auto fileId : m_fileIds) {
        const auto file = findFileChecked(fileId);
        auto record = file->getBuffer();
        for (std::size_t i = 0; i < m_numberOfRecordsPerFile; ++i, record += m_recordSize) {
            if (*record != kValueStateFree) {
                const std::uint64_t numericKey = (fileId - 1) * m_numberOfRecordsPerFile + i;
                encodeKey(numericKey, key);

                ULI_DBG_LOG_DEBUG("Index " << makeDisplayName()
                                           << ": findLeadingKey: found active record file "
                                           << fileId << " record " << i << " key " << numericKey);

                return true;
            }
        }
    }
    return false;
}

BinaryValue UniqueLinearIndex::findTrailingKey()
{
    BinaryValue result(m_keySize);
    if (!findTrailingKey(result.data())) result = m_minPossibleKey;
    return result;
}

bool UniqueLinearIndex::findTrailingKey(void* key)
{
    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findTrailingKey");
    for (auto it = m_fileIds.crbegin(); it != m_fileIds.crend(); ++it) {
        const auto fileId = *it;
        const auto file = findFileChecked(fileId);
        const auto buffer = file->getBuffer();
        auto record = buffer + (m_numberOfRecordsPerFile - 1) * m_recordSize;
        for (std::size_t i = m_numberOfRecordsPerFile; i != 0; --i, record -= m_recordSize) {
            if (*record != kValueStateFree) {
                const std::uint64_t numericKey = (fileId - 1) * m_numberOfRecordsPerFile + i - 1;
                encodeKey(numericKey, key);

                ULI_DBG_LOG_DEBUG("Index " << makeDisplayName()
                                           << ": findTrailingKey: found active record file "
                                           << fileId << " record " << (i - 1) << " offset "
                                           << " key " << numericKey);

                return true;
            }
        }
    }
    return false;
}

bool UniqueLinearIndex::findKeyBefore(const void* key, void* keyBefore)
{
    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyBefore()");

    // Check is key before exists
    if (m_keyCompare(key, m_minKey.data()) == 0
            || m_keyCompare(key, m_minPossibleKey.data()) == 0) {
        ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyBefore: key is out of range");
        return false;
    }

    // Determine file ID
    const auto numericKey = decodeKey(key);
    auto fileId = getFileIdForKey(numericKey);

    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyBefore: key=" << numericKey
                               << " fileId=" << fileId);

    // Additionally validate file ID
    const auto minFileId = getMinAvailableFileId();
    if (fileId < minFileId) {
        ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyBefore: key=" << numericKey
                                   << " fileId=" << fileId << " is before minFileId " << minFileId);
        return false;
    }

    // Get record ID for the given key
    auto recordId = numericKey % m_numberOfRecordsPerFile;
    auto currentNumericKey = numericKey;

    // Step to a valid file
    auto fileIter = std::as_const(m_fileIds).lower_bound(fileId);
    if (fileIter == m_fileIds.cend() || *fileIter > fileId) {
        fileId = *--fileIter;
        recordId = m_numberOfRecordsPerFile;
        currentNumericKey = (fileId - 1) * m_numberOfRecordsPerFile + recordId;
    }

    while (true) {
        auto file = findFileChecked(fileId);
        ULI_DBG_LOG_DEBUG(
                "Index " << makeDisplayName() << ": findKeyBefore: obtained file #" << fileId);

        // Scan file
        if (recordId > 0) {
            --recordId;
            --currentNumericKey;
            const auto base = file->getBuffer();
            auto record = base + file->getRecordOffsetInMemory(recordId);
            for (; record >= base; --recordId, --currentNumericKey, record -= m_recordSize) {
                if (*record != kValueStateFree) {
                    encodeKey(currentNumericKey, keyBefore);
                    if (m_keyCompare(keyBefore, key) < 0) {
                        ULI_DBG_LOG_DEBUG("Index " << makeDisplayName()
                                                   << ": findKeyBefore: key=" << numericKey
                                                   << " result=" << currentNumericKey);
                        return true;
                    }
                }
            }
        }

        // Step to a previous file
        if (fileIter == m_fileIds.cbegin()) {
            ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyBefore: no more files");
            return false;
        }
        fileId = *--fileIter;
        recordId = m_numberOfRecordsPerFile;
        currentNumericKey = (fileId - 1) * m_numberOfRecordsPerFile + recordId;
    }
}

bool UniqueLinearIndex::findKeyAfter(const void* key, void* keyAfter)
{
    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyAfter()");

    // Check that next key exists
    if (m_keyCompare(key, m_maxKey.data()) == 0
            || m_keyCompare(key, m_maxPossibleKey.data()) == 0) {
        ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyAfter: key is out of range");
        return false;
    }

    // Determine file ID
    const auto numericKey = decodeKey(key);
    auto fileId = getFileIdForKey(numericKey);

    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyAfter: key=" << numericKey
                               << " fileId=" << fileId);

    // Additionally validate file ID
    const auto maxFileId = getMaxAvailableFileId();
    if (fileId > maxFileId) {
        ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyBefore: key=" << numericKey
                                   << " fileId=" << fileId << " is after maxFileId " << maxFileId);
        return false;
    }

    // Get record ID for the given key
    auto recordId = numericKey % m_numberOfRecordsPerFile;
    auto currentNumericKey = numericKey;

    // Step to a valid file
    auto fileIter = std::as_const(m_fileIds).lower_bound(fileId);
    if (fileIter == m_fileIds.cend()) {
        // Key belongs to a file before first available file
        fileIter = m_fileIds.cbegin();
        fileId = *fileIter;
        recordId = 0;
        currentNumericKey = (fileId - 1) * m_numberOfRecordsPerFile;
    } else if (*fileIter > fileId) {
        // Key belongs to a not available file in the middle
        fileId = *fileIter;
        recordId = 0;
        currentNumericKey = (fileId - 1) * m_numberOfRecordsPerFile;
    } else {
        // File is available, step to next record in the file
        ++recordId;
        ++currentNumericKey;
    }

    while (true) {
        // Obtain file
        const auto file = findFileChecked(fileId);
        ULI_DBG_LOG_DEBUG(
                "Index " << makeDisplayName() << ": findKeyAfter: obtained file #" << fileId);

        // Scan file
        const auto base = file->getBuffer();
        auto record = base + file->getRecordOffsetInMemory(recordId);
        const auto end = base + m_numberOfRecordsPerFile * m_recordSize;
        for (; record != end; ++recordId, ++currentNumericKey, record += m_recordSize) {
            if (*record != kValueStateFree) {
                encodeKey(currentNumericKey, keyAfter);
                if (m_keyCompare(keyAfter, key) > 0) {
                    ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyAfter: key="
                                               << numericKey << " result=" << currentNumericKey);
                    return true;
                }
            }
        }

        // Step to a next file
        if (++fileIter == m_fileIds.cend()) {
            ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": findKeyAfter: no more files");
            return false;
        }
        fileId = *fileIter;
        recordId = 0;
        currentNumericKey = (fileId - 1) * m_numberOfRecordsPerFile;
    }
}

void UniqueLinearIndex::updateMinAndMaxKeysAfterRemoval(const void* removedKey)
{
    // Update min and max keys
    const bool isMinKeyRemoved = m_keyCompare(removedKey, m_minKey.data()) == 0;
    const bool isMaxKeyRemoved = m_keyCompare(removedKey, m_maxKey.data()) == 0;
    if (isMinKeyRemoved || isMaxKeyRemoved) {
        // Change of 2 keys must be exception-safe, so first prepare copies, then swap
        BinaryValue newMinKey, newMaxKey;
        if (isMinKeyRemoved && isMaxKeyRemoved) {
            newMinKey = m_maxPossibleKey;
            newMaxKey = m_minPossibleKey;
            m_minKey.swap(newMinKey);
            m_maxKey.swap(newMaxKey);
            m_minNumericKey = 0;
            m_maxNumericKey = 0;
        } else {
            BinaryValue lesserKey, greaterKey;

            if (isMinKeyRemoved) {
                lesserKey.resize(m_keySize);
                if (m_sortDescending ? findNextKey(removedKey, lesserKey.data())
                                     : findPreviousKey(removedKey, lesserKey.data())) {
                    newMinKey = lesserKey;
                } else {
                    lesserKey.clear();
                    greaterKey.resize(m_keySize);
                    if (m_sortDescending ? findPreviousKey(removedKey, greaterKey.data())
                                         : findNextKey(removedKey, greaterKey.data()))
                        newMinKey = greaterKey;
                    else {
                        throwDatabaseError(
                                IOManagerMessageId::kErrorUliMissingGreaterValueWhenExpected,
                                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                                m_table.getId(), m_id);
                    }
                }
            } else if (isMaxKeyRemoved) {
                if (greaterKey.empty()) {
                    greaterKey.resize(m_keySize);
                    if (!(m_sortDescending ? findNextKey(removedKey, greaterKey.data())
                                           : findPreviousKey(removedKey, greaterKey.data()))) {
                        greaterKey.clear();
                    }
                }
                if (greaterKey.empty()) {
                    lesserKey.resize(m_keySize);
                    if (m_sortDescending ? findNextKey(removedKey, lesserKey.data())
                                         : findPreviousKey(removedKey, lesserKey.data()))
                        newMaxKey = lesserKey;
                    else {
                        throwDatabaseError(
                                IOManagerMessageId::kErrorUliMissingLessValueWhenExpected,
                                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                                m_table.getId(), m_id);
                    }
                } else
                    newMaxKey = greaterKey;
            }

            if (!newMinKey.empty()) {
                m_minKey.swap(newMinKey);
                m_minNumericKey = decodeKey(m_minKey.data());
            }

            if (!newMaxKey.empty()) {
                m_maxKey.swap(newMaxKey);
                m_maxNumericKey = decodeKey(m_maxKey.data());
            }

            if (m_minNumericKey > m_maxNumericKey) std::swap(m_minNumericKey, m_maxNumericKey);
        }
    }
}

std::set<std::uint64_t> UniqueLinearIndex::scanFiles() const
{
    constexpr auto kIndexFilePrefixLength = ct_strlen(kIndexFilePrefix);
    constexpr auto kFileNameSurroundingLength =
            kIndexFilePrefixLength + ct_strlen(kDataFileExtension);
    constexpr auto kMinFileNameLength = kFileNameSurroundingLength + 1;
    std::set<std::uint64_t> fileIds;
    for (fs::directory_iterator it(m_dataDir), endIt; it != endIt; ++it) {
        const auto fileName = it->path().filename().generic_string();
        if (fileName.length() < kMinFileNameLength || fileName.find(kIndexFilePrefix) != 0)
            continue;
        const auto fileIdStr = fileName.substr(
                kIndexFilePrefixLength, fileName.length() - kFileNameSurroundingLength);
        std::uint64_t fileId;
        try {
            std::size_t pos = 0;
            fileId = std::stoull(fileIdStr, &pos);
            if (pos != fileIdStr.length() || fileId == 0)
                throw std::invalid_argument("Invalid file ID");
        } catch (std::exception& ex) {
            throwDatabaseError(IOManagerMessageId::kErrorUliInvalidIndexFileName, getDatabaseName(),
                    getTableName(), m_name, getDatabaseUuid(), getTableId(), m_id, fileName);
        }
        ULI_DBG_LOG_DEBUG("Index " << makeDisplayName() << ": scanFiles: adding file #" << fileId);
        fileIds.insert(fileId);
    }
    return fileIds;
}

std::uint64_t UniqueLinearIndex::computeMaxPossibleFileId() const noexcept
{
    const auto n = decodeKey(m_maxPossibleKey.data());
    return (n / m_numberOfRecordsPerFile) + (n % m_numberOfRecordsPerFile > 0 ? 1 : 0);
}

///////////////////// class UniqueLinearIndex::IndexFileHeader ////////////////////////////////////

std::uint8_t* UniqueLinearIndex::IndexFileHeader::serialize(std::uint8_t* buffer) const noexcept
{
    return IndexFileHeaderBase::serialize(buffer);
}

const std::uint8_t* UniqueLinearIndex::IndexFileHeader::deserialize(
        const std::uint8_t* buffer) noexcept
{
    return IndexFileHeaderBase::deserialize(buffer);
}

}  // namespace siodb::iomgr::dbengine
