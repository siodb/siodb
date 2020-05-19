# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "stl_ext"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_LIB:=siodb_common_stl_ext

CXX_SRC:= \
	internal/helpers.cpp  \
	chrono_ext.cpp  \
	ostream_ext.cpp  \
	string_ext.cpp

CXX_HDR:= \
	internal/helpers.h  \
	cstdint_ext.h  \
	chrono_ext.h  \
	ostream_ext.h  \
	string_ext.h  \
	utility_ext.h

include $(MK)/Main.mk
