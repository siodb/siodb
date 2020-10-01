# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# restcli application makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../mk/Prolog.mk

GGV_VERSION=0.0.1

TARGET_EXE:=generate_version_go

CXX_SRC:=\
	GenerateGoVersion.cpp

CXX_HDR:=\
	GenerateGoVersion.h

DEFS+=-DGGV_VERSION=\"$(GGV_VERSION)\"

TARGET_OWN_LIBS:=

TARGET_COMMON_LIBS:=

TARGET_LIBS:=-lboost_iostreams

include $(MK)/Main.mk
