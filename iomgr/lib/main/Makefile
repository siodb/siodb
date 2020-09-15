# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# IO manager library makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_LIB:=iomgr_main

CXX_SRC:= \
	IOManagerConnectionHandler.cpp  \
	IOManagerConnectionManager.cpp  \
	IOManagerMain.cpp  \
	IOManagerRequest.cpp  \
	IOManagerRequestDispatcher.cpp  \
	IOManagerRequestExecutor.cpp  \
	IOManagerRequestHandlerBase.cpp  \
	IOManagerRestConnectionHandler.cpp  \
	IOManagerRestConnectionHandlerFactory.cpp  \
	IOManagerSqlConnectionHandler.cpp  \
	IOManagerSqlConnectionHandlerFactory.cpp

CXX_HDR:= \
	ClientSession.h  \
	IOManagerConnectionHandler.h  \
	IOManagerConnectionHandlerFactory.h  \
	IOManagerConnectionManager.h  \
	IOManagerRequest.h  \
	IOManagerRequestPtr.h  \
	IOManagerRequestDispatcher.h  \
	IOManagerRequestExecutor.h  \
	IOManagerRequestHandlerBase.h  \
	IOManagerRestConnectionHandler.h  \
	IOManagerRestConnectionHandlerFactory.h  \
	IOManagerSqlConnectionHandler.h  \
	IOManagerSqlConnectionHandlerFactory.h

CXXFLAGS+=-I$(GENERATED_FILES_ROOT)

include $(MK)/Main.mk

