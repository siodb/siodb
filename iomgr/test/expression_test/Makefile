# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Expression Test Makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_EXE:=expression_test

CXX_SRC:= \
	ExpressionSerializationTest.cpp \
	ExpressionTest_Arithmetic.cpp \
	ExpressionTest_Between.cpp \
	ExpressionTest_Bitwise.cpp \
	ExpressionTest_Column.cpp \
	ExpressionTest_Comparisons.cpp \
	ExpressionTest_Concat.cpp \
	ExpressionTest_Const.cpp \
	ExpressionTest_In.cpp \
	ExpressionTest_Is.cpp \
	ExpressionTest_Like.cpp \
	ExpressionTest_Logical.cpp \
	ExpressionTest_Main.cpp \
	ExpressionTest_Serialization_Arithmetic.cpp \
	ExpressionTest_Serialization_Bitwise.cpp \
	ExpressionTest_Serialization_Comparisons.cpp \
	ExpressionTest_Serialization_Complex.cpp \
	ExpressionTest_Serialization_Constant.cpp \
	ExpressionTest_Serialization_Logical.cpp \
	ExpressionTest_Serialization_Other.cpp \
	TestContext.cpp

CXX_HDR:= \
	ExpressionFactories.h \
	ExpressionSerializationTest.h \
	TestContext.h

CXXFLAGS+=-I../../lib

TARGET_OWN_LIBS:=iomgr_dbengine

TARGET_COMMON_LIBS:=iomgr_shared unit_test proto utils data sys stl_ext crt_ext

include $(MK)/Main.mk
