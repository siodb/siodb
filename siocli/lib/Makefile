# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# siocli library makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../mk/Prolog.mk

TARGET_LIB:=siocli

CXX_SRC:= \
	Client.cpp  \
	Siocli.cpp \
	SqlDump.cpp

CXX_HDR:= \
	Client.h  \
	Siocli.h  \
	internal/ClientInternal.h  \
	SqlDump.h  \
	SqlQueryException.h

include $(MK)/Main.mk
