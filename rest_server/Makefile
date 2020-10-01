# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# rest_server application makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../mk/Prolog.mk

TARGET_GO_EXE:=siodb_rest_server

# TODO: Change value to Version.go
VERSION_GO_FILE:=TmpVersion.go

GO_SRC:=\
	TmpMain.go  \
	$(VERSION_GO_FILE)

include $(MK)/Main.mk

$(VERSION_GO_FILE): ../common/lib/siodb/common/config/SiodbVersion.h
	@echo GEN_VERSION_GO $@
	$(NOECHO)$(GENERATE_VERSION_GO_TOOL) -i $< -o $@
