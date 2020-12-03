// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "IndexColumnPtr.h"
#include "IndexColumnSpecification.h"
#include "IndexPtr.h"
#include "Table.h"
#include "ikt/IndexKeyTraits.h"

// Common project headers
#include <siodb/common/utils/HelperMacros.h>

namespace siodb::iomgr::dbengine {

class IndexColumn;

/** Index value record */
struct IndexValue {
    /** Data */
    std::uint8_t m_data[12];
};

/** Abstract column index */
class Index {
public:
    /** Index file prefix */
    static constexpr const char* kIndexFilePrefix = "i";

public:
    /**
     * 3-way key comparison function type.
     * Returns 0 if keys are equals, -1 if left key is less than right key,
     * 1 if left key is greater than right key.
     */
    typedef int (*KeyCompareFunction)(const void*, const void*) noexcept;

protected:
    /**
     * Initializes object of class Index for a new index.
     * @param table A table to which this index belongs.
     * @param type Index type.
     * @param name Index name.
     * @param keyTraits Key traits.
     * @param valueSize Value size.
     * @param keyCompare Key comparison function.
     * @param unique Index uniqueness flag.
     * @param columns Indexed column list.
     * @param description Index description.
     */
    Index(Table& table, IndexType type, std::string&& name, const IndexKeyTraits& keyTraits,
            std::size_t valueSize, KeyCompareFunction keyCompare, bool unique,
            const IndexColumnSpecificationList& columns, std::optional<std::string>&& description);

    /**
     * Initializes object of class Index for an existing index.
     * @param table A table to which this index belongs.
     * @param indexRecord Index record.
     * @param keyTraits Key traits.
     * @param valueSize Value size.
     * @param keyCompare Key comparison function.
     */
    Index(Table& table, const IndexRecord& indexRecord, const IndexKeyTraits& keyTraits,
            std::size_t valueSize, KeyCompareFunction keyCompare);

public:
    /** De-initializes object of class Index */
    virtual ~Index() = default;

    DECLARE_NONCOPYABLE(Index);

    /**
     * Returns index type.
     * @return Index type.
     */
    IndexType getType() const noexcept
    {
        return m_type;
    }

    /**
     * Returns index ID.
     * @return Index ID
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns index name.
     * @return Index name.
     */
    const auto& getName() const noexcept
    {
        return m_name;
    }

    /**
     * Returns index description.
     * @return Index description.
     */
    const auto& getDescription() const noexcept
    {
        return m_description;
    }

    /**
     * Returns database object.
     * @return Database object.
     */
    Database& getDatabase() const noexcept
    {
        return m_table.getDatabase();
    }

    /**
     * Returns database UUID.
     * @return Database UUID.
     */
    const Uuid& getDatabaseUuid() const noexcept
    {
        return m_table.getDatabaseUuid();
    }

    /**
     * Returns database name.
     * @return Database name.
     */
    const std::string& getDatabaseName() const noexcept
    {
        return m_table.getDatabaseName();
    }

    /**
     * Returns table object.
     * @return Table object.
     */
    Table& getTable() const noexcept
    {
        return m_table;
    }

    /**
     * Returns table ID.
     * @return Table ID.
     */
    std::uint32_t getTableId() const noexcept
    {
        return m_table.getId();
    }

    /**
     * Returns table name.
     * @return Table name.
     */
    const std::string& getTableName() const noexcept
    {
        return m_table.getName();
    }

    /**
     * Return indication that this index is unique index.
     * @return true if index is unique, false otherwise/
     */
    bool isUnique() const noexcept
    {
        return m_unique;
    }

    /**
     * Returns list of indexed columns with direction.
     * @return List of indexed columns with direction.
     */
    const auto& getColumns() const noexcept
    {
        return m_columns;
    }

    /**
     * Returns table data directory path.
     * @return data directory path.
     */
    const auto& getDataDir() const noexcept
    {
        return m_dataDir;
    }

    /**
     * Returns display name of the database.
     * @return Display name.
     */
    std::string makeDisplayName() const;

    /**
     * Returns display code of the database.
     * @return Display code.
     */
    std::string makeDisplayCode() const;

    /**
     * Creates index file path.
     * @param fileId File ID.
     * @return Index file path.
     */
    std::string makeIndexFilePath(std::uint64_t fileId) const;

    /**
     * Returns data file size if applicable.
     * @return Data file size or zero if not applicable.
     */
    virtual std::uint32_t getDataFileSize() const noexcept = 0;

    /**
     * Pre-allocates space for storing key.
     * @param key A key buffer.
     * @return true if key space allocated, false if key space has already existed.
     */
    virtual bool preallocate(const void* key) = 0;

    /**
     * Inserts data into the index.
     * @param key A key buffer.
     * @param value A value buffer.
     * @return true if key was new one, false if key already existed.
     */
    virtual bool insert(const void* key, const void* value) = 0;

    /**
     * Deletes data the index.
     * @param key A key buffer.
     * @return true Number of deleted entries.
     */
    virtual std::uint64_t erase(const void* key) = 0;

    /**
     * Updates data in the index.
     * @param key A key buffer.
     * @param value A value buffer.
     * @return Number of updated values.
     */
    virtual std::uint64_t update(const void* key, const void* value) = 0;

    /**
     * Writes cached changes to disk.
     */
    virtual void flush() = 0;

    /**
     * Finds key and reads corresponding value.
     * @param key A key buffer.
     * @param value An output buffer.
     * @param count Number of values that can fit in the outout buffer.
     * @return Number of values actually copied.
     */
    virtual std::uint64_t find(const void* key, void* value, std::size_t count) = 0;

    /**
     * Counts how much values available for this key.
     * @param key A key buffer.
     * @return Number of values copied available.
     */
    virtual std::uint64_t count(const void* key) = 0;

    /**
     * Returns minimum key in the index.
     * @param key Buffer for storing key.
     * @return true if minimum key exists, false if index is empty.
     */
    virtual bool getMinKey(void* key) = 0;

    /**
     * Returns maximum key in the index.
     * @param key Buffer for storing key.
     * @return true if minimum key exists, false if index is empty.
     */
    virtual bool getMaxKey(void* key) = 0;

    /**
     * Returns first key in the index. Always reads index storage.
     * @param key Buffer for storing key.
     * @return true if minimum key exists, false if index is empty.
     */
    virtual bool findFirstKey(void* key) = 0;

    /**
     * Returns last key in the index storage. Always reads index storage.
     * @param key Buffer for storing key.
     * @return true if minimum key exists, false if index is empty.
     */
    virtual bool findLastKey(void* key) = 0;

    /**
     * Returns previous key in the index.
     * @param key Current key.
     * @param prevKey Buffer for storing previous key.
     * @return true if previous key obtained, false otherwise.
     */
    virtual bool findPreviousKey(const void* key, void* prevKey) = 0;

    /**
     * Returns next key in the index.
     * @param key Current key.
     * @param nextKey Buffer for storing next key.
     * @return true if next key obtained, false otherwise.
     */
    virtual bool findNextKey(const void* key, void* nextKey) = 0;

protected:
    /** Creates initialization flag file. */
    void createInitializationFlagFile() const;

private:
    /** Index column collection type */
    using IndexColumnCollection = std::vector<IndexColumnPtr>;

private:
    /**
     * Validates index name.
     * @param indexName Index name.
     * @return The same inex name.
     * @throw DatabaseError if inex name is invalid.
     */
    static std::string&& validateIndexName(std::string&& indexName);

    /**
     * Validates table.
     * @param table Table to which this index is supposed to belong to.
     * @param indexRecord registry record.
     * @return The same table, if it is valid.
     * @throw DatabaseError if table has different ID.
     */
    static Table& validateTable(Table& table, const IndexRecord& indexRecord);

    /**
     * Ensures that data directory exists and initialized if required.
     * @param dataDir Data directory.
     * @param create Indicates that data directoty must be created.
     * @return Data directory.
     */
    std::string&& ensureDataDir(std::string&& dataDir, bool create) const;

    /**
     * Creates index column objects from index column specifications.
     * @param indexColumnSpecs Index column specifications.
     * @return Collection of IndexColumn objects.
     */
    IndexColumnCollection makeIndexColumns(
            const IndexColumnSpecificationList& indexColumnSpecs) const;

    /**
     * Creates index column objects from index column records.
     * @param indexColumnRegistry Index column record registry.
     * @return Collection of IndexColumn objects.
     */
    IndexColumnCollection makeIndexColumns(const IndexColumnRegistry& indexColumnRegistry) const;

protected:
    /** Table to which this index belongs */
    Table& m_table;

    /** Index type */
    const IndexType m_type;

    /** Index name */
    const std::string m_name;

    /** Index name */
    std::optional<std::string> m_description;

    /** Index ID */
    const std::uint64_t m_id;

    /** Data directory */
    const std::string m_dataDir;

    /** Key size */
    const std::size_t m_keySize;

    /** Value size */
    const std::size_t m_valueSize;

    /** Key-value pair size */
    const std::size_t m_kvPairSize;

    /** Key comparison function */
    const KeyCompareFunction m_keyCompare;

    /** Unique flag */
    const bool m_unique;

    /** List of indexed columns */
    const IndexColumnCollection m_columns;

    /** Index data directory prefix */
    static constexpr const char* kIndexDataDirPrefix = "i";
};

}  // namespace siodb::iomgr::dbengine
