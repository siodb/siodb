// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

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
static constexpr const char* kSysDatabases_Description_ColumnName = "DESCRIPTION";
static constexpr const char* kSysDatabases_Description_ColumnDescription = "Database description";

/** Table SYS_TABLES */
static constexpr const char* kSysTablesTableName = "SYS_TABLES";
static constexpr const char* kSysTablesTableDescription = "Stores information about known tables";
static constexpr const char* kSysTables_Type_ColumnName = "TYPE";
static constexpr const char* kSysTables_Type_ColumnDescription = "Table storage type";
static constexpr const char* kSysTables_Name_ColumnName = "NAME";
static constexpr const char* kSysTables_Name_ColumnDescription = "Table name";
static constexpr const char* kSysTables_FirstUserTrid_ColumnName = "FIRST_USER_TRID";
static constexpr const char* kSysTables_FirstUserTrid_ColumnDescription =
        "First user record row identifier";
static constexpr const char* kSysTables_CurrentColumnSetId_ColumnName = "CURRENT_COLUMN_SET_ID";
static constexpr const char* kSysTables_CurrentColumnSetId_ColumnDescription = "Current column set";
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
static constexpr const char* kSysColumnsTableDescription = "Stores information about table columns";
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
static constexpr const char* kSysColumnSetColumns_ColumnDefinitionId_ColumnName = "COLUMN_DEF_ID";
static constexpr const char* kSysColumnSetColumns_ColumnDefinitionId_ColumnDescription =
        "Associated column definition identifier";

/** Table SYS_CONSTRAINT_DEFS */
static constexpr const char* kSysConstraintDefsTableName = "SYS_CONSTRAINT_DEFS";
static constexpr const char* kSysConstraintDefsTableDescription =
        "Stores information about unique constraint definitions";
static constexpr const char* kSysConstraintDefs_Type_ColumnName = "TYPE";
static constexpr const char* kSysConstraintDefs_Type_ColumnDescription = "Constraint type";
static constexpr const char* kSysConstraintDefs_Expr_ColumnName = "EXPR";
static constexpr const char* kSysConstraintDefs_Expr_ColumnDescription = "Constraint expression";

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

/** Table SYS_USERS */
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

/** Table SYS_USER_ACCESS_KEYS */
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

/** Table SYS_USER_TOKENS */
static constexpr const char* kSysUserTokensTableName = "SYS_USER_TOKENS";
static constexpr const char* kSysUserTokensTableDescription = "Stores authentication tokens";
static constexpr const char* kSysUserTokens_UserId_ColumnName = "USER_ID";
static constexpr const char* kSysUserTokens_UserId_ColumnDescription = "User identifier";
static constexpr const char* kSysUserTokens_Name_ColumnName = "NAME";
static constexpr const char* kSysUserTokens_Name_ColumnDescription = "Token name";
static constexpr const char* kSysUserTokens_Value_ColumnName = "VALUE";
static constexpr const char* kSysUserTokens_Value_ColumnDescription = "Token value";
static constexpr const char* kSysUserTokens_ExpirationTimestamp_ColumnName = "EXPIRATION_TIMESTAMP";
static constexpr const char* kSysUserTokens_ExpirationTimestamp_ColumnDescription =
        "Token expiration timestamp";
static constexpr const char* kSysUserTokens_Description_ColumnName = "DESCRIPTION";
static constexpr const char* kSysUserTokens_Description_ColumnDescription = "Token description";

/** Table SYS_USER_PERMISSIONS */
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
static constexpr const char* kSysUserPermissions_Permissions_ColumnDescription = "Permission mask";
static constexpr const char* kSysUserPermissions_GrantOptions_ColumnName = "GRANT_OPTIONS";
static constexpr const char* kSysUserPermissions_GrantOptions_ColumnDescription =
        "Grant option mask";

/** System database name */
static constexpr const char* kSystemDatabaseName = "SYS";

/** System database description */
static constexpr const char* kSystemDatabaseDescription =
        "Stores information about other known databases, users and their permissions.";

}  // namespace siodb::iomgr::dbengine
