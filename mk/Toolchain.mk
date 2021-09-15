# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

#### TOOLCHAIN SELECTION #####

# Ubuntu 18.04
ifeq ($(DISTRO),Ubuntu)
ifeq ($(DISTRO_MAJOR),18)
TOOLCHAIN:=gcc9
endif
endif

# Default choice
ifndef TOOLCHAIN
TOOLCHAIN:=gcc
endif

ifeq ($(TOOLCHAIN),gcc)
CC:=gcc
CXX:=g++
LD:=g++
endif

ifeq ($(TOOLCHAIN),gcc7)
CC:=gcc-7
CXX:=g++-7
LD:=g++-7
endif

ifeq ($(TOOLCHAIN),gcc8)
CC:=gcc-8
CXX:=g++-8
LD:=g++-8
endif

ifeq ($(TOOLCHAIN),gcc9)
CC:=gcc-9
CXX:=g++-9
LD:=g++-9
endif

ifeq ($(TOOLCHAIN),gcc10)
CC:=gcc-10
CXX:=g++-10
LD:=g++-10
endif

ifeq ($(TOOLCHAIN),gcc11)
CC:=gcc-11
CXX:=g++-11
LD:=g++-11
endif

ifeq ($(TOOLCHAIN),clang)
CC:=clang
CXX:=clang++
LD:=clang++
endif

ifeq ($(TOOLCHAIN),clang9)
CC:=clang-9
CXX:=clang++-9
LD:=clang++-9
endif

ifeq ($(TOOLCHAIN),clang10)
CC:=clang-10
CXX:=clang++-10
LD:=clang++-10
endif

ifeq ($(TOOLCHAIN),clang11)
CC:=clang-11
CXX:=clang++-11
LD:=clang++-11
endif

ifeq ($(TOOLCHAIN),clang12)
CC:=clang-12
CXX:=clang++-12
LD:=clang++-12
endif

ifeq ($(TOOLCHAIN),clang13)
CC:=clang-13
CXX:=clang++-13
LD:=clang++-13
endif

ifeq ($(TOOLCHAIN),clang14)
CC:=clang-14
CXX:=clang++-14
LD:=clang++-14
endif

AR:=ar

# Go
GO_VERSION:=1.17.1
GO:=/usr/local/go-$(GO_VERSION)/bin/go
