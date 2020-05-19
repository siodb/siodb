# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Key Generator Test Makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_EXE:=key_generator_test

CXX_SRC:=KeyGeneratorTest.cpp

CXXFLAGS+=-I../../lib

TARGET_OWN_LIBS:=iomgr

TARGET_COMMON_LIBS:=unit_test proto utils data stl_ext crt_ext io

TARGET_LIBS:=-lcrypto

include $(MK)/Main.mk
