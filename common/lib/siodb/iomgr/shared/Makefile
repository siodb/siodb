# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# IO manager library makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_LIB:=siodb_common_iomgr_shared

include Files.mk

CXXFLAGS+=-I$(GENERATED_FILES_ROOT)

include $(MK)/Main.mk
