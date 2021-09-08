# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

include bpt/Files.mk
include crypto/Files.mk
include handlers/Files.mk
include ikt/Files.mk
include lob/Files.mk
include parser/Files.mk
include reg/Files.mk
include uli/Files.mk

CXX_SRC+= \
	BlockRegistry.cpp \
	Column_DataBlockManagement.cpp \
	Column_DataIO.cpp \
	Column_General.cpp \
	Column_MasterColumnData.cpp \
	ColumnConstraint.cpp \
	ColumnDataAddress.cpp \
	ColumnDataBlock.cpp \
	ColumnDataBlockCache.cpp \
	ColumnDataBlockHeader.cpp \
	ColumnDataRecord.cpp \
	ColumnDefinition.cpp \
	ColumnDefinitionCache.cpp \
	ColumnDefinitionConstraint.cpp \
	ColumnSet.cpp \
	ColumnSetColumn.cpp \
	ColumnSpecification.cpp \
	Constraint.cpp \
	ConstraintDefinition.cpp \
	DataSet.cpp \
	DatabaseMetadata.cpp \
	Database_Common.cpp \
	Database_Init.cpp \
	Database_ReadObjects1.cpp \
	Database_ReadObjects2.cpp \
	Database_RecordObjects.cpp \
	Database_SysTablesIO.cpp \
	Index.cpp \
	IndexColumn.cpp \
	IndexFileHeaderBase.cpp \
	Instance_Auth.cpp \
	Instance_DB.cpp \
	Instance_General.cpp \
	Instance_User.cpp \
	Instance_UserAccessKey.cpp \
	Instance_UserPermissions.cpp \
	Instance_UserToken.cpp \
	LobChunkHeader.cpp \
	MasterColumnRecord.cpp \
	NotNullConstraint.cpp \
	SystemDatabase_Common.cpp \
	SystemDatabase_Init.cpp \
	SystemDatabase_ReadObjects.cpp \
	SystemDatabase_RecordObjects.cpp \
	Table.cpp \
	TableColumns.cpp \
	TableDataSet.cpp \
	TransactionParameters.cpp \
	User.cpp \
	UserAccessKey.cpp \
	UserDatabase.cpp \
	UserPermission.cpp \
	UserToken.cpp

CXX_HDR+= \
	AuthenticationResult.h \
	BlockRegistry.h \
	ClientSession.h \
	Column.h \
	ColumnConstraint.h \
	ColumnPtr.h \
	ColumnDataAddress.h \
	ColumnDataBlock.h \
	ColumnDataBlockHeader.h \
	ColumnDataBlockCache.h \
	ColumnDataBlockPtr.h \
	ColumnDataBlockState.h \
	ColumnDataRecord.h \
	ColumnDefinition.h \
	ColumnDefinitionCache.h \
	ColumnDefinitionConstraint.h \
	ColumnDefinitionConstraintList.h \
	ColumnDefinitionConstraintListPtr.h \
	ColumnDefinitionConstraintPtr.h \
	ColumnDefinitionPtr.h \
	ColumnSet.h \
	ColumnSetCache.h \
	ColumnSetColumn.h \
	ColumnSetColumnPtr.h \
	ColumnSetPtr.h \
	ColumnSpecification.h \
	Constraint.h \
	ConstraintDefinition.h \
	ConstraintDefinitionPtr.h \
	ConstraintPtr.h \
	DBEngineDebug.h \
	DataSet.h \
	Database.h \
	DatabaseError.h \
	DatabaseMetadata.h \
	DatabasePtr.h \
	DefaultValueConstraint.h \
	DeleteRowResult.h \
	FirstUserObjectId.h \
	Index.h \
	IndexColumn.h \
	IndexColumnPtr.h \
	IndexColumnSpecification.h \
	IndexFileHeaderBase.h \
	IndexPtr.h \
	InsertRowResult.h \
	Instance.h \
	InstancePtr.h \
	LobChunkHeader.h \
	MasterColumnRecord.h \
	NotNullConstraint.h \
	SessionGuard.h \
	SimpleColumnSpecification.h \
	SystemDatabase.h \
	Table.h \
	TableDataSet.h \
	TablePtr.h \
	ThrowDatabaseError.h \
	TransactionParameters.h \
	UpdateRowResult.h \
	UpdateUserAccessKeyParameters.h \
	UpdateDatabaseParameters.h \
	UpdateUserParameters.h \
	UpdateUserTokenParameters.h \
	User.h \
	UserAccessKey.h \
	UserAccessKeyPtr.h \
	UserDatabase.h \
	UserIdGenerator.h \
	UserPermission.h \
	UserPtr.h \
	UserToken.h \
	UserTokenPtr.h
