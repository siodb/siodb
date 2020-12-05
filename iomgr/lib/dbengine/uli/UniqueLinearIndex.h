// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "FileCache.h"
#include "../Index.h"
#include "../IndexFileHeaderBase.h"

// Common project headers
#include <siodb/common/utils/FDGuard.h>

// STL headers
#include <set>

namespace siodb::iomgr::dbengine {

/**
 * Unique linear index stores value at index file offset derived from key.
 * Key can be only integer value. This index can stoere only single value
 * per key, therefore it is always unique.
 */
class UniqueLinearIndex : public Index {
public:
    /** Data file header size */
    static constexpr std::uint32_t kIndexFileHeaderSize = 1024;

    /** Minimum data size in the file */
    static constexpr std::uint32_t kMinDataSizePerFile = 4096;

    /** Minimum data file size */
    static constexpr std::uint32_t kMinDataFileSize = kIndexFileHeaderSize + kMinDataSizePerFile;

    /** Maximum data file size */
    static constexpr std::uint32_t kMaxDataFileSize = kIndexFileHeaderSize + (100 * 1024 * 1024);

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
     * Returns record size.
     * @return Record size.
     */
    auto getRecordSize() const noexcept
    {
        return m_recordSize;
    }

    /**
     * Returns number of records per file.
     * @return Number of records per file.
     */
    auto getNumberOfRecordsPerFile() const noexcept
    {
        return m_numberOfRecordsPerFile;
    }

    /**
     * Returns data file size if applicable.
     * @return Data file size or zero if not applicable.
     */
    std::uint32_t getDataFileSize() const noexcept override;

    /**
     * Pre-allocates space for storing key.
     * @param key A key buffer.
     * @return true if key space allocated, false if key space has already existed.
     */
    bool preallocate(const void* key) override;

    /**
     * Inserts data into the index.
     * @param key A key buffer.
     * @param value A value buffer.
     * @return true if key was new one, false if key already existed.
     */
    bool insert(const void* key, const void* value) override;

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
     * @return Number of updated values.
     */
    std::uint64_t update(const void* key, const void* value) override;

    /** Writes cached changes to disk. */
    void flush() override;

    /**
     * Finds key and reads corresponding value.
     * @param key A key buffer.
     * @param value An output buffer.
     * @param count Number of values that can fit in the outout buffer.
     * @return Number of values actually copied.
     */
    std::uint64_t find(const void* key, void* value, std::size_t count) override;

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
        IndexFileHeader(IndexType indexType) noexcept
            : IndexFileHeaderBase(indexType)
        {
        }

        /**
         * Initializes instance of class IndexFileHeaderBase.
         * @param databaseUuid Database UUID.
         * @param tableId Table ID.
         * @param indexId Index ID.
         * @param indexType Index type.
         */
        IndexFileHeader(const Uuid& databaseUuid, std::uint32_t tableId, std::uint64_t indexId,
                IndexType indexType) noexcept
            : IndexFileHeaderBase(databaseUuid, tableId, indexId, indexType)
        {
        }

        /** Equality operator */
        bool operator==(const IndexFileHeader& other) const noexcept
        {
            return IndexFileHeaderBase::operator==(other);
        }

        /** Non-equality operator */
        bool operator!=(const IndexFileHeader& other) const noexcept
        {
            return !(*this == other);
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
        kValueStateExists1 = 1,
        kValueStateExists2 = 2,
    };

private:
    /**
     * Validates data file size.
     * @param size Data file size.
     * @return Size if it is valid.
     * @throw std::invalid_argument if size is invalid.
     */
    static std::uint32_t validateIndexFileSize(std::uint32_t size);

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
     * Returns file with given ID.
     * @param fileId File Id.
     * @return File object if file found.
     * @throw DatabaseError if file not found.
     */
    uli::FileDataPtr findFileChecked(std::uint64_t fileId);

    /**
     * Returns file with given ID.
     * @param fileId File Id.
     * @return File object or nullptr if not found.
     */
    uli::FileDataPtr findFile(std::uint64_t fileId);

    /**
     * Makes new physical file and corresponding file object.
     * @param fileId File identifier.
     * @return File object.
     */
    uli::FileDataPtr makeFile(std::uint64_t fileId);

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
     * Returns file number which should contain the key.
     * @param key A key.
     * @return File number which should contain the key.
     */
    std::uint64_t getFileIdForKey(std::uint64_t key) const noexcept
    {
        return (key / m_numberOfRecordsPerFile) + 1;
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
     * Updates min and max keys after deleting key from index.
     * @param removedKey Removed key.
     */
    void updateMinAndMaxKeysAfterRemoval(const void* removedKey);

    /**
     * Scans index data directory for data files and returns list of data file IDs.
     * @return List of file IDs.
     */
    std::set<std::uint64_t> scanFiles() const;

    /**
     * Returns minimum available file ID.
     * @return Minimum available file ID.
     */
    std::uint64_t getMinAvailableFileId() const noexcept
    {
        return SIODB_UNLIKELY(m_fileIds.empty()) ? 0 : *m_fileIds.begin();
    }

    /**
     * Returns maximum available file ID.
     * @return Maximum available file ID.
     */
    std::uint64_t getMaxAvailableFileId() const noexcept
    {
        return SIODB_UNLIKELY(m_fileIds.empty()) ? 0 : *m_fileIds.rbegin();
    }

    /**
     * Computes maximum possible file ID for this index.
     * @return Maximum possible file ID.
     */
    std::uint64_t computeMaxPossibleFileId() const noexcept;

    /**
     * Computes index record size.
     * @return Index record size.
     */
    std::size_t computeIndexRecordSize() const noexcept
    {
        return m_valueSize * 2 + 1;
    }

    /**
     * Computes number of index records in the single file.
     * @return Number of index records in the single file.
     */
    std::uint64_t computeNumberOfRecordsPerFile() const noexcept
    {
        return (m_dataFileSize - kIndexFileHeaderSize) / m_recordSize;
    }

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

    /** Number of records per file */
    const std::size_t m_numberOfRecordsPerFile;

    /** Minimum possible key */
    const BinaryValue m_minPossibleKey;

    /** Maximum possible key */
    const BinaryValue m_maxPossibleKey;

    /** Maximum possible file ID */
    const std::uint64_t m_maxPossibleFileId;

    /** Sorted list of file IDs */
    std::set<std::uint64_t> m_fileIds;

    /** File cache */
    uli::FileCache m_fileCache;

    /** Actual minimum key */
    BinaryValue m_minKey;

    /** Actual maximum key */
    BinaryValue m_maxKey;

    /** Actual minimum key - encoded */
    std::uint64_t m_minNumericKey;

    /** Actual maximum key - encoded */
    std::uint64_t m_maxNumericKey;

    /** File cache capacity */
    static constexpr std::size_t kFileCacheCapacity = 20;
};

}  // namespace siodb::iomgr::dbengine
