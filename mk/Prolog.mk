# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Initial definitions

# debug build by default
DEBUG:=1

# Makefile dir
MK:=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))

# Project root
ROOT:=$(realpath $(addsuffix ..,$(MK)))

# Project common sources
COMMON_SUBDIR:=common
COMMON_ROOT:=$(ROOT)/$(COMMON_SUBDIR)
COMMON_LIB_ROOT:=$(COMMON_ROOT)/lib
COMMON_PROTO_DIR:=$(COMMON_LIB_ROOT)/siodb/common/proto

SIODBMC:=siodbmc.0.1.4

ARCH:=$(shell uname -m)

DISTRO=$(shell lsb_release -is)
ifeq ($(DISTRO),CentOS)
BOOST_VERSION:=169
endif

ifneq ($(VERBOSE),1)
NOECHO=@
endif

ifeq ($(DEBUG),1)
BUILDCFG:=debug
else
BUILDCFG:=release
endif

ifeq ($(PNBUILD),1)
DEFS+=-DSIODB_FORCE_PROCESSOR_NEUTRAL_CODE
override BUILDCFG:=$(BUILDCFG)-pn
endif

SRC_DIR_OFFSET:=$(subst $(ROOT),,$(SRC_DIR))

BUILD_ROOT:=$(ROOT)/build

BUILD_CFG_DIR:=$(BUILD_ROOT)/$(BUILDCFG)

BIN_DIR:=$(BUILD_CFG_DIR)/bin

LIB_DIR:=$(BUILD_CFG_DIR)/lib

OBJ_DIR:=$(BUILD_CFG_DIR)/obj
THIS_OBJ_DIR:=$(OBJ_DIR)$(SRC_DIR_OFFSET)

GENERATED_FILES_ROOT:=$(BUILD_CFG_DIR)/generated-src
GENERATED_FILES_DIR:=$(GENERATED_FILES_ROOT)/siodb-generated
THIS_GENERATED_FILES_DIR:=$(GENERATED_FILES_DIR)$(SRC_DIR_OFFSET)
GENERATED_FILES_COMMON_LIB_ROOT:=$(GENERATED_FILES_DIR)/$(COMMON_SUBDIR)/lib
