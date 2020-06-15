// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "FileCache.h"
#include "NodeCache.h"
#include "../Index.h"
#include "../IndexFileHeaderBase.h"

// Common project headers
#include <siodb/common/utils/FdGuard.h>

// STL headers
#include <atomic>
#include <set>

namespace siodb::iomgr::dbengine {

/**
 * Unique linear index stores value at index file offset derived from key.
 * Key can be only integer value. This index can stoere only single value
 * per key, therefore it is always unique.
 */
class UniqueLinearIndex : public Index {
protected:
    /**
     * Initializes object of class UniqueLinearIndex.
     * @param table A table to which this index belongs.
     * @param type Index type.
     * @param name Index name.
     * @param keyTraits Key traits.
     * @param valueSize Value size.
     * @param keyCompare Key comparison function.
     * @param columnSpec Indexed column specification.
     * @param dataFileSize Data file size.
     * @param description Index description.
     */
    UniqueLinearIndex(Table& table, IndexType type, std::string&& name,
            const IndexKeyTraits& keyTraits, std::size_t valueSize, KeyCompareFunction keyCompare,
            const IndexColumnSpecification& column, std::uint32_t dataFileSize,
            std::optional<std::string>&& description);

    /**
     * Initializes object of class Index for an existing index.
     * @param table A table to which this index belongs.
     * @param indexRecord Index record.
     * @param keyTraits Key traits.
     * @param valueSize Value size.
     * @param keyCompare Key comparison function.
     */
    UniqueLinearIndex(Table& table, const IndexRecord& indexRecord, const IndexKeyTraits& keyTraits,
            std::size_t valueSize, KeyCompareFunction keyCompare);

public:
    /**
     * Returns number of nodes per file.
     * @return Number of nodes per file.
     */
    auto getNumberOfNodesPerFile() const noexcept
    {
        return m_numberOfNodesPerFile;
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
     * @return true if key was new one, false if key already existed.
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
     * @return Number of updated values.
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
    std::uint64_t findValue(const void* key, void* value, std::size_t count) override;

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
    bool findFirstKey(void* key) override;

    /**
     * Returns last key in the index storage. Always reads index storage.
     * @param key Buffer for storing key.
     * @return true if minimum key exists, false if index is empty.
     */
    bool findLastKey(void* key) override;

    /**
     * Returns previous key in the index.
     * @param key Current key.
     * @param prevKey Buffer for storing previous key.
     * @return true if previous key obtained, false otherwise.
     */
    bool findPreviousKey(const void* key, void* prevKey) override;

    /**
     * Returns next key in the index.
     * @param key Current key.
     * @param nextKey Buffer for storing next key.
     * @return true if next key obtained, false otherwise.
     */
    bool findNextKey(const void* key, void* nextKey) override;

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

private:
    /** Value state */
    enum ValueState {
        kValueStateFree = 0,
        kValueStateExists = 1,
        kValueStateDeleted = 2,
    };

private:
    /**
     * Creates and initializes new index data file.
     * @param fileId File ID.
     * @return File object.
     */
    io::FilePtr createIndexFile(std::uint64_t fileId) const;

    /**
     * Opens existing index data file.
     * @param fileId File ID.
     * @return File object.
     */
    io::FilePtr openIndexFile(std::uint64_t fileId) const;

    /**
     * Returns node with given ID.
     * @param nodeId A node Id.
     * @return Node object if node found.
     * @throw DatabaseError if node not found.
     */
    uli::NodePtr findNodeChecked(std::uint64_t nodeId);

    /**
     * Returns node with given ID.
     * @param nodeId A node Id.
     * @return Node object or nullptr if not found.
     */
    uli::NodePtr findNode(std::uint64_t nodeId);

    /**
     * Makes new physical node and corresponding node object.
     * @param nodeId Node ID.
     * @return Node object.
     */
    uli::NodePtr makeNode(std::uint64_t nodeId);

    /**
     * Encodes 8-bit signed integer for indexing.
     * @param n A number.
     * @return Encoded number.
     */
    static std::uint64_t encodeSignedInt(std::int8_t n) noexcept
    {
        return static_cast<std::uint64_t>(n + 0x80);
    }

    /**
     * Encodes 16-bit signed integer for indexing.
     * @param n A number.
     * @return Encoded number.
     */
    static std::uint64_t encodeSignedInt(std::int16_t n) noexcept
    {
        return static_cast<std::uint64_t>(n + 0x8000);
    }

    /**
     * Encodes 32-bit signed integer for indexing.
     * @param n A number.
     * @return Encoded number.
     */
    static std::uint64_t encodeSignedInt(std::int32_t n) noexcept
    {
        return static_cast<std::uint64_t>(n + 0x80000000);
    }

    /**
     * Encodes 64-bit signed integer for indexing.
     * @param n A number.
     * @return Encoded number.
     */
    static std::uint64_t encodeSignedInt(std::int64_t n) noexcept
    {
        return static_cast<std::uint64_t>(n + 0x8000000000000000LL);
    }

    /**
     * Decodes 8-bit signed integer from index.
     * @param n Encoded number.
     * @return Decoded number.
     */
    static std::int8_t decodeSignedInt8(std::uint64_t n) noexcept
    {
        return static_cast<std::int8_t>(n) - 0x80;
    }

    /**
     * Decodes 16-bit signed integer from index.
     * @param n Encoded number.
     * @return Decoded number.
     */
    static std::int16_t decodeSignedInt16(std::uint64_t n) noexcept
    {
        return static_cast<std::int16_t>(n) - 0x8000;
    }

    /**
     * Decodes 32-bit signed integer from index.
     * @param n Encoded number.
     * @return Decoded number.
     */
    static std::int32_t decodeSignedInt32(std::uint64_t n) noexcept
    {
        return static_cast<std::int32_t>(n) - 0x80000000;
    }

    /**
     * Decodes 64-bit signed integer from index.
     * @param n Encoded number.
     * @return Decoded number.
     */
    static std::int64_t decodeSignedInt64(std::uint64_t n) noexcept
    {
        return static_cast<std::int64_t>(n) - 0x8000000000000000LL;
    }

    /**
     * Decodes key into a number that can be used for indexing.
     * @param key A key.
     * @return Unique number derived from key that can be used for indexing.
     */
    std::uint64_t decodeKey(const void* key) const noexcept;

    /**
     * Encodes key from a number that is used for indexing.
     * @param numericKey Numeric value of key.
     * @param key Encoded key output buffer.
     */
    void encodeKey(std::uint64_t numericKey, void* key) const noexcept;

    /**
     * Returns node number which should contain the key.
     * @param key A key.
     * @return A node number which should contain the key.
     */
    std::uint64_t getNodeIdForKey(std::uint64_t key) const noexcept
    {
        return (key / m_numberOfRecordsPerNode) + 1;
    }

    /**
     * Returns file ID which should contain the node.
     * @param nodeId Node ID.
     * @return A file ID which should contain the key.
     */
    std::uint64_t getFileIdForNode(std::uint64_t nodeId) const noexcept
    {
        return ((nodeId - 1) / m_numberOfNodesPerFile) + 1;
    }

    /**
     * Validates key size.
     * @return Key size if valid, otherwise throws.
     */
    std::size_t validateKeySize() const;

    /**
     * Validates key type.
     * @param keyTraits Index key traits.
     * @return true if key is signed integer, false if key is unsigned integer.
     *         Throws exception if key type is other than integer.
     */
    static bool validateKeyType(const IndexKeyTraits& keyTraits);

    /**
     * Finds leading (minimal) key in the index.
     * @return Minimal key in the index, if no elements - maximal possible key.
     */
    BinaryValue findLeadingKey();

    /**
     * Find leading (minimal) key in the index.
     * @param key Key buffer.
     * @return true if leading key found, false is index is empty.
     */
    bool findLeadingKey(void* key);

    /**
     * Finds trailing (maximum) key in the index.
     * @return Maximum key in the index, if no elements - maximal possible key.
     */
    BinaryValue findTrailingKey();

    /**
     * Finds trailing (maximum) key in the index.
     * @param key Key buffer.
     * @return true if leading key found, false is index is empty.
     */
    bool findTrailingKey(void* key);

    /**
     * Gets key before (less) in the index.
     * @param key A key.
     * @param keyBefore Buffer for a key before.
     * @return true if key before exists, false if not.
     */
    bool findKeyBefore(const void* key, void* keyBefore);

    /**
     * Gets key after (less) in the index.
     * @param key A key.
     * @param keyAfter Buffer for a key after.
     * @return true if key before exists, false if not.
     */
    bool findKeyAfter(const void* key, void* keyAfter);

    /**
     * Updates min and max keys after erasing/deletion.
     * @param key A deleteable key.
     */
    void updateMinMaxKeysAfterRemoval(const void* key);

    /**
     * Scans index data directory for data files and returns list of data file IDs.
     * @return List of file IDs.
     */
    std::set<std::uint64_t> scanFiles() const;

    /**
     * Returns minimum available node ID.
     * @return Minimum available node ID.
     */
    std::uint64_t getMinAvailableNodeId() const noexcept
    {
        return SIODB_UNLIKELY(m_fileIds.empty())
                       ? 0
                       : m_numberOfNodesPerFile * (*m_fileIds.begin() - 1) + 1;
    }

    /**
     * Returns maximum available node ID.
     * @return Maximum available node ID.
     */
    std::uint64_t getMaxAvailableNodeId() const noexcept
    {
        return SIODB_UNLIKELY(m_fileIds.empty()) ? 0
                                                 : m_numberOfNodesPerFile * (*m_fileIds.rbegin());
    }

    /**
     * Computes maximum possible node ID for this index.
     * @return Maximum possible node ID.
     */
    std::uint64_t computeMaxPossibleNodeId() const noexcept;

private:
    /** Validated key size */
    const std::uint32_t m_dataFileSize;

    /** Validated key size */
    const std::size_t m_validatedKeySize;

    /** Indicates that key is signed */
    const bool m_isSignedKey;

    /** Indicates descending sort direction */
    const bool m_sortDescending;

    /** Value record size */
    const std::size_t m_recordSize;

    /** Number of records per node */
    const std::size_t m_numberOfRecordsPerNode;

    /** Number of nodes per file */
    const std::size_t m_numberOfNodesPerFile;

    /** Number of records per file */
    const std::size_t m_numberOfRecordsPerFile;

    /** Minimum possible key */
    const BinaryValue m_minPossibleKey;

    /** Maximum possible key */
    const BinaryValue m_maxPossibleKey;

    /** Maximum possible node ID */
    const std::uint64_t m_maxPossibleNodeId;

    /** Sorted list of file IDs */
    std::set<std::uint64_t> m_fileIds;

    /** File cache */
    uli::FileCache m_fileCache;

    /** Actual minimum key */
    BinaryValue m_minKey;

    /** Actual maximum key */
    BinaryValue m_maxKey;

    /** File cache capacity */
    static constexpr std::size_t kFileCacheCapacity = 20;
};

}  // namespace siodb::iomgr::dbengine
