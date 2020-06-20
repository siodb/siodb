# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# SQL Parser Test Makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_EXE:=sql_parser_test

CXX_SRC:= \
	SqlParserTest_DDL.cpp  \
	SqlParserTest_DML.cpp  \
	SqlParserTest_General.cpp  \
	SqlParserTest_Main.cpp  \
	SqlParserTest_Other.cpp  \
	SqlParserTest_Query.cpp  \
	SqlParserTest_QueryConstExpr.cpp  \
	SqlParserTest_TC.cpp  \
	SqlParserTest_UM.cpp  \
	TestContext.cpp

CXX_HDR:= \
	TestContext.h

CXXFLAGS+=-I../../lib -I$(GENERATED_FILES_ROOT)

TARGET_OWN_LIBS:=iomgr

TARGET_COMMON_LIBS:=iomgr_shared unit_test proto utils data sys stl_ext crt_ext

TARGET_LIBS:=-lprotobuf -lantlr4-runtime -lboost_log -lboost_system -lboost_filesystem -lxxhash

include $(MK)/Main.mk
