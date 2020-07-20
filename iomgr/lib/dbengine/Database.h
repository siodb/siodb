// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "ColumnSetPtr.h"
#include "ColumnSpecification.h"
#include "ConstraintDefinitionCache.h"
#include "ConstraintPtr.h"
#include "DatabaseMetadata.h"
#include "DatabasePtr.h"
#include "FirstUserObjectId.h"
#include "Instance.h"
#include "MasterColumnRecordPtr.h"
#include "TableCache.h"
#include "TransactionParameters.h"
#include "User.h"
#include "parser/expr/Expression.h"
#include "reg/ColumnDefinitionRegistry.h"
#include "reg/ColumnRegistry.h"
#include "reg/ColumnSetRegistry.h"
#include "reg/ConstraintDefinitionRegistry.h"
#include "reg/ConstraintRegistry.h"
#include "reg/DatabaseRecord.h"
#include "reg/IndexRegistry.h"
#include "reg/TableRegistry.h"

// Common project headers
#include <siodb/common/io/MemoryMappedFile.h>
#include <siodb/iomgr/shared/dbengine/OrderingType.h>
#include <siodb/iomgr/shared/dbengine/crypto/ciphers/Cipher.h>
#include <siodb/iomgr/shared/dbengine/io/File.h>

// STL headers
#include <unordered_map>
#include <unordered_set>

namespace siodb::iomgr::dbengine {

class Column;
class ColumnDataBlock;
class ConstraintDefinition;
class Constraint;
class ColumnDefinition;
class ColumnDefinitionConstraint;
class Index;
class MasterColumnRecord;
class SystemDatabase;
using ::siodb::io::MemoryMappedFile;

/** Database object */
class Database : public std::enable_shared_from_this<Database> {
public:
    // IMPORTANT: ALL below database, table and column names MUST BE UPPERCASE

    /** Master column name */
    static constexpr const char* kMasterColumnName = "TRID";

    /** Master column description */
    static constexpr const char* kMasterColumnDescription = "Unique row identifier";

    /** NOT NULL constraint description */
    static constexpr const char* kSystemNotNullConstraintDescription =
            "Forces non-null values on the column";

    /** Table SYS_DATABASES */
    static constexpr const char* kSysDatabasesTableName = "SYS_DATABASES";
    static constexpr const char* kSysDatabasesTableDescription =
            "Stores information about known databases";
    static constexpr const char* kSysDatabases_Name_ColumnName = "NAME";
    static constexpr const char* kSysDatabases_Name_ColumnDescription = "Database name";
    static constexpr const char* kSysDatabases_Uuid_ColumnName = "UUID";
    static constexpr const char* kSysDatabases_Uuid_ColumnDescription = "Database UUID";
    static constexpr const char* kSysDatabases_CipherId_ColumnName = "CIPHER_ID";
    static constexpr const char* kSysDatabases_CipherId_ColumnDescription = "Cipher identifier";
    static constexpr const char* kSysDatabases_CipherKey_ColumnName = "CIPHER_KEY";
    static constexpr const char* kSysDatabases_CipherKey_ColumnDescription = "Encryption key";
    static constexpr const char* kSysDatabases_Description_ColumnName = "DESCRIPTION";
    static constexpr const char* kSysDatabases_Description_ColumnDescription =
            "Database description";

    /** Table SYS_TABLES */
    static constexpr const char* kSysTablesTableName = "SYS_TABLES";
    static constexpr const char* kSysTablesTableDescription =
            "Stores information about known tables";
    static constexpr const char* kSysTables_Type_ColumnName = "TYPE";
    static constexpr const char* kSysTables_Type_ColumnDescription = "Table storage type";
    static constexpr const char* kSysTables_Name_ColumnName = "NAME";
    static constexpr const char* kSysTables_Name_ColumnDescription = "Table name";
    static constexpr const char* kSysTables_FirstUserTrid_ColumnName = "FIRST_USER_TRID";
    static constexpr const char* kSysTables_FirstUserTrid_ColumnDescription =
            "First user record row identifier";
    static constexpr const char* kSysTables_CurrentColumnSetId_ColumnName = "CURRENT_COLUMN_SET_ID";
    static constexpr const char* kSysTables_CurrentColumnSetId_ColumnDescription =
            "Current column set";
    static constexpr const char* kSysTables_Description_ColumnName = "DESCRIPTION";
    static constexpr const char* kSysTables_Description_ColumnDescription = "Table description";

    /** Table SYS_DUMMY */
    static constexpr const char* kSysDummyTableName = "SYS_DUMMY";
    static constexpr const char* kSysDummyTableDescription =
            "Helper table for computing constant expressions";
    static constexpr const char* kSysDummy_Dummy_ColumnName = "DUMMY";
    static constexpr const char* kSysDummy_Dummy_ColumnDescription = "Dummy column";

    /** Table SYS_COLUMN_SETS */
    static constexpr const char* kSysColumnSetsTableName = "SYS_COLUMN_SETS";
    static constexpr const char* kSysColumnSetsTableDescription =
            "Stores information about column sets";
    static constexpr const char* kSysColumnSets_TableId_ColumnName = "TABLE_ID";
    static constexpr const char* kSysColumnSets_TableId_ColumnDescription = "Table identifier";
    static constexpr const char* kSysColumnSets_ColumnCount_ColumnName = "COLUMN_COUNT";
    static constexpr const char* kSysColumnSets_ColumnCount_ColumnDescription =
            "Number of columns in this column set";

    /** Table SYS_COLUMNS */
    static constexpr const char* kSysColumnsTableName = "SYS_COLUMNS";
    static constexpr const char* kSysColumnsTableDescription =
            "Stores information about table columns";
    static constexpr const char* kSysColumns_TableId_ColumnName = "TABLE_ID";
    static constexpr const char* kSysColumns_TableId_ColumnDescription = "Table identifier";
    static constexpr const char* kSysColumns_DataType_ColumnName = "DATA_TYPE";
    static constexpr const char* kSysColumns_DataType_ColumnDescription = "Column data type";
    static constexpr const char* kSysColumns_Name_ColumnName = "NAME";
    static constexpr const char* kSysColumns_Name_ColumnDescription = "Column name";
    static constexpr const char* kSysColumns_State_ColumnName = "STATE";
    static constexpr const char* kSysColumns_State_ColumnDescription = "Column state";
    static constexpr const char* kSysColumns_BlockDataAreaSize_ColumnName = "BLOCK_DATA_AREA_SIZE";
    static constexpr const char* kSysColumns_BlockDataAreaSize_ColumnDescription =
            "Data area size in the block file";
    static constexpr const char* kSysColumns_Description_ColumnName = "DESCRIPTION";
    static constexpr const char* kSysColumns_Description_ColumnDescription = "Column description";

    /** Table SYS_COLUMN_DEFS */
    static constexpr const char* kSysColumnDefsTableName = "SYS_COLUMN_DEFS";
    static constexpr const char* kSysColumnDefsTableDescription =
            "Stores information about column definitions";
    static constexpr const char* kSysColumnDefs_ColumnId_ColumnName = "COLUMN_ID";
    static constexpr const char* kSysColumnDefs_ColumnId_ColumnDescription = "Column identifier";
    static constexpr const char* kSysColumnDefs_ConstraintCount_ColumnName = "CONSTRAINT_COUNT";
    static constexpr const char* kSysColumnDefs_ConstraintCount_ColumnDescription =
            "Number of constraints associated with this column definition";

    /** Table SYS_COLUMN_SET_COLUMNS */
    static constexpr const char* kSysColumnSetColumnsTableName = "SYS_COLUMN_SET_COLUMNS";
    static constexpr const char* kSysColumnSetColumnsTableDescription =
            "Stores information about inclusion of column definitions into column sets";
    static constexpr const char* kSysColumnSetColumns_ColumnSetId_ColumnName = "COLUMN_SET_ID";
    static constexpr const char* kSysColumnSetColumns_ColumnSetId_ColumnDescription =
            "Column set identifier";
    static constexpr const char* kSysColumnSetColumns_ColumnDefinitionId_ColumnName =
            "COLUMN_DEF_ID";
    static constexpr const char* kSysColumnSetColumns_ColumnDefinitionId_ColumnDescription =
            "Associated column definition identifier";

    /** Table SYS_CONSTRAINT_DEFS */
    static constexpr const char* kSysConstraintDefsTableName = "SYS_CONSTRAINT_DEFS";
    static constexpr const char* kSysConstraintDefsTableDescription =
            "Stores information about unique constraint definitions";
    static constexpr const char* kSysConstraintDefs_Type_ColumnName = "TYPE";
    static constexpr const char* kSysConstraintDefs_Type_ColumnDescription = "Constraint type";
    static constexpr const char* kSysConstraintDefs_Expr_ColumnName = "EXPR";
    static constexpr const char* kSysConstraintDefs_Expr_ColumnDescription =
            "Constraint expression";

    /** Table SYS_CONSTRAINTS */
    static constexpr const char* kSysConstraintsTableName = "SYS_CONSTRAINTS";
    static constexpr const char* kSysConstraintsTableDescription =
            "Stores information about constraints";
    static constexpr const char* kSysConstraints_Name_ColumnName = "NAME";
    static constexpr const char* kSysConstraints_Name_ColumnDescription = "Constraint name";
    static constexpr const char* kSysConstraints_State_ColumnName = "STATE";
    static constexpr const char* kSysConstraints_State_ColumnDescription = "Constraint state";
    static constexpr const char* kSysConstraints_TableId_ColumnName = "TABLE_ID";
    static constexpr const char* kSysConstraints_TableId_ColumnDescription =
            "Table identifier, to which this constraint belongs";
    static constexpr const char* kSysConstraints_ColumnId_ColumnName = "COLUMN_ID";
    static constexpr const char* kSysConstraints_ColumnId_ColumnDescription =
            "Column identifier, to which this constraint belongs";
    static constexpr const char* kSysConstraints_DefinitionId_ColumnName = "DEF_ID";
    static constexpr const char* kSysConstraints_DefinitionId_ColumnDescription =
            "Constraint definition identifier";
    static constexpr const char* kSysConstraints_Description_ColumnName = "DESCRIPTION";
    static constexpr const char* kSysConstraints_Description_ColumnDescription =
            "Constraint description";

    /** Table SYS_COLUMN_DEF_CONSTRAINTS */
    static constexpr const char* kSysColumnDefConstraintsTableName = "SYS_COLUMN_DEF_CONSTRAINTS";
    static constexpr const char* kSysColumnDefConstraintsTableDescription =
            "Stores information about constraints associated with column definitions";
    static constexpr const char* kSysColumnDefinitionConstraintList_ColumnDefinitionId_ColumnName =
            "COLUMN_DEF_ID";
    static constexpr const char*
            kSysColumnDefinitionConstraintList_ColumnDefinitionId_ColumnDescription =
                    "Column definition identifier";
    static constexpr const char* kSysColumnDefinitionConstraintList_ConstraintId_ColumnName =
            "CONSTRAINT_ID";
    static constexpr const char* kSysColumnDefinitionConstraintList_ConstraintId_ColumnDescription =
            "Associated constraint identifier";

    /** Table SYS_INDICES */
    static constexpr const char* kSysIndicesTableName = "SYS_INDICES";
    static constexpr const char* kSysIndicesTableDescription = "Stores information about indices";
    static constexpr const char* kSysIndices_Type_ColumnName = "TYPE";
    static constexpr const char* kSysIndices_Type_ColumnDescription = "Index type";
    static constexpr const char* kSysIndices_Unique_ColumnName = "UNIQUE";
    static constexpr const char* kSysIndices_Unique_ColumnDescription =
            "Indication that index is unique";
    static constexpr const char* kSysIndices_Name_ColumnName = "NAME";
    static constexpr const char* kSysIndices_Name_ColumnDescription = "Index name";
    static constexpr const char* kSysIndices_TableId_ColumnName = "TABLE_ID";
    static constexpr const char* kSysIndices_TableId_ColumnDescription =
            "Table identifier, to which index applies";
    static constexpr const char* kSysIndices_DataFileSize_ColumnName = "DATA_FILE_SIZE";
    static constexpr const char* kSysIndices_DataFileSize_ColumnDescription = "Data file size";
    static constexpr const char* kSysIndices_Description_ColumnName = "DESCRIPTION";
    static constexpr const char* kSysIndices_Description_ColumnDescription = "Index description";

    /** Table SYS_INDEX_COLUMNS */
    static constexpr const char* kSysIndexColumnsTableName = "SYS_INDEX_COLUMNS";
    static constexpr const char* kSysIndexColumnsTableDescription =
            "Stores information about indexed columns";
    static constexpr const char* kSysIndexColumns_IndexId_ColumnName = "INDEX_ID";
    static constexpr const char* kSysIndexColumns_IndexId_ColumnDescription = "Index identifier";
    static constexpr const char* kSysIndexColumns_ColumnDefinitionId_ColumnName = "COLUMN_DEF_ID";
    static constexpr const char* kSysIndexColumns_ColumnDefinitionId_ColumnDescription =
            "Associated column defintion identifier";
    static constexpr const char* kSysIndexColumns_SortDesc_ColumnName = "SORT_DESC";
    static constexpr const char* kSysIndexColumns_SortDesc_ColumnDescription =
            "Indication of descending sort order by this column";

    static constexpr const char* kSysUsersTableName = "SYS_USERS";
    static constexpr const char* kSysUsersTableDescription = "Stores information about users";
    static constexpr const char* kSysUsers_Name_ColumnName = "NAME";
    static constexpr const char* kSysUsers_Name_ColumnDescription = "User name";
    static constexpr const char* kSysUsers_RealName_ColumnName = "REAL_NAME";
    static constexpr const char* kSysUsers_RealName_ColumnDescription = "User's real name";
    static constexpr const char* kSysUsers_State_ColumnName = "STATE";
    static constexpr const char* kSysUsers_State_ColumnDescription = "User state";
    static constexpr const char* kSysUsers_Description_ColumnName = "DESCRIPTION";
    static constexpr const char* kSysUsers_Description_ColumnDescription = "User description";

    static constexpr const char* kSysUserAccessKeysTableName = "SYS_USER_ACCESS_KEYS";
    static constexpr const char* kSysUserAccessKeysTableDescription =
            "Stores information about user's access keys";
    static constexpr const char* kSysUserAccessKeys_UserId_ColumnName = "USER_ID";
    static constexpr const char* kSysUserAccessKeys_UserId_ColumnDescription = "User identifier";
    static constexpr const char* kSysUserAccessKeys_Name_ColumnName = "NAME";
    static constexpr const char* kSysUserAccessKeys_Name_ColumnDescription = "Access key name";
    static constexpr const char* kSysUserAccessKeys_Text_ColumnName = "TEXT";
    static constexpr const char* kSysUserAccessKeys_Text_ColumnDescription = "Access key text";
    static constexpr const char* kSysUserAccessKeys_State_ColumnName = "STATE";
    static constexpr const char* kSysUserAccessKeys_State_ColumnDescription = "Access key state";
    static constexpr const char* kSysUserAccessKeys_Description_ColumnName = "DESCRIPTION";
    static constexpr const char* kSysUserAccessKeys_Description_ColumnDescription =
            "Access key description";

    static constexpr const char* kSysUserPermissionsTableName = "SYS_USER_PERMISSIONS";
    static constexpr const char* kSysUserPermissionsTableDescription =
            "Stores information about user permissions";
    static constexpr const char* kSysUserPermissions_UserId_ColumnName = "USER_ID";
    static constexpr const char* kSysUserPermissions_UserId_ColumnDescription = "User identifier";
    static constexpr const char* kSysUserPermissions_DatabaseId_ColumnName = "DATABASE_ID";
    static constexpr const char* kSysUserPermissions_DatabaseId_ColumnDescription =
            "Database identifier";
    static constexpr const char* kSysUserPermissions_ObjectType_ColumnName = "OBJECT_TYPE";
    static constexpr const char* kSysUserPermissions_ObjectType_ColumnDescription =
            "Database object type";
    static constexpr const char* kSysUserPermissions_ObjectId_ColumnName = "OBJECT_ID";
    static constexpr const char* kSysUserPermissions_ObjectId_ColumnDescription =
            "Database object identifier";
    static constexpr const char* kSysUserPermissions_Permissions_ColumnName = "PERMISSIONS";
    static constexpr const char* kSysUserPermissions_Permissions_ColumnDescription =
            "Permission mask";
    static constexpr const char* kSysUserPermissions_GrantOptions_ColumnName = "GRANT_OPTIONS";
    static constexpr const char* kSysUserPermissions_GrantOptions_ColumnDescription =
            "Grant option mask";

    /** System database name */
    static constexpr const char* kSystemDatabaseName = "SYS";

    /** System database description */
    static constexpr const char* kSystemDatabaseDescription =
            "Stores information about other known databases, users and their permissions.";

    /** System database ID */
    static constexpr std::uint32_t kSystemDatabaseId = 1;

    /** System database creation timestamp */
    static constexpr std::time_t kSystemDatabaseCreationTime = 1;

    /** System database UUID */
    static const Uuid kSystemDatabaseUuid;

    /** Database directory prefix */
    static constexpr const char* kDatabaseDataDirPrefix = "db-";

protected:
    /**
     * Initializes object of class Database for a new database.
     * @param instance Instance object.
     * @param name Database name.
     * @param cipherId Cipher ID used for encryption of this database.
     * @param cipherKey Key used for encryption of this database.
     * @param tableCacheCapacity Table cache capacity.
     * @param description Database description.
     */
    Database(Instance& instance, std::string&& name, const std::string& cipherId,
            BinaryValue&& cipherKey, std::size_t tableCacheCapacity,
            std::optional<std::string>&& description);

    /**
     * Initializes object of class Database for an existing non-system database.
     * @param instance Instance object.
     * @param dbRecord Database record.
     * @param cipherId Cipher ID used for encryption of this database.
     * @param cipherKey Key used for encryption of this database.
     * @param tableCacheCapacity Table cache capacity.
     */
    Database(Instance& instance, const DatabaseRecord& dbRecord, std::size_t tableCacheCapacity);

public:
    /** De-initializes object of class Database */
    virtual ~Database() = default;

    DECLARE_NONCOPYABLE(Database);

    /**
     * Returns instance object.
     * @return Instance object.
     */
    Instance& getInstance() const noexcept
    {
        return m_instance;
    }

    /**
     * Gives indication that this is system database.
     * @return true if this is system database, false otherwise.
     */
    virtual bool isSystemDatabase() const noexcept;

    /**
     * Returns database ID.
     * @return Database ID.
     */
    std::uint32_t getId() const noexcept
    {
        return m_id;
    }

    /**
     * Returns database UUID.
     * @return database UUID.
     */
    const auto& getUuid() const noexcept
    {
        return m_uuid;
    }

    /**
     * Returns database name.
     * @return Database name.
     */
    const auto& getName() const noexcept
    {
        return m_name;
    }

    /**
     * Returns database description.
     * @return Database description.
     */
    const auto& getDescription() const noexcept
    {
        return m_description;
    }

    /**
     * Returns database data directory path.
     * @return data directory path.
     */
    const auto& getDataDir() const noexcept
    {
        return m_dataDir;
    }

    /**
     * Returns transaction parameters used to create this database.
     * Has effect only for newly created database.
     * @return Create database transaction parameters.
     */
    const auto& getCreateTransactionParams() const noexcept
    {
        return m_createTransactionParams;
    }

    /**
     * Returns cipher ID.
     * @return Cipher ID.
     */
    const char* getCipherId() const noexcept
    {
        return m_cipher ? m_cipher->getCipherId() : crypto::kNoCipherId;
    }

    /**
     * Returns cipher key.
     * @return Cipher key.
     */
    const auto& getCipherKey() const noexcept
    {
        return m_cipherKey;
    }

    /**
     * Returns encryption context.
     * @return Encryption context.
     */
    const auto& getEncryptionContext() const noexcept
    {
        return m_encryptionContext;
    }

    /**
     * Returns decryption context.
     * @return Decryption context.
     */
    const auto& getDecryptionContext() const noexcept
    {
        return m_decryptionContext;
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
    std::string makeDisplayCode() const
    {
        return boost::uuids::to_string(m_uuid);
    }

    /**
     * Returns number of tables in the database.
     * @return Number of tables in the database.
     */
    std::size_t getTableCount() const
    {
        std::lock_guard lock(m_mutex);
        return m_tableRegistry.size();
    }

    /**
     * Returns indication that user table can be created in this database.
     * @return true if user table can be created in this database, false otherwise.
     */
    bool canContainUserTables() const noexcept
    {
        return !isSystemDatabase() || m_instance.canCreateUserTablesInSystemDatabase();
    }

    /**
     * Returns indication that table with provided name exists.
     * @param tableName Table name.
     * @return true if table with provided name exists, false otherwise.
     */
    bool isTableExists(const std::string& tableName) const
    {
        std::lock_guard lock(m_mutex);
        return isTableExistsUnlocked(tableName);
    }

    /**
     * Returns cached table name.
     * @param tableId Table ID.
     * @return Table name or throws exception if table doesn't exist in cache.
     */
    std::string getTableName(std::uint32_t tableId) const
    {
        std::lock_guard lock(m_mutex);
        return findTableNameUnlocked(tableId);
    }

    /**
     * Creates new table object and writes all necessary on-disk data structures.
     * @param name Table name.
     * @param type Table type.
     * @param firstUserTrid First user range TRID.
     * @param description Table description.
     * @return New table object.
     */
    TablePtr createTable(std::string&& name, TableType type, std::uint64_t firstUserTrid,
            std::optional<std::string>&& description)
    {
        std::lock_guard lock(m_mutex);
        return createTableUnlocked(std::move(name), type, firstUserTrid, std::move(description));
    }

    /**
     * Returns existing table object.
     * @param tableName Table name.
     * @return Corresponding table object.
     * @throw DatabaseError if requested table doesn't exist.
     */
    TablePtr findTableChecked(const std::string& tableName);

    /**
     * Returns existing table object.
     * @param tableId Table ID.
     * @return Corresponding table object.
     * @throw DatabaseError if requested table doesn't exist.
     */
    TablePtr findTableChecked(std::uint32_t tableId);

    /**
     * Creates new constraint definition or returns suitable existing one.
     * @param system Indicated that constraint ID must be from the system range.
     * @param constraintType Constaint type.
     * @param expression Constraint expression.
     * @param[out] existing Indicates that existing constraint definition was returned.
     * @return Constraint definition object.
     */
    ConstraintDefinitionPtr createConstraintDefinition(bool system, ConstraintType constraintType,
            requests::ConstExpressionPtr&& expression, bool& existing);

    /**
     * Finds approprite constraint definition or creates new one.
     * @param system Indicates that we are looking for system constraint definiton.
     * @param type Constraint type.
     * @param serializedExpression Serialized expression.
     * @return Constraint definition object.
     */
    ConstraintDefinitionPtr findOrCreateConstraintDefinition(
            bool system, ConstraintType type, const BinaryValue& serializedExpression);

    /**
     * Returns constaint definition object if it exists.
     * @param constraintDefinitionId Constraint definition ID.
     * @return Constraint definition object.
     * @throws DatabaseError if constraint definition doesn't exist.
     */
    ConstraintDefinitionPtr findConstraintDefinitionChecked(std::uint64_t constraintDefinitionId);

    /**
     * Creates new constraint object for the given table.
     * @param table A table to which this constraint belongs.
     * @param column A column to which this constraint belongs or nullptr
     *               if this is table constraint.
     * @param name Constraint name.
     * @param constraintDefinition Constraint definition.
     * @param description Constraint description.
     * @return New constraint object.
     * @throw DatabaseError if constraint already exists.
     */
    ConstraintPtr createConstraint(Table& table, Column* column, std::string&& name,
            const ConstConstraintDefinitionPtr& constraintDefinition,
            std::optional<std::string>&& description);

    /**
     * Creates new constraint object for the given table.
     * @param table A table to which this constraint belongs.
     * @param column A column to which this constraint belongs or nullptr
     *               if this is table constraint.
     * @param constraintRecord Constaint record.
     * @return New constraint object.
     * @throw DatabaseError if constraint already exists.
     */
    ConstraintPtr createConstraint(
            Table& table, Column* column, const ConstraintRecord& constraintRecord);

    /**
     * Returns indication that constraint exists.
     * @param constraintName Constraint name.
     * @return true if constraint exists, false otherwise.
     */
    bool isConstraintExists(const std::string& constraintName) const;

    /**
     * Returns column sets with a given ID, as recorded in the SYS_COLUMNS_SETS.
     * @param columnSetId Column set ID.
     * @return Column set record.
     * @throw DatabaseError if column set record doesn't exist.
     */
    ColumnSetRecord findColumnSetRecord(std::uint64_t columnId) const;

    /**
     * Returns column record with given ID.
     * @param columnId Column ID.
     * @return Column record.
     * @throw DatabaseError if column record doesn't exist.
     */
    ColumnRecord findColumnRecord(std::uint64_t columnId) const;

    /**
     * Returns column definition record with given ID.
     * @param columnDefinitionId Column definition ID.
     * @return Column definition record.
     * @throw DatabaseError if column definition record doesn't exist.
     */
    ColumnDefinitionRecord findColumnDefinitionRecord(std::uint64_t columnDefinitionId) const;

    /**
     * Returns latest column definition ID for the given column.
     * @param tableId Table ID.
     * @param columnId Column ID.
     * @return Column definition ID.
     * @throw DatabaseError if column doesn't exist or there is no column definitions
     *                      for the specified column.
     */
    std::uint64_t findLatestColumnDefinitionIdForColumn(
            std::uint32_t tableId, std::uint64_t columnId);

    /**
     * Returns column definition constraint record with given ID.
     * @param columnDefinitionConstraintId Column definition constraint ID.
     * @return Column definition constraint record.
     * @throw DatabaseError if column definition constraint record doesn't exist.
     */
    ColumnDefinitionConstraintRecord findColumnDefinitionConstraintRecord(
            std::uint64_t columnDefinitionConstraintId) const;

    /**
     * Returns constraint record with given ID.
     * @param constraintId Constraint ID.
     * @return Constraint record.
     * @throw DatabaseError if constaint record doesn't exist.
     */
    ConstraintRecord findConstraintRecord(std::uint64_t constraintId) const;

    /**
     * Returns index record with given ID.
     * @param indexId Index ID.
     * @return Index record.
     * @throw DatabaseError if column record doesn't exist.
     */
    IndexRecord findIndexRecord(std::uint64_t indexId) const;

    /**
     * Generates new table ID.
     * @param system Indicates that ID must be in the system object ID range.
     * @return New table ID.
     */
    std::uint32_t generateNextTableId(bool system);

    /**
     * Generates new column ID.
     * @param system Indicates that ID must be in the system object ID range.
     * @return New column ID.
     */
    std::uint64_t generateNextColumnId(bool system);

    /**
     * Generates new column definition ID.
     * @param system Indicates that ID must be in the system object ID range.
     * @return New column definition ID.
     */
    std::uint64_t generateNextColumnDefinitionId(bool system);

    /**
     * Generates new column set ID.
     * @param system Indicates that resulting ID must be in the system object ID range.
     * @return New column set ID.
     */
    std::uint64_t generateNextColumnSetId(bool system);

    /**
     * Generates new column set column ID.
     * @param system Indicates that resulting ID must be in the system object ID range.
     * @return New column set column ID.
     */
    std::uint64_t generateNextColumnSetColumnId(bool system);

    /**
     * Generates new constraint definition ID.
     * @param system Indicates that resulting ID must be in the system object ID range.
     * @return New constraint definition ID.
     */
    std::uint64_t generateNextConstraintDefinitionId(bool system);

    /**
     * Generates new constraint ID.
     * @param system Indicates that resulting ID must be in the system object ID range.
     * @return New constraint ID.
     */
    std::uint64_t generateNextConstraintId(bool system);

    /**
     * Generates new column definition constraint ID.
     * @param system Indicates that resulting ID must be in the system object ID range.
     * @return New column definition constraint ID.
     */
    std::uint64_t generateNextColumnDefinitionConstraintId(bool system);

    /**
     * Generates new index ID.
     * @param system Indicates that resulting ID must be in the system object ID range.
     * @return New index ID.
     */
    std::uint64_t generateNextIndexId(bool system);

    /**
     * Generates new index column ID.
     * @param system Indicates that resulting ID must be in the system object ID range.
     * @return New index ID.
     */
    std::uint64_t generateNextIndexColumnId(bool system);

    /**
     * Generates next transaction ID.
     * @return New transaction ID.
     */
    std::uint64_t generateNextTransactionId() noexcept
    {
        return m_metadata->generateNextTransactionId();
    }

    /**
     * Generates next atomic operation ID.
     * @return New atomic operation ID.
     */
    std::uint64_t generateNextAtomicOperationId()
    {
        return m_metadata->generateNextAtomicOperationId();
    }

    /**
     * Retruns indication that given table name is reserved system table name.
     * @param tableName Table name.
     * @return true if given table name is reserved system table name, false otherwise.
     */
    static bool isSystemTable(const std::string& tableName) noexcept
    {
        return m_allSystemTables.count(tableName) > 0;
    }

    /**
     * Returns indication that the database is currently in use.
     * @return true if database is used by anyone, false otherwise.
     */
    bool isUsed() const noexcept
    {
        return m_useCount > 0;
    }

    /** Increases usage count of the database. */
    void use() noexcept
    {
        ++m_useCount;
    }

    /** Decreases usage count of the database. */
    void release();

    /**
     * Returns "NOT NULL" system constraint definition.
     * @return Constraint definition object.
     */
    ConstraintDefinitionPtr getSystemNotNullConstraintDefinition() const noexcept
    {
        return m_systemNotNullConstraintDefinition;
    }

    /**
     * Returns "DEFAULT 0" system constraint definition.
     * @return Constraint definition object.
     */
    ConstraintDefinitionPtr getSystemDefaultZeroConstraintDefinition() const noexcept
    {
        return m_systenDefaultZeroConstraintDefinition;
    }

    /**
     * Checks that constraint type matches to the required one.
     * @param table Table, to which the constaint belong.
     * @param column Column, to which the constraint belongs, nullptr for table-level constaint.
     * @param constraintName Constraint name.
     * @param constraintDefinition Constraint definition.
     * @param expectedType Expected constraint type.
     * @throw DatabaseError if constraint type doesn't match.
     */
    void checkConstraintType(const Table& table, const Column* column,
            const std::string& constraintName, const ConstraintDefinition& constraintDefinition,
            ConstraintType expectedType) const;

    /**
     * Checks that constraint type matches to the required one.
     * @param table Table, to which the constaint belong.
     * @param column Column, to which the constraint belongs, nullptr for table-level constaint.
     * @param constraintRecord Constraint record.
     * @param expectedType Expected constraint type.
     * @throw DatabaseError if constraint type doesn't match.
     */
    void checkConstraintType(const Table& table, const Column* column,
            const ConstraintRecord& constraintRecord, ConstraintType expectedType) const;

    /**
     * Register table.
     * @param table Table object.
     **/
    void registerTable(const Table& table);

    /**
     * Registers column.
     * @param column Column object.
     */
    void registerColumn(const Column& column);

    /**
     * Registers column definition.
     * @param columnDefinition Column definition object.
     */
    void registerColumnDefinition(const ColumnDefinition& columnDefinition);

    /**
     * Updates column definition data in the registry.
     * @param columnDefinition Column definition object.
     */
    void updateColumnDefinitionRegistration(const ColumnDefinition& columnDefinition);

    /**
     * Registers column set.
     * @param columnSet Column set object.
     */
    void registerColumnSet(const ColumnSet& columnSet);

    /**
     * Updates column set data in the registry.
     * @param columnSet Column set object.
     */
    void updateColumnSetRegistration(const ColumnSet& columnSet);

    /**
     * Registers constraint definition.
     * @param constraintDefinition Constraint definition object.
     */
    void registerConstraintDefinition(const ConstraintDefinition& constraintDefinition);

    /**
     * Registers constraint.
     * @param constraint Constraint object.
     */
    void registerConstraint(const Constraint& constraint);

    /**
     * Registers index.
     * @param index Index object.
     */
    void registerIndex(const Index& index);

    /**
     * Creates new user table.
     * @param name Table name.
     * @param type Table type.
     * @param columnSpecs Column definitions.
     * @param currentUserId Current user.
     * @param description Table description.
     * @return Table object
     */
    TablePtr createUserTable(std::string&& name, TableType type,
            const std::vector<SimpleColumnSpecification>& columnSpecs, std::uint32_t currentUserId,
            std::optional<std::string>&& description);

    /**
     * Creates new user table.
     * @param name Table name.
     * @param type Table type.
     * @param columnSpecs Column definitions.
     * @param currentUserId Current user.
     * @param description Table description.
     * @return Table object.
     */
    TablePtr createUserTable(std::string&& name, TableType type,
            const std::vector<ColumnSpecification>& columnSpecs, std::uint32_t currentUserId,
            std::optional<std::string>&& description);

    /**
     * Creates new file. File is created with encrypted I/O if available.
     * @param path File path.
     * @param extraFlags Additional open file flags.
     * @param createMode File creation mode.
     * @param initialSize Initial file size
     * @return File object.
     */
    io::FilePtr createFile(
            const std::string& path, int extraFlags, int createMode, off_t initialSize = 0) const;

    /**
     * Opens existing file for reading/writing.
     * File is opened with encrypted I/O if available.
     * @param path File path.
     * @param extraFlags Additional open file flags.
     * @return File object.
     */
    io::FilePtr openFile(const std::string& path, int extraFlags = 0) const;

protected:
    /**
     * Checks that table belongs to this database.
     * @param table Table to check.
     * @param operationName Name of operation that requested this check.
     * @throw DatabaseError if table belongs to a different database.
     */
    void checkTableBelongsToThisDatabase(const Table& table, const char* operationName) const;

    /**
     * Creates new table object and writes all necessary on-disk data structures.
     * @param name Table name.
     * @param type Table type.
     * @param firstUserTrid First user range TRID.
     * @param description Table description.
     * @return New table object.
     */
    TablePtr createTableUnlocked(std::string&& name, TableType type, std::uint64_t firstUserTrid,
            std::optional<std::string>&& description);

    /**
     * Loads system table object.
     * Uses system table registry as source of information. Loads it if required.
     * @param name System table name.
     * @param firstUserTrid First user range TRID.
     * @return System table object.
     * @throw DatabaseError if table doesn't exist.
     */
    TablePtr loadSystemTable(const std::string& name);

    /**
     * Records table into the appropriate system table.
     * @param table A table to be recorded.
     * @param tp Transaction parameters.
     */
    void recordTable(const Table& table, const TransactionParameters& tp);

    /**
     * Records constraint definition into the appropriate system table.
     * @param constraintDefinition A constraint definition to be recorded.
     * @param tp Transaction parameters.
     */
    void recordConstraintDefinition(
            const ConstraintDefinition& constraintDefinition, const TransactionParameters& tp);

    /**
     * Records constraint into the appropriate system table.
     * @param constraint A constraint to be recorded.
     * @param tp Transaction parameters.
     */
    void recordConstraint(const Constraint& constraint, const TransactionParameters& tp);

    /**
     * Records column set into the appropriate system table.
     * @param columnSet A column set to be recorded.
     * @param tp Transaction parameters.
     */
    void recordColumnSet(const ColumnSet& columnSet, const TransactionParameters& tp);

    /**
     * Records column set column into the appropriate system table.
     * @param columnSetColumn A column set column to be recorded.
     * @param tp Transaction parameters.
     */
    void recordColumnSetColumn(
            const ColumnSetColumn& columnSetColumn, const TransactionParameters& tp);

    /**
     * Records column into the appropriate system table.
     * @param column A column to be recorded.
     * @param tp Transaction parameters.
     */
    void recordColumn(const Column& column, const TransactionParameters& tp);

    /**
     * Records column definition into the appropriate system table.
     * @param columnDefinition A column definition to be recorded.
     * @param tp Transaction parameters.
     */
    void recordColumnDefinition(
            const ColumnDefinition& columnDefinition, const TransactionParameters& tp);

    /**
     * Records column defintion constraint into the appropriate system table.
     * @param columnDefinitionConstraint A constraint to be recorded.
     * @param tp Transaction parameters.
     */
    void recordColumnDefinitionConstraint(
            const ColumnDefinitionConstraint& columnDefinitionConstraint,
            const TransactionParameters& tp);

    /**
     * Records index and its column into the appropriate system table.
     * @param index An index to be recorded.
     * @param tp Transaction parameters.
     */
    void recordIndexAndColumns(const Index& index, const TransactionParameters& tp);

    /**
     * Records index into the appropriate system table.
     * @param index An index to be recorded.
     * @param tp Transaction parameters.
     * @return Pair of (master column record, next block IDs).
     */
    std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> recordIndex(
            const Index& index, const TransactionParameters& tp);

    /**
     * Records all index column into the appropriate system table.
     * @param index An index columns from which to be recorded.
     * @param columnIndex Index of column in the index columns list.
     * @param tp Transaction parameters.
     */
    void recordIndexColumns(const Index& index, const TransactionParameters& tp);

    /**
     * Records index column into the appropriate system table.
     * @param index An index columns from which to be recorded.
     * @param columnIndex Index of column in the index columns list.
     * @param tp Transaction parameters.
     * @return Pair of (master column record, next block IDs).
     */
    std::pair<MasterColumnRecordPtr, std::vector<std::uint64_t>> recordIndexColumn(
            const Index& index, std::size_t columnIndex, const TransactionParameters& tp);

    /**
     * Records all definition elements related to the given table.
     * @param table A table.
     * @param tp Transaction parameters.
     */
    void recordTableDefinition(const Table& table, const TransactionParameters& tp);

    /** Load information about system objects from the special file. */
    void loadSystemObjectsInfo();

    /** Save information about system tables into the special file. */
    void saveSystemObjectsInfo() const;

    /**
     * Computes unique database ID.
     * @param databaseName Database name.
     * @param createTimestamp Creation timestamp.
     * @return Unique database ID.
     */
    static Uuid computeDatabaseUuid(
            const std::string& databaseName, std::time_t createTimestamp) noexcept;

    /** Creates initialization flag file. */
    void createInitializationFlagFile() const;

private:
    /** Creates system tables in the new database. */
    void createSystemTables();

    /** Reads all tables from SYS_TABLES */
    void readAllTables();

    /** Reads all column sets from SYS_COLUMN_SETS */
    void readAllColumnSets();

    /** Reads all columns from SYS_COLUMNS */
    void readAllColumns();

    /** Reads all columns from SYS_COLUMN_DEFS */
    void readAllColumnDefs();

    /** Reads all column set columns from SYS_COLUMN_SET_COLUMNS */
    void readAllColumnSetColumns();

    /** Reads all constraints definitions from SYS_CONSTRAINT_DEFS */
    void readAllConstraintDefs();

    /** Reads all constraints from SYS_CONSTRAINTS */
    void readAllConstraints();

    /** Reads all columns from SYS_COLUMN_DEF_CONSTRAINTS */
    void readAllColumnDefConstraints();

    /** Reads all indices from SYS_INDICES. */
    void readAllIndices();

    /** Checks data consistency */
    void checkDataConsistency();

    /**
     * Creates new metadata file.
     * @param path File path.
     * @return Metadata file object.
     */
    std::unique_ptr<MemoryMappedFile> createMetadataFile(const char* path) const;

    /**
     * Opens metadata file.
     * @param path File path.
     * @return Metadata file object.
     */
    std::unique_ptr<MemoryMappedFile> openMetadataFile(const char* path) const;

    /**
     * Constructs database metadata file path.
     * @return Database metadata file path.
     */
    std::string makeMetadataFilePath() const;

    /**
     * Constructs system tables file path.
     * @return Database metadata file path.
     */
    std::string makeSystemObjectsFilePath() const;

    /**
     * Validates database name.
     * @param databaseName Database name.
     * @return The same database name.
     * @throw DatabaseError if database name is invalid.
     */
    static std::string&& validateDatabaseName(std::string&& databaseName);

    /**
     * Returns indication that table with provided name exists.
     * @param tableName Table name.
     * @return true if table with provided name exists, false otherwise.
     */
    bool isTableExistsUnlocked(const std::string& tableName) const noexcept
    {
        return m_tableRegistry.byName().count(tableName) > 0;
    }

    /**
     * Returns cached table name.
     * @param tableId Table ID.
     * @return Table name or throws exception if table doesn't exist in cache.
     */
    std::string findTableNameUnlocked(std::uint32_t tableId) const;

    /**
     * Returns existing table object. Does not acquire table cache access synchronization lock.
     * @param tableName Table name.
     * @return Corresponding table object or nullptr if it doesn't exist.
     */
    TablePtr findTableUnlocked(const std::string& tableName);

    /**
     * Returns existing table object. Does not acquire table cache access synchronization lock.
     * @param tableId Table ID.
     * @return Corresponding table object or nullptr if it doesn't exist.
     */
    TablePtr findTableUnlocked(std::uint32_t tableId);

    /**
     * Loads table from disk. Does not acquire table cache access synchronization lock.
     * @param tableRecord Table registry record.
     * @return Table object.
     */
    TablePtr loadTableUnlocked(const TableRecord& tableRecord);

    /**
     * Creates new system constraint definition or returns suitable existing one. Does not acquire
     * constraint definition cache access synchronization lock.
     * @param constraintType Constaint type.
     * @param expression Constraint expression.
     * @param[out] existing Indicates that existing constraint definition was returned.
     * @return Constraint definition object.
     */
    ConstraintDefinitionPtr createSystemConstraintDefinitionUnlocked(
            ConstraintType constraintType, requests::ConstExpressionPtr&& expression);

    /**
     * Creates new constraint definition or returns suitable existing one. Does not acquire
     * constraint definition cache access synchronization lock.
     * @param constraintType Constaint type.
     * @param expression Constraint expression.
     * @return Constraint definition object.
     */
    ConstraintDefinitionPtr createConstraintDefinitionUnlocked(bool system,
            ConstraintType constraintType, requests::ConstExpressionPtr&& expression,
            bool& existing);

    /**
     * Returns existing constraint definition object. Does not acquire constraint definition cache
     * access synchronization lock.
     * @param constraintDefinitionId Constraint definition ID.
     * @return Corresponding constraint definition object or nullptr if it doesn't exist.
     */
    ConstraintDefinitionPtr findConstraintDefinitionUnlocked(std::uint64_t constraintDefinitionId);

    /**
     * Loads constraint definition from disk. Does not acquire constraint definition cache
     * access synchronization lock.
     * @param constraintDefinitionRecord Constraint definition registry record.
     * @return Constraint definition object.
     */
    ConstraintDefinitionPtr loadConstraintDefinitionUnlocked(
            const ConstraintDefinitionRecord& constraintDefinitionRecord);

    /**
     * Ensures that data directory exists and initialized if required.
     * @param create Indicates that data directoty must be created.
     * @return Data directory path.
     */
    std::string ensureDataDir(bool create) const;

protected:
    /** Temporary TRID counters are used until appropriate table is created. */
    struct TemporaryTridCounters {
        /** Initializes object of class TemporaryTridCounters. */
        TemporaryTridCounters() noexcept
            : m_lastTableId(0)
            , m_lastColumnId(0)
            , m_lastColumnSetId(0)
            , m_lastColumnSetColumnId(0)
            , m_lastColumnDefinitionId(0)
            , m_lastConstraintDefinitionId(0)
            , m_lastConstraintId(0)
            , m_lastColumnDefinitionConstraintId(0)
            , m_lastIndexId(0)
            , m_lastIndexColumnId(0)
        {
        }

        /** Last table ID */
        std::uint64_t m_lastTableId;

        /** Last column ID */
        std::uint64_t m_lastColumnId;

        /** Last column set ID */
        std::uint64_t m_lastColumnSetId;

        /** Last column set column ID */
        std::uint64_t m_lastColumnSetColumnId;

        /** Last column definition ID */
        std::uint64_t m_lastColumnDefinitionId;

        /** Last constraint definition ID */
        std::uint64_t m_lastConstraintDefinitionId;

        /** Last constraint ID */
        std::uint64_t m_lastConstraintId;

        /** Last column definition constraint ID */
        std::uint64_t m_lastColumnDefinitionConstraintId;

        /** Last index ID */
        std::uint64_t m_lastIndexId;

        /** Last index column ID */
        std::uint64_t m_lastIndexColumnId;
    };

protected:
    /** Instance to which this database belongs */
    Instance& m_instance;

    /** Database identifier */
    const Uuid m_uuid;

    /** Database name */
    std::string m_name;

    /** Database description */
    std::optional<std::string> m_description;

    /** Database ID */
    const std::uint32_t m_id;

    /** Database data directory */
    const std::string m_dataDir;

    /** Cipher ID */
    const crypto::CipherPtr m_cipher;

    /** Cipher key */
    const BinaryValue m_cipherKey;

    /** Encryption context */
    const crypto::CipherContextPtr m_encryptionContext;

    /** Decryption context */
    const crypto::CipherContextPtr m_decryptionContext;

    /** Database internals access synchronization object */
    mutable std::recursive_mutex m_mutex;

    /* Metadata file descriptor */
    const std::unique_ptr<MemoryMappedFile> m_metadataFile;

    /** Persistent metadata (counters, etc) */
    DatabaseMetadata* m_metadata;

    /** First transaction parameters */
    const TransactionParameters m_createTransactionParams;

    /** Table registry. Contains information about all known tables. */
    TableRegistry m_tableRegistry;

    /** Column set registry. Contains information about all known column sets. */
    ColumnSetRegistry m_columnSetRegistry;

    /** Column registry. Contains information about all known columns. */
    ColumnRegistry m_columnRegistry;

    /** Column definition registry. Contains information about all known column definitions. */
    ColumnDefinitionRegistry m_columnDefinitionRegistry;

    /**
     * Constraint definition registry.
     * Contains information about all known constraint definitions.
     */
    ConstraintDefinitionRegistry m_constraintDefinitionRegistry;

    /** Constraint registry. Contains information about all known constraints. */
    ConstraintRegistry m_constraintRegistry;

    /** Index registry. Contains information about all known indices */
    IndexRegistry m_indexRegistry;

    /** Table cache. Contains recently used tables. */
    TableCache m_tableCache;

    /** Constraint definition cache. Contains recently used constaint definitions. */
    ConstraintDefinitionCache m_constraintDefinitionCache;

    /** Temporart TRID counters, used until appropriate table is created. */
    TemporaryTridCounters m_tmpTridCounters;

    /** Database use count */
    std::atomic<std::size_t> m_useCount;

    /** System table SYS_TABLES. Must go before all other tables. */
    TablePtr m_sysTablesTable;

    /** System table SYS_DUMMY */
    TablePtr m_sysDummyTable;

    /** System table SYS_COLUMN_SETS */
    TablePtr m_sysColumnSetsTable;

    /** System table SYS_COLUMNS */
    TablePtr m_sysColumnsTable;

    /** System table SYS_COLUMN_DEFS */
    TablePtr m_sysColumnDefsTable;

    /** System table SYS_COLUMN_SET_COLUMNS */
    TablePtr m_sysColumnSetColumnsTable;

    /** System table SYS_CONSTRAINT_DEFS */
    TablePtr m_sysConstraintDefsTable;

    /** System table SYS_CONSTRAINTS */
    TablePtr m_sysConstraintsTable;

    /** System table SYS_COLUMN_DEF_CONSTRAINTS */
    TablePtr m_sysColumnDefConstraintsTable;

    /** System table SYS_INDICES */
    TablePtr m_sysIndicesTable;

    /** System table SYS_INDEX_COLUMNS */
    TablePtr m_sysIndexColumnsTable;

    /** System constraint definition for the "NOT NULL" constraint */
    ConstraintDefinitionPtr m_systemNotNullConstraintDefinition;

    /** System constraint definition for the "DEFAULT 0" constraint */
    ConstraintDefinitionPtr m_systenDefaultZeroConstraintDefinition;

    /** All system table name list */
    static const std::unordered_map<std::string, std::unordered_set<std::string>> m_allSystemTables;

    /** System database specific system table name list */
    static const std::unordered_set<std::string> m_systemDatabaseOnlySystemTables;

    /** Initialization flag file name */
    static constexpr const char* kInitializationFlagFile = "initialized";

    /** Metadata file name */
    static constexpr const char* kMetadataFileName = "db_metadata";

    /** System tables file name */
    static constexpr const char* kSystemObjectsFileName = "system_objects";

    /** First transaction ID */
    static constexpr std::uint64_t kFirstTransationId = 1;

    /** Capacity of the constraint definition cache */
    static constexpr std::size_t kConstraintDefinitionCacheCapacity = 256;

    /** Capacity of the constraint cache */
    static constexpr std::size_t kConstraintCacheCapacity = 1024;
};

}  // namespace siodb::iomgr::dbengine
