# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "crt_ext"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_LIB:=siodb_common_crt_ext

C_SRC:= \
	string_ext.c

CXX_HDR:= \
	ct_string.h  \

C_HDR:= \
	detail/string_ext_detail.h  \
	stdint_ext.h  \
	string_ext.h

include $(MK)/Main.mk
