# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# IO manager library makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_LIB:=siodb_common_iomgr_shared

CXX_SRC:= \
	dbengine/crypto/ciphers/AesCipher.cpp  \
	dbengine/crypto/ciphers/AesCipherContext.cpp  \
	dbengine/crypto/ciphers/CamelliaCipher.cpp  \
	dbengine/crypto/ciphers/CamelliaCipherContext.cpp  \
	dbengine/crypto/ciphers/Cipher.cpp  \
	dbengine/crypto/KeyGenerator.cpp  \
	\
	dbengine/io/EncryptedFile.cpp  \
	dbengine/io/File.cpp  \
	dbengine/io/NormalFile.cpp  \
	\
	dbengine/lob/BinaryValueBlobStream.cpp  \
	dbengine/lob/BlobStream.cpp  \
	dbengine/lob/BlobWrapperClobStream.cpp  \
	dbengine/lob/ClobStream.cpp  \
	dbengine/lob/ClobWrapperBlobStream.cpp  \
	dbengine/lob/StringClobStream.cpp  \
	\
	dbengine/ColumnDataType.cpp  \
	dbengine/ConstraintType.cpp  \
	dbengine/DatabaseObjectName.cpp  \
	dbengine/Variant_Arithmetics.cpp  \
	dbengine/Variant_Common.cpp  \
	dbengine/Variant_Error.cpp  \
	dbengine/Variant_Init.cpp  \
	dbengine/Variant_IO.cpp  \
	dbengine/Variant_BitwiseArithmetics.cpp  \
	dbengine/Variant_Comparisons.cpp  \
	dbengine/Variant_Conversions.cpp  \
	dbengine/Variant_Serialization.cpp  \
	dbengine/Variant_Type.cpp

CXX_HDR:= \
	dbengine/crypto/ciphers/AesCipher.h  \
	dbengine/crypto/ciphers/AesCipherContext.h  \
	dbengine/crypto/ciphers/CamelliaCipher.h  \
	dbengine/crypto/ciphers/CamelliaCipherContext.h  \
	dbengine/crypto/ciphers/Cipher.h  \
	dbengine/crypto/KeyGenerator.h  \
	\
	dbengine/io/EncryptedFile.h  \
	dbengine/io/File.h  \
	dbengine/io/NormalFile.h  \
	\
	dbengine/lob/BinaryValueBlobStream.h  \
	dbengine/lob/BlobStream.h  \
	dbengine/lob/BlobWrapperClobStream.h  \
	dbengine/lob/ClobStream.h  \
	dbengine/lob/ClobWrapperBlobStream.h  \
	dbengine/lob/LobStream.h  \
	dbengine/lob/StringClobStream.h  \
	\
	dbengine/ColumnDataType.h  \
	dbengine/ColumnState.h  \
	dbengine/ConstraintState.h  \
	dbengine/ConstraintType.h  \
	dbengine/DatabaseObjectName.h  \
	dbengine/DatabaseObjectType.h  \
	dbengine/DmlOperationType.h  \
	dbengine/IndexType.h  \
	dbengine/PermissionType.h  \
	dbengine/TableType.h  \
	dbengine/Variant.h  \
	dbengine/Variant_Error.h  \
	dbengine/Variant_Type.h  \
	dbengine/SystemObjectNames.h \
	\
	IOManagerExitCode.h

CXXFLAGS+=-I$(GENERATED_FILES_ROOT)

include $(MK)/Main.mk
