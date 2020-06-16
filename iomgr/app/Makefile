# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# iomgr application makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../mk/Prolog.mk

TARGET_EXE:=siodb_iomgr

CXX_SRC:=IOManagerMain.cpp

TARGET_OWN_LIBS:=iomgr

TARGET_COMMON_LIBS:=iomgr_shared crypto options log net proto protobuf sys utils data io stl_ext crt_ext

TARGET_LIBS:=-lboost_filesystem -lboost_log -lboost_thread -lboost_program_options \
		-lboost_system -lprotobuf -lcrypto -lantlr4-runtime -lxxhash

BIN_FILES:=iomgr_messages.txt

include $(MK)/Main.mk

iomgr_messages.txt: ../lib/messages/iomgr.msg
	@echo SIODBMC $@
	@$(SIODBMC) -T -VM -i $< -o $@
