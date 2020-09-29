# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
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
	BlockRegistry.cpp  \
	Column.cpp  \
	ColumnConstraint.cpp  \
	ColumnDataAddress.cpp  \
	ColumnDataBlock.cpp  \
	ColumnDataBlockCache.cpp  \
	ColumnDataBlockHeader.cpp  \
	ColumnDataRecord.cpp  \
	ColumnDefinition.cpp  \
	ColumnDefinitionCache.cpp  \
	ColumnDefinitionConstraint.cpp  \
	ColumnSet.cpp  \
	ColumnSetColumn.cpp  \
	ColumnSpecification.cpp  \
	Constraint.cpp  \
	ConstraintCache.cpp  \
	ConstraintDefinition.cpp  \
	ConstraintDefinitionCache.cpp  \
	DataSet.cpp  \
	DatabaseCache.cpp  \
	DatabaseMetadata.cpp  \
	Database_Common.cpp  \
	Database_Init.cpp  \
	Database_ReadObjects.cpp  \
	Database_RecordObjects.cpp  \
	Database_SysTablesIO.cpp  \
	Index.cpp  \
	IndexColumn.cpp  \
	IndexFileHeaderBase.cpp  \
	Instance.cpp  \
	LobChunkHeader.cpp  \
	MasterColumnRecord.cpp  \
	NotNullConstraint.cpp  \
	SystemDatabase_Common.cpp  \
	SystemDatabase_Init.cpp  \
	SystemDatabase_ReadObjects.cpp  \
	SystemDatabase_RecordObjects.cpp  \
	Table.cpp  \
	TableCache.cpp  \
	TableColumns.cpp  \
	TableDataSet.cpp  \
	TransactionParameters.cpp  \
	User.cpp  \
	UserAccessKey.cpp  \
	UserCache.cpp  \
	UserDatabase.cpp  \
	UserPermission.cpp  \
	UserToken.cpp

CXX_HDR+= \
	AuthenticationResult.h  \
	BlockRegistry.h  \
	ClientSession.h  \
	Column.h  \
	ColumnConstraint.h  \
	ColumnPtr.h  \
	ColumnDataAddress.h  \
	ColumnDataBlock.h  \
	ColumnDataBlockHeader.h  \
	ColumnDataBlockCache.h  \
	ColumnDataBlockPtr.h  \
	ColumnDataBlockState.h  \
	ColumnDataRecord.h  \
	ColumnDefinition.h  \
	ColumnDefinitionCache.h  \
	ColumnDefinitionConstraint.h  \
	ColumnDefinitionConstraintList.h  \
	ColumnDefinitionConstraintListPtr.h  \
	ColumnDefinitionConstraintPtr.h  \
	ColumnDefinitionPtr.h  \
	ColumnSet.h  \
	ColumnSetCache.h  \
	ColumnSetColumn.h  \
	ColumnSetColumnPtr.h  \
	ColumnSetPtr.h  \
	ColumnSpecification.h  \
	Constraint.h  \
	ConstraintCache.h  \
	ConstraintDefinition.h  \
	ConstraintDefinitionCache.h  \
	ConstraintDefinitionPtr.h  \
	ConstraintPtr.h  \
	DataSet.h  \
	Database.h  \
	DatabaseCache.h  \
	DatabaseError.h  \
	DatabaseMetadata.h  \
	DatabasePtr.h  \
	Debug.h  \
	DefaultValueConstraint.h  \
	FirstUserObjectId.h  \
	Index.h  \
	IndexColumn.h  \
	IndexColumnPtr.h  \
	IndexColumnSpecification.h  \
	IndexFileHeaderBase.h  \
	IndexPtr.h  \
	Instance.h  \
	InstancePtr.h  \
	LobChunkHeader.h  \
	MasterColumnRecord.h  \
	NotNullConstraint.h  \
	SessionGuard.h  \
	SimpleColumnSpecification.h  \
	SystemDatabase.h  \
	Table.h  \
	TableCache.h  \
	TableDataSet.h  \
	TablePtr.h  \
	ThrowDatabaseError.h  \
	TransactionParameters.h  \
	UpdateUserAccessKeyParameters.h  \
	UpdateDatabaseParameters.h  \
	UpdateUserParameters.h  \
	UpdateUserTokenParameters.h  \
	User.h  \
	UserAccessKey.h  \
	UserAccessKeyPtr.h  \
	UserCache.h  \
	UserDatabase.h  \
	UserPermission.h  \
	UserPtr.h  \
	UserToken.h  \
	UserTokenPtr.h
