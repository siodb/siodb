# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Database engine startup test makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_EXE:=dbengine_startup_test

CXX_SRC:=DBEngineStartupTest.cpp

CXXFLAGS+=-I../../lib

TARGET_OWN_LIBS:=iomgr

TARGET_COMMON_LIBS:=unit_test options log net proto protobuf io sys utils data stl_ext crt_ext crypto utils

TARGET_LIBS:=-lboost_filesystem -lboost_log -lboost_thread -lboost_program_options \
		-lboost_system -lprotobuf -lcrypto -lxxhash

include $(MK)/Main.mk
