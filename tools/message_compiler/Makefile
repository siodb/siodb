# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# restcli application makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../mk/Prolog.mk

COMPILER_VERSION=0.1.5

TARGET_EXE:=message_compiler

CXX_SRC:=\
	Compiler.cpp \
	CompilerOptions.cpp

DEFS+=-DCOMPILER_VERSION=\"$(COMPILER_VERSION)\"

TARGET_OWN_LIBS:=

TARGET_COMMON_LIBS:=

TARGET_LIBS:=-lboost_iostreams -lboost_system -lboost_filesystem

include $(MK)/Main.mk
