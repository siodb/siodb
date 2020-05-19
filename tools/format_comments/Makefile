# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the comment formatting mini utility

DEBUG:=0
TARGET:=format_comments

SRC:=main.cpp
OBJ:=$(SRC:.cpp=.o)
DEP:=$(OBJ:.o=.d)

CXX:=g++
LD:=g++

DEFS:=-D_GNU_SOURCE
CXXFLAGS:=-std=gnu++17 -Wall -Wextra -Werror -Wpedantic -fmax-errors=5 $(INCLUDES) $(DEFS)
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
	install -s -m 0755 ./siodbmc1 /usr/local/bin
