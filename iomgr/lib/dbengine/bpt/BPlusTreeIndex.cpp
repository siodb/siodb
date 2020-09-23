// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "BPlusTreeIndex.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/common/utils/FSUtils.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// CRT headers
#include <cstring>

// STL headers
#include <algorithm>
#include <fstream>
#include <sstream>

// System headers
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace siodb::iomgr::dbengine {

///////////////////// class BPlusTreeIndex /////////////////////////////////////////////////////////

BPlusTreeIndex::BPlusTreeIndex(Table& table, std::string&& name, const IndexKeyTraits& keyTraits,
        std::size_t valueSize, KeyCompareFunction keyCompare, bool unique,
        const IndexColumnSpecificationList& columns, std::uint32_t dataFileSize,
        std::optional<std::string>&& description)
    : Index(table, IndexType::kBPlusTreeIndex, std::move(name), keyTraits, valueSize, keyCompare,
            unique, columns, std::move(description))
    , m_dataFileSize(dataFileSize)
    , m_internalKvPairSize(m_keySize + sizeof(std::uint64_t))
    , m_branchingFactor(
              std::min((Node::kSize - InternalNodeHeader::kSerializedSize) / m_internalKvPairSize,
                      (Node::kSize - LeafNodeHeader::kSerializedSize) / m_kvPairSize))
    , m_splitThreshold((m_branchingFactor + 1) / 2)
    , m_indexFilePath(makeIndexFilePath(0))
    , m_file(createIndexFile())
    , m_nodeCount(1)
    , m_rootNodeId(1)
    , m_nextFreeNodeId(1)
    , m_nodeCache(*this, kNodeCacheCapacity)
{
    createInitializationFlagFile();
}

BPlusTreeIndex::BPlusTreeIndex(Table& table, const IndexRecord& indexRecord,
        const IndexKeyTraits& keyTraits, std::size_t valueSize, KeyCompareFunction keyCompare)
    : Index(table, indexRecord, keyTraits, valueSize, keyCompare)
    , m_dataFileSize(indexRecord.m_dataFileSize)
    , m_internalKvPairSize(m_keySize + sizeof(std::uint64_t))
    , m_branchingFactor(
              std::min((Node::kSize - InternalNodeHeader::kSerializedSize) / m_internalKvPairSize,
                      (Node::kSize - LeafNodeHeader::kSerializedSize) / m_kvPairSize))
    , m_splitThreshold((m_branchingFactor + 1) / 2)
    , m_indexFilePath(makeIndexFilePath(0))
    , m_file(openIndexFile())
    , m_nodeCount(calculateNodeCount())
    , m_rootNodeId(findRootNode())
    , m_nextFreeNodeId(1)
    , m_nodeCache(*this, kNodeCacheCapacity)
{
}

std::uint32_t BPlusTreeIndex::getDataFileSize() const noexcept
{
    return m_dataFileSize;
}

bool BPlusTreeIndex::insert(const void* key, const void* value)
{
    // Find a node which should contain the key
    auto node = findNode(key);
    assert(node->isLeaf());

    // Check if a key already exists
    auto insertPos = node->m_header.m_common.m_childCount;
    const auto it = std::lower_bound(
            node->begin(), node->end(), key, BinarySearchNodeDataCompareFunction(m_keyCompare));
    if (it != node->end()) {
        const auto newEntry = (*it).m_data;
        if (std::memcmp(newEntry, key, m_keySize) == 0) {
            // Key exists
            return false;
        }
        insertPos = (newEntry - node->m_data - LeafNodeHeader::kSerializedSize) / m_kvPairSize;
    }

    if (node->m_header.m_common.m_childCount < m_branchingFactor)
        insertNewEntryToNonFullLeafNode(*node, insertPos, key, value);
    else
        insertNewEntryToFullLeafNode(*node, insertPos, key, value);

    return true;
}

std::uint64_t BPlusTreeIndex::erase(const void* key)
{
    // TODO: Implement BPlusTreeIndex::erase()
    TEMPORARY_UNUSED(key);
    return 0;
}

std::uint64_t BPlusTreeIndex::update(const void* key, const void* value)
{
    // TODO: Implement BPlusTreeIndex::update()
    TEMPORARY_UNUSED(key);
    TEMPORARY_UNUSED(value);
    return 0;
}

void BPlusTreeIndex::flush()
{
    try {
        m_nodeCache.flush();
    } catch (std::exception& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorBptiFlushNodeCacheFailed,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, ex.what());
    }
}

std::uint64_t BPlusTreeIndex::find(const void* key, void* value, std::size_t count)
{
    // Check that buffer has some capacity,
    // otherwise there is no sense to continue
    if (count == 0) return 0;

    // Find a node which may contain the key
    auto node = findNode(key);
    assert(node->isLeaf());

    // Attempt to find a key
    const auto it = std::lower_bound(
            node->begin(), node->end(), key, BinarySearchNodeDataCompareFunction(m_keyCompare));
    if (it == node->end()) return 0;
    auto v = (*it).m_data;
    if (m_keyCompare(key, v)) return 0;

    std::memcpy(value, v + m_keySize, m_valueSize);
    return 1;
}

std::uint64_t BPlusTreeIndex::count(const void* key)
{
    // Find a node which may contain the key
    auto node = findNode(key);
    assert(node->isLeaf());

    // Check that node actually has a key
    return std::binary_search(node->begin(), node->end(), key,
                   BinarySearchNodeDataCompareFunction(m_keyCompare))
                   ? 1
                   : 0;
}

bool BPlusTreeIndex::getMinKey([[maybe_unused]] void* key)
{
    // TODO: Implement BPlusTreeIndex::getMinKey()
    return false;
}

bool BPlusTreeIndex::getMaxKey([[maybe_unused]] void* key)
{
    // TODO: Implement BPlusTreeIndex::getMinKey()
    return false;
}

bool BPlusTreeIndex::findFirstKey([[maybe_unused]] void* key)
{
    // TODO: Implement BPlusTreeIndex::findFirstKey()
    return false;
}

bool BPlusTreeIndex::findLastKey([[maybe_unused]] void* key)
{
    // TODO: Implement BPlusTreeIndex::findLastKey()
    return false;
}

bool BPlusTreeIndex::findPreviousKey(
        [[maybe_unused]] const void* key, [[maybe_unused]] void* prevKey)
{
    // TODO: Implement BPlusTreeIndex::findPreviousKey()
    return false;
}

bool BPlusTreeIndex::findNextKey([[maybe_unused]] const void* key, [[maybe_unused]] void* nextKey)
{
    // TODO: Implement BPlusTreeIndex::findNextKey()
    return false;
}

io::FilePtr BPlusTreeIndex::createIndexFile() const
{
    std::string tmpFilePath;

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
            tmpFilePath = m_indexFilePath + kTempFileExtension;
            file = m_table.getDatabase().createFile(
                    tmpFilePath, kBaseExtraOpenFlags, kDataFileCreationMode, m_dataFileSize);
        }
    } catch (std::system_error& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotCreateIndexFile, m_indexFilePath,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, ex.code().value(), std::strerror(ex.code().value()));
    }

    stdext::buffer<std::uint8_t> buffer(Node::kSize, 0);

    // Write index header
    IndexFileHeader indexFileHeader;
    indexFileHeader.serialize(buffer.data());
    if (file->write(buffer.data(), buffer.size(), 0) != buffer.size()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteIndexFile, m_indexFilePath,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, 0, buffer.size(), errorCode, std::strerror(errorCode));
    }

    // Write root node
    constexpr std::uint64_t kInitialRootNodeId = 1;
    LeafNodeHeader rootNodeHeader;
    rootNodeHeader.m_nodeId = kInitialRootNodeId;
    rootNodeHeader.m_nodeType = NodeType::kRootLeafNode;
    rootNodeHeader.m_childCount = 0;
    rootNodeHeader.m_prevNodeId = 0;
    rootNodeHeader.m_nextNodeId = 0;
    std::memset(buffer.data(), 0, IndexFileHeader::kSerializedSize);
    rootNodeHeader.serialize(buffer.data());
    auto nodeOffset = Node::getOffset(1);
    if (file->write(buffer.data(), buffer.size(), nodeOffset) != buffer.size()) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteIndexFile, m_indexFilePath,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, nodeOffset, buffer.size(), errorCode,
                std::strerror(errorCode));
    }

    // Write root node ID
    ::pbeEncodeUInt64(kInitialRootNodeId, buffer.data());
    nodeOffset = Node::getOffset(0);
    if (file->write(buffer.data(), sizeof(std::uint64_t), nodeOffset) != sizeof(std::uint64_t)) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotWriteIndexFile, m_indexFilePath,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, nodeOffset, sizeof(std::uint64_t), errorCode,
                std::strerror(errorCode));
    }

    if (tmpFilePath.empty()) {
        // Link to the filesystem
        const auto fdPath = "/proc/self/fd/" + std::to_string(file->getFD());
        if (::linkat(AT_FDCWD, fdPath.c_str(), AT_FDCWD, m_indexFilePath.c_str(), AT_SYMLINK_FOLLOW)
                < 0) {
            const int errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotLinkIndexFile, m_indexFilePath,
                    m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                    m_table.getId(), m_id, errorCode, std::strerror(errorCode));
        }
    } else {
        // Rename temporary file to the regular one
        if (::rename(tmpFilePath.c_str(), m_indexFilePath.c_str()) < 0) {
            const int errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotRenameIndexFile, tmpFilePath,
                    m_indexFilePath, m_table.getDatabaseName(), m_table.getName(), m_name,
                    m_table.getDatabaseUuid(), m_table.getId(), m_id, errorCode,
                    std::strerror(errorCode));
        }
    }

    return file;
}

io::FilePtr BPlusTreeIndex::openIndexFile() const
{
    io::FilePtr file;
    try {
        file = m_table.getDatabase().openFile(m_indexFilePath, O_DSYNC);
    } catch (std::system_error& ex) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotOpenIndexFile, m_indexFilePath,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, ex.code().value(), std::strerror(ex.code().value()));
    }
    return file;
}

std::size_t BPlusTreeIndex::calculateNodeCount() const
{
    // Validate index file size
    struct stat st;
    if (!m_file->stat(st)) {
        const auto indexFilePath = makeIndexFilePath(0);
        throwDatabaseError(IOManagerMessageId::kErrorCannotStatIndexFile, indexFilePath,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, m_file->getLastError(),
                std::strerror(m_file->getLastError()));
    }
    if (st.st_size % Node::kSize > 0 || st.st_size < static_cast<off_t>(2 * Node::kSize)) {
        throwDatabaseError(IOManagerMessageId::kErrorIndexFileCorrupted, m_table.getDatabaseName(),
                m_table.getName(), m_name, m_table.getDatabaseUuid(), m_table.getId(), m_id,
                "invalid file size");
    }

    // Determine number of nodes
    const auto nodeCount = (st.st_size / Node::kSize) - 1;
    //LOG_DEBUG << "Node count: " << m_nodeCount;
    if (static_cast<off_t>((nodeCount + 1) * Node::kSize) != st.st_size) {
        throwDatabaseError(IOManagerMessageId::kErrorIndexFileCorrupted, m_table.getDatabaseName(),
                m_table.getName(), m_name, m_table.getDatabaseUuid(), m_table.getId(), m_id,
                "invalid file size");
    }

    return nodeCount;
}

std::size_t BPlusTreeIndex::findRootNode()
{
    // Read root node ID
    std::uint8_t buffer[sizeof(uint64_t)];
    const auto readOffset = Node::getOffset(0);
    if (m_file->read(buffer, sizeof(std::uint64_t), readOffset) != sizeof(std::uint64_t)) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadIndexFile, m_indexFilePath,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, readOffset, sizeof(std::uint64_t), m_file->getLastError(),
                std::strerror(m_file->getLastError()));
    }
    std::uint64_t rootNodeId = 0;
    ::pbeDecodeUInt64(buffer, &rootNodeId);

#if 0
    // Fallback method - find root node by enumerating and testing all nodes sequentially
    std::uint64_t rootNodeId = 1;
    auto node = findNode(rootNodeId);
    while (node && node->m_header.m_common.m_nodeType != NodeType::kRootInternalNode
            && node->m_header.m_common.m_nodeType != NodeType::kRootLeafNode) {
        ++rootNodeId;
        node = findNode(rootNodeId);
    }
#endif

    // Load and validate root node
    auto node = findNode(rootNodeId);
    if (!node || !node->isRoot()) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotFindIndexRoot, m_table.getDatabaseName(),
                m_table.getName(), m_name, m_table.getDatabaseUuid(), m_table.getId(), m_id);
    }

    // Save root node ID
    return rootNodeId;
}

BPlusTreeIndex::NodePtr BPlusTreeIndex::findNode(const void* key)
{
    // Each node contains:
    // - header
    // - series of (key, value) pairs, where value:
    //     - in the internal node is 64-bit child node ID
    //     - in the leaf node is 64-bit data offset

    // Search algorithm as described in the https://en.wikipedia.org/wiki/B%2B_tree
    //
    // Function: search (k)
    // return tree_search (k, root);
    //
    // Function: tree_search (k, node)
    // if node is a leaf then
    //     return node;
    // switch k do
    // case k ≤ k_0
    //     return tree_search(k, p_0);
    // case k_i < k ≤ k_{i+1}
    //     return tree_search(k, p_{i+1});
    // case k_d < k
    //     return tree_search(k, p_{d});
    //

    auto currentNodeId = m_rootNodeId;
    auto currentNode = findNode(currentNodeId);

    while (currentNode && currentNode->m_header.m_common.m_nodeType != NodeType::kLeafNode
            && currentNode->m_header.m_common.m_nodeType != NodeType::kRootLeafNode) {
        std::uint64_t nextNodeId = 0;
        if (m_keyCompare(key, currentNode->m_data + InternalNodeHeader::kSerializedSize) <= 0) {
            // case k ≤ k_0
            ::pbeDecodeUInt64(currentNode->m_data + InternalNodeHeader::kSerializedSize + m_keySize,
                    &nextNodeId);
        } else {
            // There must be at least 2 child nodes in this case
            auto n = currentNode->m_header.m_common.m_childCount;
            if (n < 2) {
                throwDatabaseError(IOManagerMessageId::kErrorIndexNodeCorrupted,
                        m_table.getDatabaseName(), m_table.getName(), m_name, currentNodeId,
                        m_table.getDatabaseUuid(), m_table.getId(), m_id);
            }

            --n;
            std::uint32_t i = 1;
            for (; i < n - 1; ++i) {
                const auto currentEntryOffset =
                        InternalNodeHeader::kSerializedSize + i * m_internalKvPairSize;
                const auto nextEntryOffset = currentEntryOffset + m_internalKvPairSize;
                if (m_keyCompare(key, currentNode->m_data + currentEntryOffset) > 0
                        && m_keyCompare(key, currentNode->m_data + nextEntryOffset) <= 0) {
                    // case k_i < k ≤ k_{i+1}
                    ::pbeDecodeUInt64(
                            currentNode->m_data + nextEntryOffset + m_keySize, &nextNodeId);
                    break;
                }
            }

            if (i == n - 1) {
                const auto offset = InternalNodeHeader::kSerializedSize + n * m_internalKvPairSize;
                if (m_keyCompare(key, currentNode->m_data + offset) > 0) {
                    // case k_d < k
                    ::pbeDecodeUInt64(currentNode->m_data + offset + m_keySize, &nextNodeId);
                } else {
                    throwDatabaseError(IOManagerMessageId::kErrorIndexNodeCorrupted,
                            m_table.getDatabaseName(), m_table.getName(), m_name, currentNodeId,
                            m_table.getDatabaseUuid(), m_table.getId(), m_id);
                }
            }
        }

        currentNode = findNode(nextNodeId);
        currentNodeId = nextNodeId;
    }

    if (!currentNode) {
        throwDatabaseError(IOManagerMessageId::kErrorCannotFindIndexNode, m_table.getDatabaseName(),
                m_table.getName(), m_name, currentNodeId, m_table.getDatabaseUuid(),
                m_table.getId(), m_id);
    }

    return currentNode;
}

BPlusTreeIndex::NodePtr BPlusTreeIndex::findNode(io::File& file, std::uint64_t nodeId)
{
    auto cachedNode = m_nodeCache.get(nodeId);
    return cachedNode ? *cachedNode : readNode(file, nodeId);
}

BPlusTreeIndex::NodePtr BPlusTreeIndex::readNode(io::File& file, std::uint64_t nodeId)
{
    // Create new node object
    auto node = std::make_shared<Node>(*this, nodeId);

    // Read node data
    const auto nodeOffset = Node::getOffset(nodeId);
    if (file.read(node->m_data, Node::kSize, nodeOffset) != Node::kSize) {
        const int errorCode = errno;
        throwDatabaseError(IOManagerMessageId::kErrorCannotReadIndexFile, m_indexFilePath,
                m_table.getDatabaseName(), m_table.getName(), m_name, m_table.getDatabaseUuid(),
                m_table.getId(), m_id, nodeOffset, Node::kSize, errorCode,
                std::strerror(errorCode));
    }

    // Validate node type
    if (static_cast<int>(node->m_data[0]) >= static_cast<int>(NodeType::kMaxNodeType)) {
        throwDatabaseError(IOManagerMessageId::kErrorIndexNodeCorrupted, m_table.getDatabaseName(),
                m_table.getName(), m_name, nodeId, m_table.getDatabaseUuid(), m_table.getId(),
                m_id);
    }

    // Deserialize node header
    const auto nodeType = static_cast<NodeType>(node->m_data[0]);
    if (Node::isLeafNodeType(nodeType))
        node->m_header.m_leafNodeHeader.deserialize(node->m_data);
    else
        node->m_header.m_internalNodeHeader.deserialize(node->m_data);

    // Put node to cache
    m_nodeCache.emplace(nodeId, node);
    return node;
}

BPlusTreeIndex::NodePtr BPlusTreeIndex::makeNode(io::File& file, std::uint64_t nodeId)
{
    // TODO: Implement BPlusTreeIndex::makeNode()
    TEMPORARY_UNUSED(file);
    TEMPORARY_UNUSED(nodeId);
    return nullptr;
}

void BPlusTreeIndex::insertNewEntryToNonFullLeafNode(
        Node& node, std::uint32_t pos, const void* key, const void* value)
{
    if (pos > node.m_header.m_common.m_childCount)
        throw std::out_of_range("BPlusTreeIndex: new leaf node element index is out of range");

    const auto newEntry = node.m_data + LeafNodeHeader::kSerializedSize + pos * m_kvPairSize;
    const auto movedEntryCount = node.m_header.m_common.m_childCount - pos;
    if (movedEntryCount > 0)
        std::memmove(newEntry + m_kvPairSize, newEntry, movedEntryCount * m_kvPairSize);

    std::memcpy(newEntry, key, m_keySize);
    std::memcpy(newEntry + m_keySize, value, m_valueSize);
    ++node.m_header.m_common.m_childCount;
    node.m_modified = true;
}

void BPlusTreeIndex::insertNewEntryToFullLeafNode(
        Node& node, std::uint32_t pos, const void* key, const void* value)
{
    auto newNode = getNewNode();

    // TODO: Split a node and insert record to a new node
    TEMPORARY_UNUSED(node);
    TEMPORARY_UNUSED(pos);
    TEMPORARY_UNUSED(key);
    TEMPORARY_UNUSED(value);
}

///////////////////// class BPlusTreeIndex::IndexFileHeader ///////////////////////////////////////

std::uint8_t* BPlusTreeIndex::IndexFileHeader::serialize(std::uint8_t* buffer) const noexcept
{
    return IndexFileHeaderBase::serialize(buffer);
}

const std::uint8_t* BPlusTreeIndex::IndexFileHeader::deserialize(
        const std::uint8_t* buffer) noexcept
{
    return IndexFileHeaderBase::deserialize(buffer);
}

///////////////////// class BPlusTreeIndex::NodeCache /////////////////////////////////////////////

void BPlusTreeIndex::NodeCache::flush()
{
    bool hadErrors = false;
    for (const auto& e : map_internal()) {
        try {
            if (e.second.first->m_modified) {
                // TODO: Write modified block to disk
                e.second.first->m_modified = false;
            }
        } catch (...) {
            // ignore all exceptions
        }
    }
    if (hadErrors) {
        throw std::runtime_error("Error flushing BPTI node cache");
    }
}

bool BPlusTreeIndex::NodeCache::can_evict(
        [[maybe_unused]] const key_type& key, const mapped_type& value) const noexcept
{
    return !value->m_modified;
}

void BPlusTreeIndex::NodeCache::on_evict([[maybe_unused]] const key_type& key, mapped_type& value,
        [[maybe_unused]] bool clearingCache) const
{
    if (value->m_modified)
        throw std::runtime_error("BPlusTreeIndex: attempt to evict modified node from the cache");
}

bool BPlusTreeIndex::NodeCache::on_last_chance_cleanup()
{
    std::size_t savedCount = 0;
    for (const auto& e : map_internal()) {
        if (!e.second.first->m_modified) continue;
        // Save node to data file
        const auto nodeOffset = Node::getOffset(e.first);
        if (m_owner.m_file->write(e.second.first->m_data, Node::kSize, nodeOffset) != Node::kSize) {
            const int errorCode = errno;
            throwDatabaseError(IOManagerMessageId::kErrorCannotWriteIndexFile,
                    m_owner.getIndexFilePath(), m_owner.getTable().getDatabaseName(),
                    m_owner.m_table.getName(), m_owner.m_name, m_owner.getTable().getDatabaseUuid(),
                    m_owner.m_table.getId(), m_owner.m_id, nodeOffset, Node::kSize, errorCode,
                    std::strerror(errorCode));
        }
        e.second.first->m_modified = false;
        ++savedCount;
    }
    return savedCount > 0;
}

///////////////////// class BPlusTreeIndex::CommonNodeHeader////////////////////////////////////

std::uint8_t* BPlusTreeIndex::CommonNodeHeader::serialize(std::uint8_t* buffer) const noexcept
{
    // Node type must go first
    *buffer++ = static_cast<std::uint8_t>(m_nodeType);
    buffer = ::pbeEncodeUInt64(m_nodeId, buffer);
    buffer = ::pbeEncodeUInt32(m_childCount, buffer);
    return buffer;
}

const std::uint8_t* BPlusTreeIndex::CommonNodeHeader::deserialize(
        const std::uint8_t* buffer) noexcept
{
    m_nodeType = static_cast<NodeType>(*buffer++);
    buffer = ::pbeDecodeUInt64(buffer, &m_nodeId);
    buffer = ::pbeDecodeUInt32(buffer, &m_childCount);
    return buffer;
}

///////////////////// class BPlusTreeIndex::LeafNodeHeader /////////////////////////////////////////

std::uint8_t* BPlusTreeIndex::LeafNodeHeader::serialize(std::uint8_t* buffer) const noexcept
{
    buffer = CommonNodeHeader::serialize(buffer);
    buffer = ::pbeEncodeUInt64(m_prevNodeId, buffer);
    buffer = ::pbeEncodeUInt64(m_nextNodeId, buffer);
    return buffer;
}

const std::uint8_t* BPlusTreeIndex::LeafNodeHeader::deserialize(const std::uint8_t* buffer) noexcept
{
    buffer = CommonNodeHeader::deserialize(buffer);
    buffer = ::pbeDecodeUInt64(buffer, &m_prevNodeId);
    buffer = ::pbeDecodeUInt64(buffer, &m_nextNodeId);
    return buffer;
}

}  // namespace siodb::iomgr::dbengine
