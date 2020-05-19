# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# LRU Cache Test Makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../mk/Prolog.mk

TARGET_EXE:=string_scanner_test

CXX_SRC:=StringScannerTest.cpp

CXXFLAGS+=-I../../lib

TARGET_COMMON_LIBS:=unit_test utils

TARGET_LIBS:=

include $(MK)/Main.mk
