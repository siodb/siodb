# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Datetime tests makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../mk/Prolog.mk

TARGET_EXE:=digital_signature_key_test

CXX_SRC:= \
	Main.cpp \
	DigitalSignatureKeyTest.cpp

CXXFLAGS+=-I../../lib

TARGET_COMMON_LIBS:=unit_test crypto stl_ext crt_ext utils io

TARGET_LIBS:=-lcrypto


include $(MK)/Main.mk
