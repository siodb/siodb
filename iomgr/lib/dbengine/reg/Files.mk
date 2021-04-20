# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

CXX_SRC+= \
	reg/CipherKeyRecord.cpp \
	reg/ColumnRecord.cpp \
	reg/ColumnDefinitionConstraintRecord.cpp \
	reg/ColumnDefinitionRecord.cpp \
	reg/ColumnSetColumnRecord.cpp \
	reg/ColumnSetRecord.cpp \
	reg/ConstraintDefinitionRecord.cpp \
	reg/ConstraintRecord.cpp \
	reg/DatabaseRecord.cpp \
	reg/Helpers.cpp \
	reg/IndexColumnRecord.cpp \
	reg/IndexRecord.cpp \
	reg/RegistryRecordUuidChecker.cpp \
	reg/TableRecord.cpp \
	reg/UserAccessKeyRecord.cpp \
	reg/UserPermissionRecord.cpp \
	reg/UserRecord.cpp \
	reg/UserTokenRecord.cpp

CXX_HDR+= \
	reg/CipherKeyRecord.h \
	reg/ColumnDefinitionConstraintRecord.h \
	reg/ColumnDefinitionConstraintRegistry.h \
	reg/ColumnDefinitionRecord.h \
	reg/ColumnDefinitionRegistry.h \
	reg/ColumnRecord.h \
	reg/ColumnRegistry.h \
	reg/ColumnSetColumnRecord.h \
	reg/ColumnSetColumnRegistry.h \
	reg/ColumnSetRecord.h \
	reg/ColumnSetRegistry.h \
	reg/ConstraintDefinitionRecord.h \
	reg/ConstraintDefinitionRegistry.h \
	reg/ConstraintRecord.h \
	reg/ConstraintRegistry.h \
	reg/DatabaseRecord.h \
	reg/DatabaseRegistry.h \
	reg/Helpers.h \
	reg/IndexColumnRecord.h \
	reg/IndexColumnRegistry.h \
	reg/IndexRecord.h \
	reg/IndexRegistry.h \
	reg/RegistryRecordUuidChecker.h \
	reg/TableRecord.h \
	reg/TableRegistry.h \
	reg/UserAccessKeyRecord.h \
	reg/UserAccessKeyRegistry.h \
	reg/UserPermissionRecord.h \
	reg/UserPermissionRegistry.h \
	reg/UserRecord.h \
	reg/UserRegistry.h \
	reg/UserTokenRecord.h \
	reg/UserTokenRegistry.h
