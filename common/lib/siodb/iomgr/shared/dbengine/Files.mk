# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

include dbengine/crypto/Files.mk
include dbengine/io/Files.mk
include dbengine/lob/Files.mk
include dbengine/parser/Files.mk
include dbengine/util/Files.mk

CXX_SRC+= \
	dbengine/ColumnDataType.cpp \
	dbengine/ConstraintType.cpp \
	dbengine/DatabaseObjectName.cpp \
	dbengine/Variant_Arithmetics.cpp \
	dbengine/Variant_Common.cpp \
	dbengine/Variant_Error.cpp \
	dbengine/Variant_Init.cpp \
	dbengine/Variant_IO.cpp \
	dbengine/Variant_BitwiseArithmetics.cpp \
	dbengine/Variant_Comparisons.cpp \
	dbengine/Variant_Conversions.cpp \
	dbengine/Variant_Serialization.cpp \
	dbengine/Variant_Type.cpp

CXX_HDR+= \
	dbengine/ColumnDataType.h \
	dbengine/ColumnState.h \
	dbengine/ConstraintState.h \
	dbengine/ConstraintType.h \
	dbengine/DatabaseObjectName.h \
	dbengine/DatabaseObjectType.h \
	dbengine/DmlOperationType.h \
	dbengine/IndexType.h \
	dbengine/PermissionType.h \
	dbengine/TableType.h \
	dbengine/Variant.h \
	dbengine/Variant_Error.h \
	dbengine/Variant_Type.h \
	dbengine/SystemObjectNames.h
