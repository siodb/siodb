# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Variant Test Makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_EXE:=variant_test

CXX_SRC:= \
	VariantTest_Main.cpp  \
	VariantTest_Serialization.cpp

# CXX_HDR:=

CXXFLAGS+=-I../../lib

TARGET_OWN_LIBS:=iomgr

TARGET_COMMON_LIBS:=unit_test proto utils data sys stl_ext crt_ext

include $(MK)/Main.mk
