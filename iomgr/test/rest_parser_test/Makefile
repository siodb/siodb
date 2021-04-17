# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# REST Request Parser Test Makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_EXE:=rest_parser_test

CXX_SRC:= \
	RestParserTest_Get.cpp \
	RestParserTest_Delete.cpp \
	RestParserTest_Main.cpp \
	RestParserTest_Patch.cpp \
	RestParserTest_Post.cpp

CXX_HDR:=

CXXFLAGS+=-I../../lib -I$(GENERATED_FILES_ROOT)

TARGET_OWN_LIBS:=iomgr_dbengine

TARGET_COMMON_LIBS:=iomgr_shared unit_test proto data sys io utils stl_ext crt_ext

TARGET_LIBS:=-lprotobuf -lantlr4-runtime -lboost_log -lboost_system -lboost_filesystem -lxxhash

include $(MK)/Main.mk
