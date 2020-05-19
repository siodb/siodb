# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "options"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_LIB:=siodb_common_options

CXX_SRC:= \
	DatabaseInstance.cpp  \
	DatabaseInstanceSocket.cpp  \
	InstanceOptions.cpp

CXX_HDR:= \
	DatabaseInstance.h  \
	DatabaseInstanceSocket.h  \
	InstanceOptions.h  \
	InvalidConfigurationOptionError.h  \
	LogOptions.h  \
	internal/DatabaseOptionsInternal.h

include $(MK)/Main.mk
