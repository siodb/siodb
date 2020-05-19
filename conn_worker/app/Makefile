# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# conn_worker application makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../mk/Prolog.mk

TARGET_EXE:=siodb_conn_worker

CXX_SRC:=ConnWorkerMain.cpp

TARGET_OWN_LIBS:=conn_worker

TARGET_COMMON_LIBS:=options log net proto protobuf io sys utils stl_ext crypto

TARGET_LIBS:=-lboost_filesystem -lboost_log -lboost_thread -lboost_program_options \
		-lboost_system -lprotobuf -lantlr4-runtime -lcrypto -lssl -lxxhash

include $(MK)/Main.mk
