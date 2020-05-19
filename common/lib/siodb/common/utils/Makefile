# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "utils"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_LIB:=siodb_common_utils

CXX_SRC:= \
	CheckOSUser.cpp  \
	EmptyString.cpp  \
	ErrorCodeChecker.cpp  \
	FsUtils.cpp \
	MessageCatalog.cpp  \
	SignalHandlers.cpp  \
	StartupActions.cpp  \
	SiodbException.cpp  \
	StringScanner.cpp  \
	StringUtils.cpp  \
	SystemError.cpp  \
	Uuid.cpp \
	Utf8String.cpp  \
	Base128VariantEncodingCxx.cpp

CXX_HDR:= \
	Align.h  \
	BinaryValue.h  \
	Bitmask.h  \
	ByteOrder.h  \
	CheckOSUser.h  \
	ConcurrentQueue.h  \
	Constants.h  \
	ComparisonMacros.h  \
	ComparisonOperations.h  \
	ContainerUtils.h  \
	Debug.h  \
	DeserializationError.h  \
	EmptyString.h  \
	ErrorCodeChecker.h  \
	FileDescriptorGuard.h  \
	Format.h  \
	FsUtils.h  \
	HelperMacros.h  \
	LruCache.h  \
	MemoryBuffer.h  \
	MessageCatalog.h  \
	MutableOrConstantString.h  \
	OrderedLruCache.h  \
	SignalHandlers.h  \
	SiodbException.h  \
	StartupActions.h  \
	StringBuilder.h  \
	StringScanner.h  \
	StringUtils.h  \
	SystemError.h  \
	UnorderedLruCache.h  \
	Uuid.h  \
	Utf8String.h  \
	internal/SignalHandlersInternal.h

C_SRC:= \
	Base128VariantEncoding.c  \
	PlainBinaryEncoding.c

C_HDR:= \
	Base128VariantEncoding.h  \
	PlainBinaryEncoding.h

include $(MK)/Main.mk
