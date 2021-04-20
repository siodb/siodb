# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# siodb library makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../mk/Prolog.mk

TARGET_LIB:=siodb

CXX_SRC:= \
	ProcessMonitor.cpp \
	Siodb.cpp \
	SiodbConnectionManager.cpp

CXX_HDR:= \
	ProcessMonitor.h \
	SiodbConnectionManager.h

include $(MK)/Main.mk
