# Copyright (C) 2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Request handler test makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_EXE:=rh1_test

CXX_SRC:= \
	RequestHandlerTest_DDL_176_RH.cpp \
	RequestHandlerTest_DummyTest.cpp \
	RequestHandlerTest_Main.cpp \
	RequestHandlerTest_TestEnv.cpp

CXX_HDR:= \
	RequestHandlerTest_TestEnv.h

CXXFLAGS+=-I../../lib -I$(GENERATED_FILES_ROOT)

TARGET_OWN_LIBS:=iomgr_dbengine

TARGET_COMMON_LIBS:=iomgr_shared unit_test crypto options log net proto protobuf io sys \
	utils data stl_ext crt_ext

TARGET_LIBS:=-lboost_filesystem -lboost_log -lboost_thread -lboost_program_options \
		-lboost_system -lprotobuf -lcrypto -lantlr4-runtime -lxxhash

include $(MK)/Main.mk
