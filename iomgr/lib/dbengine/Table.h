// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnConstraintSpecification.h"
#include "ColumnDataAddress.h"
#include "ColumnDefinitionPtr.h"
#include "ColumnPtr.h"
#include "ColumnSetCache.h"
#include "ConstraintCache.h"
#include "Database.h"
#include "IndexPtr.h"
#include "TableColumns.h"
#include "TablePtr.h"
#include "Variant.h"

namespace siodb::iomgr::dbengine {

class ColumnSet;
class ColumnDefinition;
class Constraint;

/** Database table */
class Table : public std::enable_shared_from_this<Table> {
public:
    /**
     * Initializes object of class Table for the new table.
     * @param database Database object.
     * @param type Table type.
     * @param name Table name.
     * @param firstUserTrid First user range TRID.
     */
    Table(Database& database, TableType type, const std::string& name, std::uint64_t firstUserTrid);

    /**
     * Initializes object of class Table for the existing table.
     * @param database Database object.
     * @param tableRecord Table record from registry.
     */
    Table(Database& database, const TableRecord& tableRecord);

    DECLARE_NONCOPYABLE(Table);

    /**
     * Returns database object.
     * @return Database object.
     */
    Database& getDatabase() const
    {
        return m_database;
    }

    /**
     * Returns database UUID.
     * @return Database UUID.
     */
    const Uuid& getDatabaseUuid() const noexcept
    {
        return m_database.getUuid();
    }

    /**
     * Returns database name.
     * @return Database name.
     */
    const std::string& getDatabaseName() const noexcept
    {
        return m_database.getName();
    }

    /**
     * Returns table ID.
     * @return Table ID.
     */
    auto getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns table type.
     * @return Table type.
     */
    auto getType() const noexcept
    {
        return m_type;
    }

    /**
     * Returns table name.
     * @return Table name.
     */
    const std::string& getName() const noexcept
    {
        return m_name;
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
     * Returns display name of the table.
     * @return Display name.
     */
    std::string getDisplayName() const;

    /**
     * Returns display code of the table.
     * @return Display code.
     */
    std::string getDisplayCode() const;

    /**
     * Return indication that this is system table.
     * @return true if this is system table, false otherwise.
     */
    bool isSystemTable() const noexcept
    {
        return m_isSystemTable;
    }

    /**
     * Returns first user range TRID.
     * @return First user range TRID.
     */
    auto getFirstUserTrid() const noexcept
    {
        return m_firstUserTrid;
    }

    /**
     * Returns number of column in the table.
     * @return Number of columns in the table.
     */
    std::size_t getColumnCount() const
    {
        std::lock_guard lock(m_mutex);
        return m_currentColumns.size();
    }

    /**
     * Returns indication that column with provided name exists.
     * @param columnName Column name.
     * @return true if table with provided name exists, false otherwise.
     */
    bool isColumnExists(const std::string& columnName) const
    {
        std::lock_guard lock(m_mutex);
        return isColumnExistsUnlocked(columnName);
    }

    /**
     * Returns cached column info by column ID.
     * @param columnId Column ID.
     * @return Table column record if such column exists in the current column set.
     * @throw DatabaseError if column doesn't exist in cache.
     */
    TableColumn getColumnById(std::uint64_t columnId) const
    {
        std::lock_guard lock(m_mutex);
        return getColumnByIdUnlocked(columnId);
    }

    /**
     * Returns cached column info by column position.
     * @param position Column position.
     * @return Table column record if position is valid.
     * @throw DatabaseError if column doesn't exist in the current column set.
     */
    TableColumn getColumnByPosition(std::uint32_t position) const
    {
        std::lock_guard lock(m_mutex);
        return getColumnByPositionUnlocked(position);
    }

    /**
     * Returns current position of the column in the table.
     * @param columnId Column ID.
     * @return Current postion of the column in table.
     * @throw DatabaseError, if column is not a member of table.
     */
    std::uint32_t getColumnCurrentPosition(std::uint64_t columnId) const;

    /**
     * Returns column list sorted by position.
     * @return Column list sorted by position.
     */
    std::vector<ColumnPtr> getColumnsOrderedByPosition() const;

    /**
     * Returns current column set ID.
     * @return Current column set ID.
     */
    std::uint64_t getCurrentColumnSetId() const;

    /**
     * Returns current column set.
     * @return Current column set.
     */
    std::shared_ptr<ColumnSet> getCurrentColumnSet() const
    {
        std::lock_guard lock(m_mutex);
        return m_currentColumnSet;
    }

    /**
     * Returns current column with given ID.
     * @param columnSetId Column set ID.
     * @return Column set with given ID.
     * @throw DatabaseError if column set doesn't exist
     */
    ColumnSetPtr getColumnSetChecked(std::uint64_t columnSetId);

    /**
     * Creates new column set.
     * @return New column set.
     */
    ColumnSetPtr createColumnSet()
    {
        std::lock_guard lock(m_mutex);
        return createColumnSetUnlocked();
    }

    /** Closes current column set */
    void closeCurrentColumnSet();

    /**
     * Creates new column object and writes all necessary on-disk data structures.
     * @param columnSpec Column specification.
     * @param firstUserTrid First user range TRID (effective only for the master column).
     * @return New column object.
     */
    ColumnPtr createColumn(const ColumnSpecification& columnSpec, std::uint64_t firstUserTrid = 1);

    /**
     * Returns existing master column object.
     * @return Corresponding column object.
     */
    ColumnPtr getMasterColumn() const
    {
        std::lock_guard lock(m_mutex);
        return m_masterColumn;
    }

    /**
     * Returns master column main index object.
     * @return Master column main index object.
     */
    IndexPtr getMasterColumnMainIndex() const;

    /**
     * Returns existing column object.
     * @param columnId Column ID.
     * @return Corresponding column object. Throws exception if requested column doesn't exist.
     */
    ColumnPtr getColumnChecked(std::uint64_t columnId) const
    {
        std::lock_guard lock(m_mutex);
        return getColumnCheckedUnlocked(columnId);
    }

    /**
     * Returns existing column object.
     * @param columnName Column name.
     * @return Corresponding column object. Throws exception if requested column doesn't exist.
     */
    ColumnPtr getColumnChecked(const std::string& columnName) const
    {
        std::lock_guard lock(m_mutex);
        return getColumnCheckedUnlocked(columnName);
    }

    /**
     * Returns existing column object.
     * @param columnId Column ID.
     * @return Corresponding column object or nullptr if it doesn't exist.
     */
    ColumnPtr getColumn(std::uint64_t columnId) const
    {
        std::lock_guard lock(m_mutex);
        return getColumnUnlocked(columnId);
    }

    /**
     * Returns existing column object.
     * @param columnName Column name.
     * @return Corresponding column object or nullptr if it doesn't exist.
     */
    ColumnPtr getColumn(const std::string& columnName) const
    {
        std::lock_guard lock(m_mutex);
        return getColumnUnlocked(columnName);
    }

    /**
     * Returns existing column position.
     * @param columnId Column ID.
     * @return Corresponding column position or empty value if it doesn't exist.
     */
    std::optional<std::uint32_t> getColumnPosition(std::uint64_t columnId) const
    {
        std::lock_guard lock(m_mutex);
        return getColumnPositionUnlocked(columnId);
    }

    /**
     * Returns existing column position.
     * @param columnName Column name.
     * @return Corresponding column position or empty value if it doesn't exist.
     */
    std::optional<std::uint32_t> getColumnPosition(const std::string& columnName) const
    {
        std::lock_guard lock(m_mutex);
        return getColumnPositionUnlocked(columnName);
    }

    /**
     * Checks that given column belongs to this table.
     * @param column A column to be checked.
     * @param operationName Calling operation name.
     * @throw DatabaseError if column doesn't belong to this table.
     */
    void checkColumnBelongsToTable(const Column& column, const char* operationName) const;

    /**
     * Creates new constraint object.
     * @param name Constraint name.
     * @param constraintDefintion Constraint definition.
     * @param column A column to which constraint brlongs, or nullptr if this is table constraint.
     * @return Constraint object.
     * @throw DatabaseError if costraint already exists.
     */
    ConstraintPtr createConstraint(const std::string& name,
            const ConstConstraintDefinitionPtr& constraintDefinition, Column* column);

    /**
     * Returns existing constraint object.
     * @param column Column, to which constraint belongs ot nullptr for table constraint.
     * @param constraintId Constraint ID.
     * @return Constraint object.
     * @throw DatabaseError if costraint doesn't exist.
     */
    ConstraintPtr getConstraintChecked(Column* column, std::uint64_t constraintId);

    /**
     * Returns "NOT NULL" system constraint definition.
     * @return Constraint definition object.
     */
    ConstraintDefinitionPtr getSystemNotNullConstraintDefinition() const noexcept
    {
        return m_database.getSystemNotNullConstraintDefinition();
    }

    /**
     * Returns constaint definition object if it exists.
     * @param constraintDefinitionId Constraint definition ID.
     * @return Constraint definition object.
     * @throws DatabaseError if constraint definition doesn't exist.
     */
    ConstraintDefinitionPtr getConstraintDefinitionChecked(std::uint64_t constraintDefinitionId)
    {
        return m_database.getConstraintDefinitionChecked(constraintDefinitionId);
    }

    /**
     * Inserts new row into the table. Allows specifying columns in the custom order.
     * @param columnNames Column names.
     * @param columnValues Column values. May be modified by this function.
     * @param transactionParameters Transaction parameters.
     * @param customTrid Custom TRID to use. Zero causes automatic generation of new TRID.
     * @return Pair of (master column record, next block IDs).
     * @throw DatabaseError if operation has failed.
     */
    std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> insertRow(
            const std::vector<std::string>& columnNames, std::vector<Variant>& columnValues,
            const TransactionParameters& transactionParameters, std::uint64_t customTrid = 0);

    /**
     * Inserts new row into the table. Assumes values correspond to columns in other order
     * they are in the table.
     * @param columnValues Column values. May be modified by this function.
     * @param transactionParameters Transaction parameters.
     * @param customTrid Custom TRID to use. Zero causes automatic generation of new TRID.
     * @return Pair of (master column record, next block IDs).
     * @throw DatabaseError if operation has failed.
     */
    std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> insertRow(
            std::vector<Variant>& columnValues, const TransactionParameters& transactionParameters,
            std::uint64_t customTrid = 0);

    /**
     * Deletes existing row from the table.
     * @param trid Table row ID.
     * @param transactionParameters Transaction parameters.
     * @return true if operation succeeded, false if row not found.
     * @throw DatabaseError if operation has failed.
     */
    bool deleteRow(std::uint64_t trid, const TransactionParameters& transactionParameters);

    /**
     * Deletes existing row from the table.
     * @param mcr Master column record to be updated.
     * @param mcrAddress MCR address.
     * @param transactionParameters Transaction parameters.
     * @throw DatabaseError if operation has failed.
     */
    void deleteRow(const MasterColumnRecord& mcr, const ColumnDataAddress& mcrAddress,
            const TransactionParameters& transactionParameters);

    /**
     * Updates existing row.
     * @param trid Table row ID.
     * @param columnValues New values.
     * @param columnPositions Positions of columns to place values.
     * @param tp Transaction parameters.
     * @return true if operation succeeded, false if row not found.
     * @throw DatabaseError if operation has failed for any other reason than row not found.
     */
    bool updateRow(std::uint64_t trid, std::vector<Variant>&& columnValues,
            const std::vector<std::size_t>& columnPositions, const TransactionParameters& tp);

    /**
     * Updates existing row.
     * @param mcr Master column record to be updated.
     * @param mcrAddress MCR address.
     * @param columnValues New values.
     * @param columnPositions Positions of columns to place values.
     * @param tp Transaction parameters.
     * @throw DatabaseError if operation has failed.
     */
    void updateRow(const MasterColumnRecord& mcr, const ColumnDataAddress& mcrAddress,
            std::vector<Variant>&& columnValues, const std::vector<std::size_t>& columnPositions,
            const TransactionParameters& tp);

    /**
     * Rolls back last recorded row.
     * @param mcr Master column record.
     * @param nextBlockIds List of next block IDs in the chain.
     */
    void rollbackLastRow(
            const MasterColumnRecord& mcr, const std::vector<std::uint64_t>& nextBlockIds);

    /** Flushes all pending changes in indices to disk. */
    void flushIndices();

    /**
     * Generates next TRID from the user TRID range.
     * @return Next user record TRID.
     */
    std::uint64_t generateNextUserTrid();

    /**
     * Generates next TRID from the system TRID range.
     * @return Next system objet record TRID.
     */
    std::uint64_t generateNextSystemTrid();

    /**
     * Sets last system TRID value. Used to adjust TRID counter when initializing database.
     * @param lastSystemTrid Last system TRID value.
     */
    void setLastSystemTrid(std::uint64_t lastSystemTrid);

    /**
     * Returns existing column definition object.
     * @param columnDefinitionId Column definition ID.
     * @throw DatabaseError if column definition doesn't exist.
     */
    ColumnDefinitionPtr getColumnDefinitionChecked(std::uint64_t columnDefinitionId);

private:
    /**
     * Validates table name.
     * @param tableName Table name.
     * @return The same table name.
     * @throw DatabaseError if table name is invalid.
     */
    static const std::string& validateTableName(const std::string& tableName);

    /**
     * Creates new master column object and writes all necessary on-disk data structures.
     * @param firstUserTrid First user range TRID.
     */
    void createMasterColumn(std::uint64_t firstUserTrid);

    /** Loads all columns. */
    void loadColumnsUnlocked();

    /**
     * Creates new column set. Doesn't acquire access synchronization lock for the
     * column set cache.
     * @return New column set object.
     */
    ColumnSetPtr createColumnSetUnlocked();

    /**
     * Creates column set from a given column set registry record. Doesn't acquire access
     * synchronization lock for the column set cache.
     * @param columnSetRecord Column set registry record.
     * @return Column set object.
     */
    ColumnSetPtr createColumnSetUnlocked(const ColumnSetRecord& columnSetRecord);

    /**
     * Creates new constraint. Doesn't acquire access synchronization lock for the
     * constraint cache.
     * @param column Column to which constraint belongs. nullptr indicates table constraint.
     * @param name Constraint name.
     * @param constraintDefinition Constraint definition.
     * @return New column set object.
     */
    ConstraintPtr createConstraintUnlocked(Column* column, const std::string& name,
            const ConstConstraintDefinitionPtr& constraintDefinition);

    /**
     * Creates constraint from a given constraint registry record. Doesn't acquire access
     * synchronization lock for the column set cache.
     * @param column Column to which the constraint belongs, or nullptr for table constraint.
     * @param constraintRecord Constraint registry record.
     * @return Constraint object.
     */
    ConstraintPtr createConstraintUnlocked(
            Column* column, const ConstraintRecord& constraintRecord);

    /**
     * Returns indication that column with provided name exists.
     * @param columnName Column name.
     * @return true if table with provided name exists, false otherwise.
     */
    bool isColumnExistsUnlocked(const std::string& columnName) const noexcept
    {
        return m_currentColumns.byName().count(columnName) > 0;
    }

    /**
     * Returns cached column info by column ID.
     * @param columnId Column ID.
     * @return Column record or throws exception if column doesn't exist in cache.
     */
    TableColumn getColumnByIdUnlocked(std::uint64_t columnId) const;

    /**
     * Returns cached column info by column position.
     * @param position Column ID.
     * @return Table column record if such column exists.
     * @throw DatabaseException if column doesn't exist in the current column set.
     */
    TableColumn getColumnByPositionUnlocked(std::uint32_t position) const;

    /**
     * Returns cached column info by column name.
     * @param columnName Column name.
     * @return Column record or throws exception if column doesn't exist in cache.
     */
    ColumnRecord getColumnInfoByNameUnlocked(const std::string& columnName) const;

    /**
     * Returns existing column object. Does not acquire column registry lock.
     * @param columnId Column Id.
     * @return Corresponding column object. Throws exception if requested column doesn't exist.
     */
    ColumnPtr getColumnCheckedUnlocked(uint64_t columnId) const;

    /**
     * Returns existing column object. Does not acquire column registry lock.
     * @param columnName Column name.
     * @return Corresponding column object. Throws exception if requested column doesn't exist.
     */
    ColumnPtr getColumnCheckedUnlocked(const std::string& columnName) const;

    /**
     * Returns existing column object. Does not acquire column registry lock.
     * @param columnId Column Id.
     * @return Corresponding column object or nullptr if it doesn't exist.
     */
    ColumnPtr getColumnUnlocked(uint64_t columnId) const noexcept;

    /**
     * Returns existing column object. Does not acquire column registry lock.
     * @param columnName Column name.
     * @return Corresponding column object or nullptr if it doesn't exist.
     */
    ColumnPtr getColumnUnlocked(const std::string& columnName) const noexcept;

    /**
     * Returns existing column position. Does not acquire column registry lock.
     * @param columnId Column Id.
     * @return Corresponding column position or empty value if it doesn't exist.
     */
    std::optional<std::uint32_t> getColumnPositionUnlocked(uint64_t columnId) const noexcept;

    /**
     * Returns existing column position. Does not acquire column registry lock.
     * @param columnName Column name.
     * @return Corresponding column position or empty value if it doesn't exist.
     */
    std::optional<std::uint32_t> getColumnPositionUnlocked(const std::string& columnName) const
            noexcept;

    /**
     * Ensures that data directory exists and initialized if required.
     * @param dataDir Data directory.
     * @param create Indicates that data directoty must be created.
     * @return Data directory.
     */
    std::string&& ensureDataDir(std::string&& dataDir, bool create) const;

    /** Creates initialization flag file. */
    void createInitializationFlagFile() const;

    /**
     * Inserts new row into the table. Assumes values correspond to columns in other order
     * they are in the table. Does not obtain column registry lock.
     * @param columnValues Column values. May be modified by this function.
     * @param tp Transaction parameters.
     * @param customTrid Custom TRID to use. Zero causes automatic generation of new TRID.
     * @return Pair of (master column record, next block IDs).
     * @throw DatabaseError if operation has failed.
     */
    std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> doInsertRowUnlocked(
            std::vector<Variant>& columnValues, const TransactionParameters& transactionParameters,
            std::uint64_t customTrid);

private:
    /** Database to which this table belongs */
    Database& m_database;

    /** Table name */
    std::string m_name;

    /** System table flag */
    const bool m_isSystemTable;

    /** Table ID */
    const std::uint32_t m_id;

    /** Table type */
    const TableType m_type;

    /** Table data directory */
    const std::string m_dataDir;

    /** Column registry synchronization object */
    mutable std::recursive_mutex m_mutex;

    /** Column sets */
    ColumnSetCache m_columnSetCache;

    /** Current column set */
    ColumnSetPtr m_currentColumnSet;

    /** Previous column set */
    ColumnSetPtr m_prevColumnSet;

    /** Current columns. Must be updated when column set changes. */
    TableColumns m_currentColumns;

    /** Constraint cache */
    ConstraintCache m_constraintCache;

    /** Master column reference */
    ColumnPtr m_masterColumn;

    /** 
     * Cached first user TRID.
     * NOTE: We have to keep it here, to prevent some crashes.
     */
    const std::uint64_t m_firstUserTrid;

    /** Initialization flag file name */
    static constexpr const char* kInitializationFlagFile = "initialized";

    /** Table directory prefix */
    static constexpr const char* kTableDataDirPrefix = "t";

    /** Column set cache capacity */
    static constexpr std::size_t kColumnSetCacheCapacity = 10;

    /** Constraint cache capacity */
    static constexpr std::size_t kConstraintCacheCapacity = 256;
};

}  // namespace siodb::iomgr::dbengine
