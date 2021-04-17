# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "utils"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_COMMON_LIB:=siodb_common_utils

CXX_SRC:= \
	CheckOSUser.cpp \
	DebugUtils.cpp \
	EmptyString.cpp \
	ErrorCodeChecker.cpp \
	FSUtils.cpp \
	MessageCatalog.cpp \
	RandomUtils.cpp \
	SignalHandlers.cpp \
	StartupActions.cpp \
	SiodbException.cpp \
	StringScanner.cpp \
	StringUtils.cpp \
	Uuid.cpp \
	Utf8String.cpp \
	Base128VariantEncodingCxx.cpp

CXX_HDR:= \
	detail/SignalHandlersDetail.h \
	Align.h \
	BinaryValue.h \
	ByteOrder.h \
	CheckOSUser.h \
	Constants.h \
	ComparisonMacros.h \
	ComparisonOperations.h \
	DebugMacros.h \
	DebugUtils.h \
	DeserializationError.h \
	EmptyString.h \
	ErrorCodeChecker.h \
	FDGuard.h \
	Format.h \
	FSUtils.h \
	HelperMacros.h \
	MessageCatalog.h \
	MutableOrConstantString.h \
	RandomUtils.h \
	SignalHandlers.h \
	SiodbException.h \
	StartupActions.h \
	StringScanner.h \
	StringUtils.h \
	Uuid.h \
	Utf8String.h

C_SRC:= \
	Base128VariantEncoding.c \
	PlainBinaryEncoding.c

C_HDR:= \
	Base128VariantEncoding.h \
	PlainBinaryEncoding.h

include $(MK)/Main.mk
