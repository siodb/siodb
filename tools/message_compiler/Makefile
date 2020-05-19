# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the message compiler tool

DEBUG:=0

TARGET_VERSION=0.1.4
COPYRIGHT_YEARS=2019-2020

ifeq ($(DEBUG),1)
TARGET:=siodbmc
else
TARGET:=siodbmc.$(TARGET_VERSION)
endif

SRC:=Compiler.cpp CompilerOptions.cpp
OBJ:=$(SRC:.cpp=.o)
DEP:=$(OBJ:.o=.d)

DISTRO=$(shell lsb_release -is)
ifeq ($(DISTRO),CentOS)
BOOST_VERSION:=169
endif

CXX:=g++
LD:=g++

ifdef BOOST_VERSION
INCLUDE+=-I/usr/include/boost$(BOOST_VERSION)
endif

DEFS:=-D_GNU_SOURCE \
	-DSIODB_MESSAGE_COMPILER_VERSION=\"$(TARGET_VERSION)\" \
	-DSIODB_MESSAGE_COMPILER_COPYRIGHT_YEARS=\"$(COPYRIGHT_YEARS)\"
CXXFLAGS:=-std=gnu++17 -Wall -Wextra -Werror -Wpedantic -fmax-errors=5 $(INCLUDE) $(DEFS)
CPPFLAGS:=-MMD -MP
LIBS:=

ifeq ($(DEBUG),1)
CXXFLAGS+=-g3 -O0 -D_DEBUG
LDFLAGS+=-g3
else
CXXFLAGS+=-O3
endif

all: $(TARGET)

clean:
	-rm -f $(TARGET)
	-rm -f $(DEP) $(OBJ)

-include $(DEP)

$(TARGET): $(OBJ)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

install:
	install -s -m 0755 ./$(TARGET) /usr/local/bin

test1:  $(TARGET)
	./$(TARGET) -T -VM -i test.msg -o testmsg.txt
