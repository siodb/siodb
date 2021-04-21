# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# siocli library makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../mk/Prolog.mk

TARGET_LIB:=siocli

CXX_SRC:= \
	Siocli.cpp \
	SqlClient.cpp \
	SqlDump.cpp

CXX_HDR:= \
	internal/SqlClientInternal.h \
	internal/SqlDumpInternal.h \
	Siocli.h \
	SqlClient.h \
	SqlDump.h \
	SqlQueryException.h

include $(MK)/Main.mk
