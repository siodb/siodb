# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "options"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_COMMON_LIB:=siodb_common_options

CXX_SRC:= \
	SiodbInstance.cpp \
	SiodbOptions.cpp

CXX_HDR:= \
	InvalidConfigurationError.h \
	LogOptions.h \
	SiodbInstance.h \
	SiodbOptions.h

include $(MK)/Main.mk
