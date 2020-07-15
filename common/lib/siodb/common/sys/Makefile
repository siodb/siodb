# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "sys"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_COMMON_LIB:=siodb_common_sys

C_SRC:= \
	Syscalls.c

CXX_HDR:= \
	PosixSemaphore.h

C_HDR:= \
	Syscalls.h

include $(MK)/Main.mk
