# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Request handler test makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_EXE:=encrypted_file_test

CXX_SRC:=EncryptedFileTest.cpp

CXXFLAGS+=-I../../lib

TARGET_OWN_LIBS:=iomgr

TARGET_COMMON_LIBS:=unit_test io sys utils data stl_ext crt_ext

TARGET_LIBS:= -lcrypto -lboost_filesystem -lboost_system

include $(MK)/Main.mk
