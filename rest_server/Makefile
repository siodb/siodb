# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# rest_server application makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../mk/Prolog.mk

TARGET_GO_EXE:=siodb_rest_server

VERSION_GO_FILE:=Version.go

GO_SRC:=\
	$(VERSION_GO_FILE)  \
	Authentication.go  \
	Helper.go  \
	IOMgrConnPool.go  \
	IOMgrConnection.go  \
	Main.go  \
	RequestHandlerDelete.go  \
	RequestHandlerGet.go  \
	RequestHandlerPatch.go  \
	RequestHandlerPost.go  \
	RestServerConfig.go  \
	RestWorker.go  \
	SiodbConfig.go  \
	SiodbLogger.go  \
	SiodbLoggerPool.go

include $(MK)/Main.mk

$(VERSION_GO_FILE): ../common/lib/siodb/common/config/SiodbVersion.h $(GENERATE_VERSION_GO_TOOL)
	@echo GEN_VERSION_GO $@
	$(NOECHO)$(GENERATE_VERSION_GO_TOOL) -i $< -o $@
