# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Encoding/Decoding tests makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../mk/Prolog.mk

TARGET_EXE:=plain_binary_encoding_test

CXX_SRC:=PlainBinaryEncodingTest.cpp

CXXFLAGS+=-I../../lib

TARGET_COMMON_LIBS:=unit_test utils

TARGET_LIBS:=

ifeq ("$(USE_PCH)","1")
TARGET_LIBS+=-lboost_system
endif

include $(MK)/Main.mk
