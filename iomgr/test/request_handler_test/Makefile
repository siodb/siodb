# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Request handler test makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_EXE:=request_handler_test

CXX_SRC:= \
	RequestHandlerTest_DDL.cpp \
	RequestHandlerTest_DML_Complex.cpp \
	RequestHandlerTest_DML_Delete.cpp \
	RequestHandlerTest_DML_Insert.cpp \
	RequestHandlerTest_DML_Update.cpp \
	RequestHandlerTest_Main.cpp \
	RequestHandlerTest_Query_Select.cpp \
	RequestHandlerTest_Query_Select_MutliTable.cpp \
	RequestHandlerTest_Query_Select_Limits.cpp \
	RequestHandlerTest_Query_Show.cpp \
	RequestHandlerTest_RestComplex.cpp \
	RequestHandlerTest_RestDelete.cpp \
	RequestHandlerTest_RestGet.cpp \
	RequestHandlerTest_RestPatch.cpp \
	RequestHandlerTest_RestPost.cpp \
	RequestHandlerTest_TestEnv.cpp \
	RequestHandlerTest_UM.cpp

CXX_HDR:= \
	RequestHandlerTest_TestEnv.h

CXXFLAGS+=-I../../lib -I$(GENERATED_FILES_ROOT)

TARGET_OWN_LIBS:=iomgr_dbengine

TARGET_COMMON_LIBS:=iomgr_shared unit_test crypto options log net proto protobuf io sys \
	utils data stl_ext crt_ext

TARGET_LIBS:=-lboost_filesystem -lboost_log -lboost_thread -lboost_program_options \
		-lboost_system -lprotobuf -lcrypto -lantlr4-runtime -lxxhash

include $(MK)/Main.mk
