# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# conn_worker library makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../mk/Prolog.mk

TARGET_LIB:=conn_worker

CXX_SRC:= \
	ConnWorker.cpp  \
	ConnWorkerConnectionHandler.cpp

CXX_HDR:= \
	ConnWorker.h  \
	ConnWorkerConnectionHandler.h 

include $(MK)/Main.mk
