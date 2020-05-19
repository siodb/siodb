// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../Index.h"
#include "../IndexFileHeaderBase.h"

// Common project headers
#include <siodb/common/utils/FileDescriptorGuard.h>
#include <siodb/common/utils/UnorderedLruCache.h>

// System headers
#include <sys/types.h>

namespace siodb::iomgr::dbengine {

/** B+ tree index */
class BPlusTreeIndex : public Index {
public:
    /**
     * Initializes object of class BPlusTreeIndex for a new index.
     * @param table A table to which this index belongs.
     * @param name Index name.
     * @param keyTraits Key traits.
     * @param valueSize Value size.
     * @param keyCompare Key comparison function.
     * @param unique Index uniqueness flag.
     * @param columns Indexed column list.
     * @param dataFileSize Data file size.
     */
    BPlusTreeIndex(Table& table, const std::string& name, const IndexKeyTraits& keyTraits,
            std::size_t valueSize, KeyCompareFunction keyCompare, bool unique,
            const IndexColumnSpecificationList& columns, std::uint32_t dataFileSize);

    /**
     * Initializes object of class BPlusTreeIndex for an existing index.
     * @param table A table to which this index belongs.
     * @param indexRecord Index record.
     * @param keyTraits Key traits.
     * @param valueSize Value size.
     * @param keyCompare Key comparison function.
     */
    BPlusTreeIndex(Table& table, const IndexRecord& indexRecord, const IndexKeyTraits& keyTraits,
            std::size_t valueSize, KeyCompareFunction keyCompare);

    /**
     * Returns index file path.
     * @return Index file path.
     */
    const auto& getIndexFilePath() const noexcept
    {
        return m_indexFilePath;
    }

    /**
     * Returns data file size if applicable.
     * @return Data file size or zero if not applicable.
     */
    std::uint32_t getDataFileSize() const noexcept override;

    /**
     * Inserts data into the index.
     * @param key A key buffer.
     * @param value A value buffer.
     * @param replaceExisting flag that indicated if existing value to be replaced.
     * @return true if key was a new one, false if key already existed.
     */
    bool insert(const void* key, const void* value, bool replaceExisting = false) override;

    /**
     * Deletes data the index.
     * @param key A key buffer.
     * @return true Number of deleted entries.
     */
    std::uint64_t erase(const void* key) override;

    /**
     * Updates data in the index.
     * @param key A key buffer.
     * @param value A value buffer.
     * @param replaceExisting flag that indicated if existing value to be replaced.
     * @return true if key found, false if key not found.
     */
    std::uint64_t update(const void* key, const void* value) override;

    /**
     * Marks existing key as deleted and updates value.
     * @param key A key buffer.
     * @param value A value buffer.
     * @return true if key existed and changed, false otherwise.
     */
    bool markAsDeleted(const void* key, const void* value) override;

    /** Writes cached changes to disk. */
    void flush() override;

    /**
     * Gets data from the index.
     * @param key A key buffer.
     * @param value An output buffer.
     * @param count Number of values that can fit in the outout buffer.
     * @return Number of values actually copied.
     */
    std::uint64_t getValue(const void* key, void* value, std::size_t count) override;

    /**
     * Counts how much values available for this key.
     * @param key A key buffer.
     * @return Number of values copied available.
     */
    std::uint64_t count(const void* key) override;

    /**
     * Returns minimum key in the index.
     * @param key Buffer for storing key.
     * @return true if minimum key exists, false if index is empty.
     */
    bool getMinKey(void* key) override;

    /**
     * Returns maximum key in the index.
     * @param key Buffer for storing key.
     * @return true if minimum key exists, false if index is empty.
     */
    bool getMaxKey(void* key) override;

    /**
     * Returns first key in the index. Always reads index storage.
     * @param key Buffer for storing key.
     * @return true if minimum key exists, false if index is empty.
     */
    bool getFirstKey(void* key) override;

    /**
     * Returns last key in the index storage. Always reads index storage.
     * @param key Buffer for storing key.
     * @return true if minimum key exists, false if index is empty.
     */
    bool getLastKey(void* key) override;

    /**
     * Returns previous key in the index.
     * @param key Current key.
     * @param prevKey Buffer for storing previous key.
     * @return true if previous key obtained, false otherwise.
     */
    bool getPrevKey(const void* key, void* prevKey) override;

    /**
     * Returns next key in the index.
     * @param key Current key.
     * @param nextKey Buffer for storing next key.
     * @return true if next key obtained, false otherwise.
     */
    bool getNextKey(const void* key, void* nextKey) override;

private:
    /** Index file header */
    struct IndexFileHeader : public IndexFileHeaderBase {
        /** Initialized object of class Header */
        IndexFileHeader()
            : IndexFileHeaderBase(IndexType::kBPlusTreeIndex)
        {
        }

        /**
         * Serializes this object to buffer.
         * @return Address after a last written byte.
         */
        std::uint8_t* serialize(std::uint8_t* buffer) const noexcept;

        /**
         * De-serializes this object from buffer.
         * @param buffer A buffer.
         * @return Address after a last read byte.
         */
        const std::uint8_t* deserialize(const std::uint8_t* buffer) noexcept;

        /** Serialized size */
        static constexpr std::size_t kSerializedSize = IndexFileHeaderBase::kSerializedSize;
    };

    /** Node type */
    enum class NodeType {
        kInternalNode,
        kLeafNode,
        kRootInternalNode,
        kRootLeafNode,
        kMaxNodeType
    };

    /** Common part of all node headers */
    struct CommonNodeHeader {
        /** Node ID */
        std::uint64_t m_nodeId;

        /** Node type */
        NodeType m_nodeType;

        /** Number of child nodes in this root node */
        std::uint32_t m_childCount;

    protected:
        /**
         * Serializes this object to buffer.
         * @return Address after a last written byte.
         */
        std::uint8_t* serialize(std::uint8_t* buffer) const noexcept;

        /**
         * De-serializes this object from buffer.
         * @param buffer A buffer.
         * @return Address after a last read byte.
         */
        const std::uint8_t* deserialize(const std::uint8_t* buffer) noexcept;

        /** Serialized size */
        static constexpr std::size_t kSerializedSize = sizeof(m_nodeId) + 1 + sizeof(m_childCount);
    };

    /** Internal node header */
    struct InternalNodeHeader : public CommonNodeHeader {
        /**
         * Serializes this object to buffer.
         * @return Address after a last written byte.
         */
        std::uint8_t* serialize(std::uint8_t* buffer) const noexcept
        {
            return CommonNodeHeader::serialize(buffer);
        }

        /**
         * De-serializes this object from buffer.
         * @param buffer A buffer.
         * @return Address after a last read byte.
         */
        const std::uint8_t* deserialize(const std::uint8_t* buffer) noexcept
        {
            return CommonNodeHeader::deserialize(buffer);
        }

        /** Serialized size of this header */
        static constexpr std::size_t kSerializedSize = CommonNodeHeader::kSerializedSize;
    };

    /** Leaf node header */
    struct LeafNodeHeader : public CommonNodeHeader {
        /**
         * Serializes this object to buffer.
         * @return Address after a last written byte.
         */
        std::uint8_t* serialize(std::uint8_t* buffer) const noexcept;

        /**
         * De-serializes this object from buffer.
         * @param buffer A buffer.
         * @return Address after a last read byte.
         */
        const std::uint8_t* deserialize(const std::uint8_t* buffer) noexcept;

        /** Previous node ID */
        std::uint64_t m_prevNodeId;

        /** Next node ID */
        std::uint64_t m_nextNodeId;

        /** Serialized size of this header */
        static constexpr std::size_t kSerializedSize =
                CommonNodeHeader::kSerializedSize + sizeof(m_prevNodeId) + sizeof(m_nextNodeId);
    };

    class NodeKeyValuePairIterator;

    /** Tree node */
    struct Node {
        /**
         * Initializes object of class Node.
         * @param owner Node owner.
         * @param nodeId Node ID.
         */
        Node(const BPlusTreeIndex& owner, std::uint64_t nodeId) noexcept
            : m_owner(owner)
            , m_modified(false)
        {
            m_header.m_common.m_nodeId = nodeId;
        }

        /**
         * Calculates node offset in the index file.
         * @param nodeId Node ID.
         * @return Node offset.
         */
        static constexpr off_t getOffset(std::uint64_t nodeId) noexcept
        {
            return nodeId * Node::kSize;
        }

        /**
         * Returns indication that this node is leaf node.
         * @return true, if this is leaf node, false otherwise.
         */
        bool isLeaf() const noexcept
        {
            return isLeafNodeType(m_header.m_common.m_nodeType);
        }

        /**
         * Returns indication that given node type corresponds to the leaf node.
         * @param nodeType A node type.
         * @return true, if this is leaf node, false otherwise.
         */
        static constexpr bool isLeafNodeType(NodeType nodeType) noexcept
        {
            return nodeType == NodeType::kLeafNode || nodeType == NodeType::kRootLeafNode;
        }

        /**
         * Returns indication that this node is root node.
         * @return true, if this is root node, false otherwise.
         */
        bool isRoot() const noexcept
        {
            return isRootNodeType(m_header.m_common.m_nodeType);
        }

        /**
         * Returns indication that given node type corresponds to the root node.
         * @param nodeType A node type.
         * @return true, if this is root node, false otherwise.
         */
        static constexpr bool isRootNodeType(NodeType nodeType) noexcept
        {
            return nodeType == NodeType::kRootInternalNode || nodeType == NodeType::kRootLeafNode;
        }

        /**
         * Returns begin iterator.
         * @return Begin iterator.
         */
        NodeKeyValuePairIterator begin() noexcept;

        /**
         * Returns end iterator.
         * @return end iterator.
         */
        NodeKeyValuePairIterator end() noexcept;

        /** B+ tree node size */
        static constexpr std::size_t kSize = 8 * 1024;

        /** Owner object */
        const BPlusTreeIndex& m_owner;

        /** Node header */
        union {
            CommonNodeHeader m_common;
            InternalNodeHeader m_internalNodeHeader;
            LeafNodeHeader m_leafNodeHeader;
        } m_header;

        /** Node data */
        std::uint8_t m_data[kSize];

        /** Modification flag */
        bool m_modified;

        /** Node type offset */
        static constexpr std::size_t kNodeTypeOffset = 0;
    };

    /** Node shared pointer shortcut type */
    using NodePtr = std::shared_ptr<Node>;

    /** Regular cache of recently used nodes */
    class NodeCache final : public utils::unordered_lru_cache<std::uint64_t, NodePtr> {
    private:
        using Base = utils::unordered_lru_cache<std::uint64_t, NodePtr>;

    public:
        /**
         * Initializes object of class NodeCache.
         * @param capacity Cache capacity (maximum allowed size).
         * @param fd Index file descriptor.
         */
        explicit NodeCache(const BPlusTreeIndex& owner, std::size_t capacity) noexcept
            : Base(capacity)
            , m_owner(owner)
        {
        }

        /** De-initializes object of class NodeCache. */
        ~NodeCache();

        /**
         * Writes all pending changes to disk.
         */
        void flush();

    protected:
        /**
         * Returns indication if item can be evicted. By default all items can be evicted.
         * @param key A key.
         * @param value A value.
         * @return true if item can be evicted, false otherwise.
         */
        bool can_evict(const key_type& key, const mapped_type& value) const noexcept override;

        /**
         * Called before item gets evicted from the cache.
         * @param key A key.
         * @param value A value.
         * @param clearingCache Indication that cache is being completely cleared.
         */
        void on_evict(const key_type& key, mapped_type& value, bool clearingCache) const override;

        /**
         * Called if evict() could not find any discardable element
         * to give last chance for making some additional cleanup.
         * @return true if some changes were made so that next eviction attempt
         *         must be made, false otherwise.
         */
        bool on_last_chance_cleanup() override;

    private:
        /** Owner object */
        const BPlusTreeIndex& m_owner;
    };

    /**
     * Lightweight object type returned by iterator dereference operator.
     * Contains only actual data address.
     */
    struct NodeDataValue {
        /** Address of a key-value pair in the node */
        std::uint8_t* m_data;
    };

    /** Iterator for key-value pairs in the node */
    class NodeKeyValuePairIterator {
    public:
        typedef NodeKeyValuePairIterator self_type;
        typedef NodeDataValue value_type;
        typedef NodeDataValue& reference;
        typedef NodeDataValue* pointer;
        typedef std::forward_iterator_tag iterator_category;
        typedef std::ptrdiff_t difference_type;

    public:
        /**
         * Initializes object of class NodeKeyValuePairIterator.
         * @param node Node object.
         * @param entryIndex Initial entry index.
         */
        explicit NodeKeyValuePairIterator(Node& node, std::size_t entryIndex = 0) noexcept
            : m_node(&node)
            , m_entryIndex(entryIndex)
        {
        }

        /**
         * Equality operator.
         * @param other Other object.
         * @return true if this object is equal to other, false otherwise.
         */
        bool operator==(const NodeKeyValuePairIterator& other) const noexcept
        {
            return m_node == other.m_node && m_entryIndex == other.m_entryIndex;
        }

        /**
         * Non-equality operator.
         * @param other Other object.
         * @return true if this object is not equal to other, false otherwise.
         */
        bool operator!=(const NodeKeyValuePairIterator& other) const noexcept
        {
            return m_node != other.m_node || m_entryIndex != other.m_entryIndex;
        }

        /**
         * Prefix increment operator.
         * @return This object.
         * @throw std::out_of_range if move out of iteration range happened.
         */
        NodeKeyValuePairIterator& operator++()
        {
            if (m_entryIndex >= m_node->m_header.m_common.m_childCount) {
                throw std::out_of_range("NodeKeyValuePairIterator increment is out of range");
            }
            ++m_entryIndex;
            return *this;
        }

        /**
         * Postfix increment operator.
         * @return Iterator object, pointing to a current element.
         * @throw std::out_of_range if move out of iteration range happened.
         */
        NodeKeyValuePairIterator operator++(int)
        {
            if (m_entryIndex >= m_node->m_header.m_common.m_childCount) {
                throw std::out_of_range("NodeKeyValuePairIterator increment is out of range");
            }
            return NodeKeyValuePairIterator(*m_node, m_entryIndex++);
        }

        /**
         * Prefix decrement operator.
         * @return This object.
         * @throw std::out_of_range if move out of iteration range happened.
         */
        NodeKeyValuePairIterator& operator--()
        {
            if (m_entryIndex == 0) {
                throw std::out_of_range("NodeKeyValuePairIterator decrement is out of range");
            }
            --m_entryIndex;
            return *this;
        }

        /**
         * Postfix decrement operator.
         * @return Iterator object, pointing to a current element.
         * @throw std::out_of_range if move out of iteration range happened.
         */
        NodeKeyValuePairIterator operator--(int)
        {
            if (m_entryIndex == 0) {
                throw std::out_of_range("NodeKeyValuePairIterator decrement is out of range");
            }
            return NodeKeyValuePairIterator(*m_node, m_entryIndex--);
        }

        /**
         * Addition to this object operator.
         * @param n Number of steps forward.
         * @return This object.
         * @throw std::out_of_range if move out of iteration range happened.
         */
        NodeKeyValuePairIterator& operator+=(difference_type n)
        {
            if (n < 0) return *this -= (-n);
            if (m_node->m_header.m_common.m_childCount - m_entryIndex
                    < static_cast<std::size_t>(n)) {
                throw std::out_of_range("NodeKeyValuePairIterator step forward is out of range");
            }
            m_entryIndex += n;
            return *this;
        }

        /**
         * Subtraction from this object operator.
         * @param n Number of steps back.
         * @return This object.
         * @throw std::out_of_range if move out of iteration range happened.
         */
        NodeKeyValuePairIterator& operator-=(difference_type n)
        {
            if (n < 0) return *this += (-n);
            if (m_entryIndex < static_cast<std::size_t>(n)) {
                throw std::out_of_range("NodeKeyValuePairIterator step back is out of range");
            }
            m_entryIndex -= n;
            return *this;
        }

        /**
         * Addition operator.
         * @param n Number of steps forward.
         * @return Iterator object pointing N steps forward.
         * @throw std::out_of_range if move out of iteration range happened.
         */
        NodeKeyValuePairIterator operator+(difference_type n) const
        {
            if (n < 0) return *this - (-n);
            if (m_node->m_header.m_common.m_childCount - m_entryIndex
                    < static_cast<std::size_t>(n)) {
                throw std::out_of_range("NodeKeyValuePairIterator step forward is out of range");
            }
            return NodeKeyValuePairIterator(*m_node, m_entryIndex + n);
        }

        /**
         * Addition operator.
         * @param n Number of steps forward.
         * @return Iterator object pointing N steps forward.
         * @throw std::out_of_range if move out of iteration range happened.
         */
        NodeKeyValuePairIterator operator-(difference_type n) const
        {
            if (n < 0) return *this + (-n);
            if (m_entryIndex < static_cast<std::size_t>(n)) {
                throw std::out_of_range("NodeKeyValuePairIterator step back is out of range");
            }
            return NodeKeyValuePairIterator(*m_node, m_entryIndex - n);
        }

        /**
         * Dereference operator.
         * @return Lightweight object, which contains pointer to actual data inside.
         */
        NodeDataValue operator*() const noexcept
        {
            return NodeDataValue {
                    m_node->m_data + m_entryIndex * m_node->m_owner.m_internalKvPairSize};
        }

    private:
        /** Underlying node object */
        Node* m_node;

        /** Current entry index */
        std::size_t m_entryIndex;
    };

    /** Comparison function for the binary search */
    struct BinarySearchNodeDataCompareFunction {
        /** Initializes object of class BinarySearchNodeDataCompareFunction */
        explicit BinarySearchNodeDataCompareFunction(KeyCompareFunction cmp) noexcept
            : m_cmp(cmp)
        {
        }

        /**
         * Compares data from node and given key.
         * @return true, if value is "less" than key, false otherwise.
         */
        bool operator()(const NodeDataValue& value, const void* key) const
        {
            return m_cmp(value.m_data, key);
        }

        /**
         * Compares data from node and given key.
         * @return true, if key is "less" than value, false otherwise.
         */
        bool operator()(const void* key, const NodeDataValue& value) const
        {
            return m_cmp(key, value.m_data);
        }

        /** Key comparison function */
        const KeyCompareFunction m_cmp;
    };

private:
    /**
     * Creates and initializes new index data file.
     * @return Index file descriptor.
     */
    io::FilePtr createIndexFile() const;

    /**
     * Opens existing index data file.
     * @return Index file descriptor.
     */
    io::FilePtr openIndexFile() const;

    /**
     * Calculates number of nodes in the index file.
     * @return Number of nodes in the index file.
     */
    std::size_t calculateNodeCount() const;

    /**
     * Finds root node.
     * @return Root node ID.
     */
    std::size_t findRootNode();

    /**
     * Finds node that contains or must contain given key.
     * @param key A key.
     * @return Node object.
     */
    NodePtr findNode(const void* key);

    /**
     * Gets existing node object from the standard data file.
     * @param nodeId Node ID.
     * @return Node object.
     */
    NodePtr getNode(std::uint64_t nodeId)
    {
        return getNode(*m_file, nodeId);
    }

    /**
     * Gets existing node object from a given file.
     * @param file File object.
     * @param nodeId Node ID.
     * @return Node object.
     */
    NodePtr getNode(io::File& file, std::uint64_t nodeId);

    /**
     * Reads existing node object from the standard data file.
     * @param nodeId Node ID.
     * @return Node object.
     */
    NodePtr readNode(std::uint64_t nodeId)
    {
        return readNode(*m_file, nodeId);
    }

    /**
     * Reads existing node object from a given file.
     * @param file File object.
     * @param nodeId Node ID.
     * @return Node object.
     */
    NodePtr readNode(io::File& file, std::uint64_t nodeId);

    /**
     * Makes new physical node in the standard data file and corresponding node object.
     * @param nodeId Node ID.
     * @return Node object.
     */
    NodePtr makeNode(std::uint64_t nodeId)
    {
        return makeNode(*m_file, nodeId);
    }

    /**
     * Makes new physical node in a given data file and corresponding node object.
     * @param file File object.
     * @param nodeId Node ID.
     * @return Node object.
     */
    NodePtr makeNode(io::File& file, std::uint64_t nodeId);

    /**
     * Makes new physical node in the standard data file or obtains first available free node.
     * @return New node object.
     */
    NodePtr getNewNode()
    {
        return getNewNode(*m_file);
    }

    /**
     * Makes new physical node in a given data file or obtains first available free node.
     * @param file File object.
     * @return New node object.
     */
    NodePtr getNewNode(io::File& file)
    {
        return (m_nextFreeNodeId > m_nodeCount) ? makeNode(file, m_nextFreeNodeId++)
                                                : getNode(file, m_nextFreeNodeId);
    }

    /**
     * Inserts new key-value pair into the non-full leaf node.
     * @param node Node object.
     * @param pos Position of the new record.
     * @param key A key.
     * @param value A value.
     */
    void insertNewEntryToNonFullLeafNode(
            Node& node, std::uint32_t pos, const void* key, const void* value);

    /**
     * Inserts new key-value pair into the full leaf node.
     * @param node Node object.
     * @param pos Position of the new record.
     * @param key A key.
     * @param value A value.
     */
    void insertNewEntryToFullLeafNode(
            Node& node, std::uint32_t pos, const void* key, const void* value);

private:
    /** Data file size */
    const std::uint32_t m_dataFileSize;

    /** Internal key-value pair size in the node */
    const std::size_t m_internalKvPairSize;

    /** Maximum number of entries in the node */
    const std::size_t m_branchingFactor;

    /** Node split threshold */
    const std::size_t m_splitThreshold;

    /** Index file name */
    const std::string m_indexFilePath;

    /** Index file descriptor */
    io::FilePtr m_file;

    /** Total number of nodes */
    std::uint64_t m_nodeCount;

    /** Root node ID */
    std::uint64_t m_rootNodeId;

    /** Next free node ID */
    std::uint64_t m_nextFreeNodeId;

    /** Node cache */
    NodeCache m_nodeCache;

    /** Modified nodes (used during modification operations) */
    std::unordered_set<Node*> m_modifiedNodes;

    /** Node cache capacity */
    static constexpr std::size_t kNodeCacheCapacity = 16;
};

inline BPlusTreeIndex::NodeKeyValuePairIterator BPlusTreeIndex::Node::begin() noexcept
{
    return NodeKeyValuePairIterator(*this);
}

inline BPlusTreeIndex::NodeKeyValuePairIterator BPlusTreeIndex::Node::end() noexcept
{
    return NodeKeyValuePairIterator(*this, m_header.m_common.m_childCount);
}

}  // namespace siodb::iomgr::dbengine
