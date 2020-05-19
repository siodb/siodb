# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the sysdbid utility

.PHONY: all build clean rebuild

# Target
TARGET:=sys_db_uuid

# Sources
SRC:=$(TARGET).c

# Objects
OBJ:=$(SRC:.c=.o)

# Dependencies
DEP:=$(OBJ:.o=.d)

# Toolchain
CC:=gcc
INCLUDE:=
CFLAGS:=-std=gnu99 -Wall -Wextra -Wpedantic -Werror -fmax-errors=5 -fPIC $(INCLUDE)

LD:=gcc
LDFLAGS:=

ifeq ($(DEBUG),1)
CFLAGS+=-DDEBUD -D_DEBUG -O0 -g3
LDFLAGS+=-g3
else
CFLAGS+=-O2
endif

LIBS:=-lcrypto
ifeq ($(DEBUG),1)
LIBS+= 
else
LIBS+=
endif

CPP:=cpp
CPPFLAGS:=-MP -MMD

# Build rules

all: build

build: $(TARGET)

rebuild:
	echo Rebuilding $(TARGET)
	$(MAKE) clean
	$(MAKE) build

clean:
	-rm -f $(TARGET)
	-rm -f *.o
	-rm -f *.d

$(TARGET): $(OBJ)
	$(LD) -o$@ $(LDFLAGS) $^ $(LIBS)
