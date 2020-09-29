# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Initial definitions

# Debug build by default
DEBUG:=1

# Build unit tests by default
BUILD_UNIT_TESTS:=1

# Message compiler version
SIODBMC_VERSION:=0.1.4

# Makefile dir
MK:=$(dir $(realpath $(lastword $(MAKEFILE_LIST))))

# Project root
ROOT:=$(realpath $(addsuffix ..,$(MK)))

# Project common sources
COMMON_SUBDIR:=common
COMMON_ROOT:=$(ROOT)/$(COMMON_SUBDIR)
COMMON_LIB_ROOT:=$(COMMON_ROOT)/lib
COMMON_PROTO_DIR:=$(COMMON_LIB_ROOT)/siodb/common/proto

ARCH:=$(shell uname -m)

# Based on this: https://stackoverflow.com/a/8540718/1540501
_get_major_version=$(firstword $(subst ., ,$1))

DISTRO=$(shell lsb_release -is)
DISTRO_VERSION=$(shell lsb_release -rs)
DISTRO_MAJOR:=$(call _get_major_version, $(DISTRO_VERSION))

ifeq ($(DISTRO),CentOS)
RHEL:=1
endif

ifeq ($(DISTRO),RedHatEnterpriseServer)
RHEL:=1
endif

ifeq ($(DISTRO),RedHatEnterprise)
RHEL:=1
endif

# CentOS/RHEL specific
ifeq ($(RHEL),1)
BOOST_VERSION:=169
endif # CentOS


# Various options

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

ifeq ("$(BUILD_NAME)","")
BUILD_DIR:=build
else
BUILD_DIR:=build_$(BUILD_NAME)
endif

BUILD_ROOT:=$(ROOT)/$(BUILD_DIR)

BUILD_CFG_DIR:=$(BUILD_ROOT)/$(BUILDCFG)

BIN_DIR:=$(BUILD_CFG_DIR)/bin

LIB_DIR:=$(BUILD_CFG_DIR)/lib

OBJ_DIR:=$(BUILD_CFG_DIR)/obj
THIS_OBJ_DIR:=$(OBJ_DIR)$(SRC_DIR_OFFSET)

GENERATED_FILES_ROOT:=$(BUILD_CFG_DIR)/generated-src
GENERATED_FILES_DIR:=$(GENERATED_FILES_ROOT)/siodb-generated
THIS_GENERATED_FILES_DIR:=$(GENERATED_FILES_DIR)$(SRC_DIR_OFFSET)
GENERATED_FILES_COMMON_LIB_ROOT:=$(GENERATED_FILES_DIR)/$(COMMON_SUBDIR)/lib

SIODBMC:=siodbmc.$(SIODBMC_VERSION)

include $(MK)/ThirdpartyLibVersions.mk
