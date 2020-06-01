// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UniqueLinearIndex.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "FileData.h"
#include "Node.h"
#include "../IndexColumn.h"
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/config/Config.h>
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/log/Log.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FsUtils.h>
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
    , m_dataFileSize(dataFileSize)
    , m_validatedKeySize(validateKeySize())
    , m_isSignedKey(validateKeyType(keyTraits))
    , m_sortDescending(columnSpec.m_sortDescending)
    , m_recordSize(m_valueSize + 1)
    , m_numberOfRecordsPerNode(uli::Node::kSize / m_recordSize)
    , m_numberOfNodesPerFile(m_dataFileSize / uli::Node::kSize - 1)
    , m_numberOfRecordsPerFile(m_numberOfNodesPerFile * m_numberOfRecordsPerNode)
    , m_minPossibleKey(keyTraits.getMinKey())
    , m_maxPossibleKey(keyTraits.getMaxKey())
    , m_maxPossibleNodeId(computeMaxPossibleNodeId())
    , m_fileCache(*this, kFileCacheCapacity)
    , m_minKey(getLeadingKey())
    , m_maxKey(getTrailingKey())
{
    createInitializationFlagFile();

    // Log this always
    LOG_DEBUG << "Index " << getDisplayName() << ": fileCount=" << m_fileIds.size()
              << ", minKey=" << decodeKey(m_minKey.data())
              << ", maxKey=" << decodeKey(m_maxKey.data());
}

UniqueLinearIndex::UniqueLinearIndex(Table& table, const IndexRecord& indexRecord,
        const IndexKeyTraits& keyTraits, std::size_t valueSize, KeyCompareFunction keyCompare)
    : Index(table, indexRecord, keyTraits, valueSize, keyCompare)
    , m_dataFileSize(indexRecord.m_dataFileSize)
    , m_validatedKeySize(validateKeySize())
    , m_isSignedKey(validateKeyType(keyTraits))
    , m_sortDescending(m_columns.at(0)->isDescendingSortOrder())
    , m_recordSize(m_valueSize + 1)
    , m_numberOfRecordsPerNode(uli::Node::kSize / m_recordSize)
    , m_numberOfNodesPerFile(m_dataFileSize / uli::Node::kSize - 1)
    , m_numberOfRecordsPerFile(m_numberOfNodesPerFile * m_numberOfRecordsPerNode)
    , m_minPossibleKey(keyTraits.getMinKey())
    , m_maxPossibleKey(keyTraits.getMaxKey())
    , m_maxPossibleNodeId(computeMaxPossibleNodeId())
    , m_fileIds(scanFiles())
    , m_fileCache(*this, kFileCacheCapacity)
    , m_minKey(getLeadingKey())
    , m_maxKey(getTrailingKey())
{
    // Log this always
    LOG_DEBUG << "Index " << getDisplayName() << ": fileCount=" << m_fileIds.size()
              << ", minKey=" << decodeKey(m_minKey.data())
              << ", maxKey=" << decodeKey(m_maxKey.data());
}

std::uint32_t UniqueLinearIndex::getDataFileSize() const noexcept
{
    return m_dataFileSize;
}

bool UniqueLinearIndex::insert(const void* key, const void* value, bool replaceExisting)
{
    const auto numericKey = decodeKey(key);
    const auto nodeId = getNodeIdForKey(numericKey);
    auto node = getNode(nodeId);
    if (!node) node = makeNode(nodeId);
    const auto offset = (numericKey % m_numberOfRecordsPerNode) * m_recordSize;
    const auto record = node->m_data + offset;
    const bool keyDoesntExist = *record != kValueStateExists;

    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": INSERT key=" << numericKey << " (node "
                               << node.get() << " id " << node->m_nodeId << ", tag " << node->m_tag
                               << ", offset " << offset << ", key "
                               << (keyDoesntExist ? "doesn't exist" : "exists") << ')');

    if (keyDoesntExist || replaceExisting) {
        // Store value
        ::memcpy(record + 1, value, m_valueSize);
        *record = kValueStateExists;
        node->m_modified = true;
        // Update min and max keys
        if (m_keyCompare(key, m_minKey.data()) < 0) std::memcpy(m_minKey.data(), key, m_keySize);
        if (m_keyCompare(key, m_maxKey.data()) > 0) std::memcpy(m_maxKey.data(), key, m_keySize);
    }
    return keyDoesntExist;
}

std::uint64_t UniqueLinearIndex::erase(const void* key)
{
    // Find record
    const auto numericKey = decodeKey(key);
    auto node = getNode(getNodeIdForKey(numericKey));
    if (!node) return 0;
    const auto offset = (numericKey % m_numberOfRecordsPerNode) * m_recordSize;
    const auto record = node->m_data + offset;
    const bool keyExists = (*record == kValueStateExists);

    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": ERASE key=" << numericKey << " (node "
                               << node.get() << " id " << node->m_nodeId << ", tag " << node->m_tag
                               << ", offset " << offset << ", key "
                               << (keyExists ? "exists" : "doesn't exist") << ')');

    if (!keyExists) return 0;

    updateMinMaxKeysAfterRemoval(key);

    // Mark record as free
    *record = kValueStateFree;
    node->m_modified = true;
    return 1;
}

std::uint64_t UniqueLinearIndex::update(const void* key, const void* value)
{
    const auto numericKey = decodeKey(key);
    auto node = getNode(getNodeIdForKey(numericKey));
    if (!node) return 0;
    const auto offset = (numericKey % m_numberOfRecordsPerNode) * m_recordSize;
    const auto record = node->m_data + offset;
    const bool keyExists = (*record == kValueStateExists);

    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": UPDATE key=" << numericKey << " (node "
                               << node.get() << " id " << node->m_nodeId << ", tag " << node->m_tag
                               << ", offset " << offset << ", key "
                               << (keyExists ? "exists" : "doesn't exist") << ')');

    if (keyExists) {
        ::memcpy(record + 1, value, m_valueSize);
        node->m_modified = true;
        return 1;
    }
    return 0;
}

bool UniqueLinearIndex::markAsDeleted(const void* key, const void* value)
{
    const auto numericKey = decodeKey(key);
    auto node = getNode(getNodeIdForKey(numericKey));
    if (!node) return 0;
    const auto offset = (numericKey % m_numberOfRecordsPerNode) * m_recordSize;
    const auto record = node->m_data + offset;
    const bool keyExists = (*record == kValueStateExists);

    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": markAsDeleted key=" << numericKey
                               << " (node " << node.get() << " id " << node->m_nodeId << ", tag "
                               << node->m_tag << ", offset " << offset << ", key "
                               << (keyExists ? "exists" : "doesn't exist") << ')');

    if (keyExists) {
        updateMinMaxKeysAfterRemoval(key);
        ::memcpy(record + 1, value, m_valueSize);
        *record = kValueStateDeleted;
        node->m_modified = true;
    }
    return keyExists;
}

void UniqueLinearIndex::flush()
{
    for (const auto& e : m_fileCache) {
        try {
            e.second->m_nodeCache.flush();
        } catch (std::exception& ex) {
            throwDatabaseError(IOManagerMessageId::kErrorUliFlushNodeCacheFailed, getDatabaseName(),
                    m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(), m_id, e.first,
                    ex.what());
        }
    }
}

std::uint64_t UniqueLinearIndex::getValue(const void* key, void* value, std::size_t count)
{
    if (count == 0) return 0;
    const auto numericKey = decodeKey(key);
    auto node = getNode(getNodeIdForKey(numericKey));
    if (!node) return 0;
    const auto offset = (numericKey % m_numberOfRecordsPerNode) * m_recordSize;
    const auto record = node->m_data + offset;
    const auto keyExists = (*record == kValueStateExists);

    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getValue: key=" << numericKey << " (node "
                               << node.get() << " id " << node->m_nodeId << ", tag " << node->m_tag
                               << ", offset " << offset << ", key "
                               << (keyExists ? "exists" : "doesn't exist") << ')');

    if (keyExists) {
        ::memcpy(value, record + 1, m_valueSize);
        return 1;
    }
    return 0;
}

std::uint64_t UniqueLinearIndex::count(const void* key)
{
    const auto numericKey = decodeKey(key);
    auto node = getNode(getNodeIdForKey(numericKey));
    if (!node) return 0;
    const auto offset = (numericKey % m_numberOfRecordsPerNode) * m_recordSize;
    const auto record = node->m_data + offset;
    const auto keyExists = (*record == kValueStateExists);

    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": COUNT key=" << numericKey << " (node "
                               << node.get() << " id " << node->m_nodeId << ", tag " << node->m_tag
                               << ", offset " << offset << ", key "
                               << (keyExists ? "exists" : "doesn't exist") << ')');

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

bool UniqueLinearIndex::getFirstKey(void* key)
{
    return m_sortDescending ? getTrailingKey(key) : getLeadingKey(key);
}

bool UniqueLinearIndex::getLastKey(void* key)
{
    return m_sortDescending ? getLeadingKey(key) : getTrailingKey(key);
}

bool UniqueLinearIndex::getPrevKey(const void* key, void* prevKey)
{
    return m_sortDescending ? getKeyAfter(key, prevKey) : getKeyBefore(key, prevKey);
}

bool UniqueLinearIndex::getNextKey(const void* key, void* nextKey)
{
    return m_sortDescending ? getKeyBefore(key, nextKey) : getKeyAfter(key, nextKey);
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
            // O_TMPFILE not supported, fallback to named temporary file
            tmpFilePath = indexFilePath + kTempFileExtension;
            file = m_table.getDatabase().createFile(
                    tmpFilePath, kBaseExtraOpenFlags, kDataFileCreationMode, m_dataFileSize);
        }
    } catch (std::system_error& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateIndexFile, indexFilePath,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, ex.code().value(), std::strerror(ex.code().value()));
    }

    stdext::buffer<std::uint8_t> buffer(uli::Node::kSize, 0);

    // Write header
    IndexFileHeader indexFileHeader;
    indexFileHeader.serialize(buffer.data());
    if (file->write(buffer.data(), buffer.size(), 0) != buffer.size()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteIndexFile, indexFilePath,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, 0, buffer.size(), file->getLastError(), std::strerror(file->getLastError()));
    }

    // Write nodes
    const off_t firstNodeOffset = buffer.size();
    buffer.resize(m_numberOfNodesPerFile * uli::Node::kSize);
    buffer.fill(0);
    if (file->write(buffer.data(), buffer.size(), firstNodeOffset) != buffer.size()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteIndexFile, indexFilePath,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, firstNodeOffset, buffer.size(), file->getLastError(),
                std::strerror(file->getLastError()));
    }

    if (tmpFilePath.empty()) {
        // Link to the filesystem
        const auto fdPath = "/proc/self/fd/" + std::to_string(file->getFd());
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
    const auto indexFilePath = makeIndexFilePath(fileId);
    io::FilePtr file;
    try {
        file = getDatabase().openFile(indexFilePath, O_DSYNC);
    } catch (std::system_error& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenIndexFile, indexFilePath,
                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(), m_table.getId(),
                m_id, ex.code().value(), std::strerror(ex.code().value()));
    }
    return file;
}

uli::NodePtr UniqueLinearIndex::getNodeChecked(std::uint64_t nodeId)
{
    auto node = getNode(nodeId);
    if (node) return node;
    throwDatabaseError(IOManagerMessageId::kErrorUliMissingNodeWhenExpected, getDatabaseName(),
            m_table.getName(), m_name, nodeId, getDatabaseUuid(), m_table.getId(), m_id);
}

uli::NodePtr UniqueLinearIndex::getNode(std::uint64_t nodeId)
{
    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": Getting node " << nodeId);

    if (nodeId > m_maxPossibleNodeId) throw std::out_of_range("Index node ID is out of range");

    const auto fileId = getFileIdForNode(nodeId);
    if (m_fileIds.count(fileId) == 0) return nullptr;

    uli::FileDataPtr fileData;
    auto maybeFileData = m_fileCache.get(fileId);
    if (maybeFileData)
        fileData = *maybeFileData;
    else {
        fileData = std::make_shared<uli::FileData>(*this, openIndexFile(fileId), fileId);
        m_fileCache.emplace(fileId, fileData);
    }

    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": Getting node " << nodeId << " from file #"
                               << fileData->getFileId());

    return fileData->getNode(nodeId);
}

uli::NodePtr UniqueLinearIndex::makeNode(std::uint64_t nodeId)
{
    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": Creating node " << nodeId);
    const auto fileId = getFileIdForNode(nodeId);
    auto indexFile = createIndexFile(fileId);
    auto fileData = std::make_shared<uli::FileData>(*this, std::move(indexFile), fileId);
    m_fileIds.insert(fileId);
    m_fileCache.emplace(fileId, fileData);
    return fileData->getNode(nodeId);
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

BinaryValue UniqueLinearIndex::getLeadingKey()
{
    BinaryValue result(m_keySize);
    if (!getLeadingKey(result.data())) result = m_maxPossibleKey;
    return result;
}

bool UniqueLinearIndex::getLeadingKey(void* key)
{
    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getLeadingKey");
    for (const auto fileId : m_fileIds) {
        std::uint64_t nodeId = (fileId - 1) * m_numberOfNodesPerFile + 1;
        for (std::size_t j = 0; j <= m_numberOfNodesPerFile; ++j, ++nodeId) {
            const auto node = getNodeChecked(nodeId);
            auto record = node->m_data;
            for (std::size_t i = 0; i < m_numberOfRecordsPerNode; ++i, record += m_recordSize) {
                if (*record == kValueStateExists) {
                    const std::uint64_t numericKey = (nodeId - 1) * m_numberOfRecordsPerNode + i;
                    encodeKey(numericKey, key);

                    ULI_DBG_LOG_DEBUG("Index " << getDisplayName()
                                               << ": getLeadingKey: found active record node "
                                               << nodeId << " tag " << node->m_tag << " record "
                                               << i << " offset " << (record - node->m_data)
                                               << " key " << numericKey);

                    return true;
                }
            }
        }
    }
    return false;
}

BinaryValue UniqueLinearIndex::getTrailingKey()
{
    BinaryValue result(m_keySize);
    if (!getTrailingKey(result.data())) result = m_minPossibleKey;
    return result;
}

bool UniqueLinearIndex::getTrailingKey(void* key)
{
    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getTrailingKey");
    for (auto it = m_fileIds.crbegin(); it != m_fileIds.crend(); ++it) {
        const auto fileId = *it;
        std::uint64_t nodeId = fileId * m_numberOfNodesPerFile;
        for (std::size_t j = 0; j < m_numberOfNodesPerFile; ++j, --nodeId) {
            const auto node = getNodeChecked(nodeId);
            auto record = node->m_data + (m_numberOfRecordsPerNode - 1) * m_recordSize;
            for (std::size_t i = 0; i < m_numberOfRecordsPerNode; ++i, record -= m_recordSize) {
                if (*record == kValueStateExists) {
                    const std::uint64_t numericKey = nodeId * m_numberOfRecordsPerNode - (i + 1);
                    encodeKey(numericKey, key);

                    ULI_DBG_LOG_DEBUG("Index " << getDisplayName()
                                               << ": getTrailingKey: found active record node "
                                               << nodeId << " tag " << node->m_tag << " record "
                                               << (m_numberOfRecordsPerNode - 1 - i) << " offset "
                                               << (record - node->m_data) << " key " << numericKey);

                    return true;
                }
            }
        }
    }
    return false;
}

bool UniqueLinearIndex::getKeyBefore(const void* key, void* keyBefore)
{
    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyBefore()");

    // Check is key before exists
    if (m_keyCompare(key, m_minKey.data()) == 0
            || m_keyCompare(key, m_minPossibleKey.data()) == 0) {
        ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyBefore: key is out of range");
        return false;
    }

    // Determine node ID
    const auto numericKey = decodeKey(key);
    auto nodeId = getNodeIdForKey(numericKey);

    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyBefore: key=" << numericKey
                               << " nodeId=" << nodeId);

    // Additionally validate node ID
    const auto minNodeId = getMinAvailableNodeId();
    if (nodeId < minNodeId) {
        ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyBefore: key=" << numericKey
                                   << " nodeId=" << nodeId << " is before minNodeId " << minNodeId);
        return false;
    }

    // Get record ID for the given key
    auto recordId = numericKey % m_numberOfRecordsPerNode;

    // Step to valid file
    auto fileId = getFileIdForNode(nodeId);
    auto firstNodeId = (fileId - 1) * m_numberOfNodesPerFile + 1;
    auto fileIter = std::as_const(m_fileIds).lower_bound(fileId);
    if (fileIter == m_fileIds.cend() || *fileIter > fileId) {
        fileId = *--fileIter;
        firstNodeId = (fileId - 1) * m_numberOfNodesPerFile + 1;
        nodeId = firstNodeId + m_numberOfNodesPerFile - 1;
        recordId = m_numberOfRecordsPerNode;
    }

    while (true) {
        // Obtain node
        const auto node = getNodeChecked(nodeId);
        ULI_DBG_LOG_DEBUG(
                "Index " << getDisplayName() << ": getKeyBefore: obtained node #" << nodeId);

        // Scan node
        if (recordId > 0) {
            --recordId;
            for (auto record = node->m_data + recordId * m_recordSize; record >= node->m_data;
                    --recordId, record -= m_recordSize) {
                if (*record == kValueStateExists) {
                    const std::uint64_t numericKey =
                            (nodeId - 1) * m_numberOfRecordsPerNode + recordId;
                    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyBefore: key="
                                               << numericKey << " result=" << numericKey);
                    encodeKey(numericKey, keyBefore);
                    return true;
                }
            }
        }

        // Step back
        if (nodeId > firstNodeId) {
            // Step to previous node
            --nodeId;
        } else {
            // Step to previous file
            if (fileIter == m_fileIds.cbegin()) {
                ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyBefore: no more files");
                return false;
            }
            fileId = *--fileIter;
            firstNodeId = (fileId - 1) * m_numberOfNodesPerFile + 1;
            nodeId = firstNodeId + m_numberOfNodesPerFile - 1;
        }
        recordId = m_numberOfRecordsPerNode;
    }
}

bool UniqueLinearIndex::getKeyAfter(const void* key, void* keyAfter)
{
    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyAfter()");

    // Check that next key exists
    if (m_keyCompare(key, m_maxKey.data()) == 0
            || m_keyCompare(key, m_maxPossibleKey.data()) == 0) {
        ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyAfter: key is out of range");
        return false;
    }

    // Determine node ID
    const auto numericKey = decodeKey(key);
    auto nodeId = getNodeIdForKey(numericKey);

    ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyAfter: key=" << numericKey
                               << " nodeId=" << nodeId);

    // Additionally validate node ID
    const auto maxNodeId = getMaxAvailableNodeId();
    if (nodeId > maxNodeId) {
        ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyBefore: key=" << numericKey
                                   << " nodeId=" << nodeId << " is after maxNodeId " << maxNodeId);
        return false;
    }

    // Get record ID for the given key
    auto recordId = numericKey % m_numberOfRecordsPerNode;

    // Step to valid file
    auto fileId = getFileIdForNode(nodeId);
    auto lastNodeId = fileId * m_numberOfNodesPerFile;
    auto fileIter = std::as_const(m_fileIds).lower_bound(fileId);
    if (fileIter == m_fileIds.cend()) {
        // Key belongs to a file before first available file
        fileIter = m_fileIds.cbegin();
        fileId = *fileIter;
        lastNodeId = fileId * m_numberOfNodesPerFile;
        nodeId = lastNodeId - m_numberOfNodesPerFile + 1;
        recordId = 0;
    } else if (*fileIter > fileId) {
        // Key belongs to a not available file in the middle
        fileId = *fileIter;
        lastNodeId = fileId * m_numberOfNodesPerFile;
        nodeId = lastNodeId - m_numberOfNodesPerFile + 1;
        recordId = 0;
    } else {
        // File is available, step to next record in the node
        ++recordId;
    }

    while (true) {
        // Obtain node
        const auto node = getNodeChecked(nodeId);
        ULI_DBG_LOG_DEBUG(
                "Index " << getDisplayName() << ": getKeyAfter: obtained node #" << nodeId);

        // Scan node
        for (auto record = node->m_data + recordId * m_recordSize;
                recordId < m_numberOfRecordsPerNode; ++recordId, record += m_recordSize) {
            if (*record == kValueStateExists) {
                const std::uint64_t numericKey = (nodeId - 1) * m_numberOfRecordsPerNode + recordId;
                encodeKey(numericKey, keyAfter);
                ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyAfter: key="
                                           << numericKey << " result=" << numericKey);
                return true;
            }
        }

        // Step forward
        if (nodeId < lastNodeId) {
            // Step to next node
            ++nodeId;
        } else {
            // Step to next file
            if (++fileIter == m_fileIds.cend()) {
                ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": getKeyAfter: no more files");
                return false;
            }
            fileId = *fileIter;
            lastNodeId = fileId * m_numberOfNodesPerFile;
            nodeId = lastNodeId - m_numberOfNodesPerFile + 1;
        }
        recordId = 0;
    }
}

void UniqueLinearIndex::updateMinMaxKeysAfterRemoval(const void* key)
{
    // Update min and max keys
    const bool isMinKey = m_keyCompare(key, m_minKey.data()) == 0;
    const bool isMaxKey = m_keyCompare(key, m_maxKey.data()) == 0;
    if (isMinKey || isMaxKey) {
        // Change of 2 keys must be exception-safe, so first prepare copies, then swap
        BinaryValue newMinKey, newMaxKey;
        if (isMinKey && isMaxKey) {
            newMinKey = m_maxPossibleKey;
            newMaxKey = m_minPossibleKey;
        } else {
            BinaryValue lessKey, greaterKey;

            if (isMinKey) {
                lessKey.resize(m_keySize);
                if (m_sortDescending ? getNextKey(key, lessKey.data())
                                     : getPrevKey(key, lessKey.data())) {
                    newMinKey = lessKey;
                } else {
                    lessKey.clear();
                    greaterKey.resize(m_keySize);
                    if (m_sortDescending ? getPrevKey(key, greaterKey.data())
                                         : getNextKey(key, greaterKey.data()))
                        newMinKey = greaterKey;
                    else {
                        throwDatabaseError(
                                IOManagerMessageId::kErrorUliMissingGreaterValueWhenExpected,
                                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                                m_table.getId(), m_id);
                    }
                }
            }

            if (isMaxKey) {
                if (greaterKey.empty()) {
                    greaterKey.resize(m_keySize);
                    if (!(m_sortDescending ? getNextKey(key, greaterKey.data())
                                           : getPrevKey(key, greaterKey.data()))) {
                        greaterKey.clear();
                    }
                }
                if (greaterKey.empty()) {
                    lessKey.resize(m_keySize);
                    if (m_sortDescending ? getNextKey(key, lessKey.data())
                                         : getPrevKey(key, lessKey.data()))
                        newMaxKey = lessKey;
                    else {
                        throwDatabaseError(
                                IOManagerMessageId::kErrorUliMissingLessValueWhenExpected,
                                getDatabaseName(), m_table.getName(), m_name, getDatabaseUuid(),
                                m_table.getId(), m_id);
                    }
                } else
                    newMaxKey = greaterKey;
            }
        }
        if (!newMinKey.empty()) m_minKey.swap(newMinKey);
        if (!newMaxKey.empty()) m_maxKey.swap(newMaxKey);
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
            if (pos != fileIdStr.length()) throw std::invalid_argument("Invalid file ID");
        } catch (std::exception& ex) {
            continue;
        }
        ULI_DBG_LOG_DEBUG("Index " << getDisplayName() << ": scanFiles: adding file #" << fileId);
        fileIds.insert(fileId);
    }
    return fileIds;
}

std::uint64_t UniqueLinearIndex::computeMaxPossibleNodeId() const noexcept
{
    const auto n = decodeKey(m_maxPossibleKey.data());
    return (n / m_numberOfRecordsPerNode) + (n % m_numberOfRecordsPerNode > 0 ? 1 : 0);
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
