# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# siodb application makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../mk/Prolog.mk

TARGET_EXE:=siodb

CXX_SRC:=SiodbMain.cpp

TARGET_OWN_LIBS:=siodb

TARGET_COMMON_LIBS:=options log net proto protobuf io sys utils stl_ext

TARGET_LIBS:=-lboost_filesystem -lboost_log -lboost_thread -lboost_program_options \
		-lboost_system -lprotobuf -lcrypt

include $(MK)/Main.mk
