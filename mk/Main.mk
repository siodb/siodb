# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Common part of all makefiles.
# Contains common make rules

.PHONY: all clean full-clean check-headers print-config

#### TOOLCHAIN SELECTION #####

# Ubuntu 18.04
ifeq ($(DISTRO),Ubuntu)
ifeq ($(DISTRO_MAJOR),18)
TOOLCHAIN:=gcc8
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

ifeq ($(TOOLCHAIN),clang)
CC:=clang
CXX:=clang++
LD:=clang++
endif

ifeq ($(TOOLCHAIN),clang8)
CC:=clang-8
CXX:=clang++-8
LD:=clang++-8
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

AR:=ar

##### BUILD SETTINGS #####

PROTO_CXX_SRC_N:=$(PROTO_SRC:.proto=.pb.cc)
PROTO_CXX_HDR_N:=$(PROTO_SRC:.proto=.pb.h)
PROTO_CXX_SRC:=$(addprefix $(THIS_GENERATED_FILES_DIR), $(PROTO_CXX_SRC_N))
PROTO_CXX_HDR:=$(addprefix $(THIS_GENERATED_FILES_DIR), $(PROTO_CXX_HDR_N))


OBJ:=$(addprefix $(THIS_OBJ_DIR),$(PROTO_CXX_SRC_N:.pb.cc=.pb.o) $(C_SRC:.c=.o) $(CXX_SRC:.cpp=.o))

DEP:=$(OBJ:.o=.d)

CXX_CHK:=$(addprefix $(THIS_OBJ_DIR), $(CXX_HDR:.h=.cxx-hdr-check))
C_CHK:=$(addprefix $(THIS_OBJ_DIR), $(C_HDR:.h=.c-hdr-check))

CXX_CHK_TMP:=$(addsuffix .tmp, $(CXX_CHK))
C_CHK_TMP:=$(addsuffix .tmp, $(C_CHK))

OBJ_DIRS:=$(addsuffix .,$(sort $(dir $(OBJ))))
GENERATED_FILES_DIRS:=$(addsuffix .,$(sort $(dir $(PROTO_CXX_SRC))))

INCLUDE+=-I$(COMMON_LIB_ROOT) -I$(GENERATED_FILES_COMMON_LIB_ROOT)
C_INCLUDE+=
CXX_INCLUDE+=

# Custom ANTLR4 Runtime
ifdef ANTLR4_RUNTIME_ROOT
CXX_INCLUDE+=-isystem $(ANTLR4_RUNTIME_ROOT)/include/antlr4-runtime
LDFLAGS+=-L$(ANTLR4_RUNTIME_ROOT)/lib -Wl,-rpath -Wl,$(ANTLR4_RUNTIME_ROOT)/lib
else
CXX_INCLUDE+=-I/usr/local/include/antlr4-runtime
endif

# Custom Boost
ifdef BOOST_ROOT
CXX_INCLUDE+=-isystem $(BOOST_ROOT)/include
LDFLAGS+=-L$(BOOST_ROOT)/lib -Wl,-rpath -Wl,$(BOOST_ROOT)/lib
else
ifdef BOOST_VERSION
CXX_INCLUDE+=-I/usr/include/boost$(BOOST_VERSION)
LDFLAGS+=-L/usr/lib64/boost$(BOOST_VERSION)
endif
endif

# Custom libdate
ifdef DATE_ROOT
CXX_INCLUDE+=-isystem $(DATE_ROOT)/include
LDFLAGS+=-L$(DATE_ROOT)/lib -Wl,-rpath -Wl,$(DATE_ROOT)/lib
endif

# Custom Google Test
ifdef GTEST_ROOT
CXX_INCLUDE+=-isystem $(GTEST_ROOT)/include/gtest-gmock-1.8.1
else
CXX_INCLUDE+=-I/usr/local/include/gtest-gmock-1.8.1
endif

# Custom Oat++
ifdef OATPP_ROOT
CXX_INCLUDE+=-isystem $(OATPP_ROOT)/include/oatpp-$(OATPP_VERSION)
LDFLAGS+=-L$(OATPP_ROOT)/lib -Wl,-rpath -Wl,$(OATPP_ROOT)/lib
else
CXX_INCLUDE+=-I/usr/local/include/oatpp-$(OATPP_VERSION)
endif

# Custom OpenSSL
ifdef OPENSSL_ROOT
C_INCLUDE+=-isystem $(OPENSSL_ROOT)/include
CXX_INCLUDE+=-isystem $(OPENSSL_ROOT)/include
LDFLAGS+=-L$(OPENSSL_ROOT)/lib -Wl,-rpath -Wl,$(OPENSSL_ROOT)/lib
endif

# Custom Protocol Buffers
ifdef PROTOBUF_ROOT
PROTOC:=$(PROTOBUF_ROOT)/bin/protoc
CXX_INCLUDE+=-isystem $(PROTOBUF_ROOT)/include
LDFLAGS+=-L$(PROTOBUF_ROOT)/lib -Wl,-rpath -Wl,$(PROTOBUF_ROOT)/lib
else
PROTOC:=protoc
endif

# Custom utf8cpp
ifdef UTF8CPP_ROOT
CXX_INCLUDE+=-isystem $(UTF8CPP_ROOT)/include
LDFLAGS+=-L$(UTF8CPP_ROOT)/lib -Wl,-rpath -Wl,$(UTF8CPP_ROOT)/lib
endif

# Custom xxHash
ifdef XXHASH_ROOT
C_INCLUDE+=-isystem $(XXHASH_ROOT)/include
CXX_INCLUDE+=-isystem $(XXHASH_ROOT)/include
LDFLAGS+=-L$(XXHASH_ROOT)/lib -Wl,-rpath -Wl,$(XXHASH_ROOT)/lib
endif

DEFS+=-D_GNU_SOURCE $(TARGET_DEFS)
C_DEFS+=$(TARGET_C_DEFS)
CXX_DEFS+=-DBOOST_ALL_DYN_LINK -DBOOST_FILESYSTEM_NO_DEPRECATED $(TARGET_CXX_DEFS)

ifeq ($(findstring gcc,$(CC)),gcc)
ERROR_LIMIT:=-fmax-errors=5
endif
ifeq ($(findstring clang,$(CC)),clang)
ERROR_LIMIT:=-ferror-limit=5
endif
ifeq ("$(ERROR_LIMIT)","")
$(error Unsupported toolchain)
endif

DEFAULT_CFLAGS:=-pthread -g3 -fPIC \
	-std=gnu11 -Wall -Wextra -Werror -Wpedantic -Wno-unused-value \
	$(ERROR_LIMIT) $(C_INCLUDE) $(INCLUDE) $(DEFS) $(C_DEFS)

DEFAULT_CXXFLAGS:=-pthread -g3 -fPIC \
	-std=gnu++17 -Wall -Wextra -Werror -Wpedantic -Wno-unused-value \
	$(ERROR_LIMIT) $(CXX_INCLUDE) $(INCLUDE) $(DEFS) $(CXX_DEFS)

CFLAGS+=$(DEFAULT_CFLAGS)
CXXFLAGS+=$(DEFAULT_CXXFLAGS)

CPPFLAGS+=-MMD -MP

LDFLAGS+=-pthread -g3 -rdynamic

SYSTEM_LIBS:=-ldl -lrt
OWN_LIBS:=-L$(LIB_DIR) $(addprefix -l,$(TARGET_OWN_LIBS))
COMMON_LIBS:=-L$(LIB_DIR) $(addprefix -lsiodb_common_,$(TARGET_COMMON_LIBS))
LIBS+=$(OWN_LIBS) $(COMMON_LIBS) $(TARGET_LIBS) $(SYSTEM_LIBS)

OWN_LIBS_DEP:=$(addprefix $(LIB_DIR)/lib, $(addsuffix .a, $(TARGET_OWN_LIBS)))
COMMON_LIBS_DEP:=$(addprefix $(LIB_DIR)/libsiodb_common_, $(addsuffix .a, $(TARGET_COMMON_LIBS)))

# Debug info
ifeq ($(DEBUG),1)
CFLAGS+=-DDEBUG -D_DEBUG -O0
CXXFLAGS+=-DDEBUG -D_DEBUG -O0
else
CFLAGS+=-O3 -fno-omit-frame-pointer
CXXFLAGS+=-O3 -fno-omit-frame-pointer
endif

# Precompiled headers
C_PCH_HDR:=common/lib/siodb/common/pch/SiodbPCH_C.h
CXX_PCH_HDR:=common/lib/siodb/common/pch/SiodbPCH_Cxx.h
C_PCH:=$(addprefix $(OBJ_DIR)/, $(C_PCH_HDR:.h=.gch))
CXX_PCH:=$(addprefix $(OBJ_DIR)/, $(CXX_PCH_HDR:.h=.gch))

# Use precompiled headers
# (must be last, because creats copy of CFLAGS and CXXFLAGS)
ifeq ("$(USE_PCH)","1")
C_PCH_DEP:=$(C_PCH)
CXX_PCH_DEP:=$(CXX_PCH)
PCH_CFLAGS:=$(DEFAULT_CFLAGS)
PCH_CXXFLAGS:=$(DEFAULT_CXXFLAGS)
CFLAGS+=-include $(ROOT)/$(C_PCH_HDR)
CXXFLAGS+=-include $(ROOT)/$(CXX_PCH_HDR)
endif

##### TARGETS #####

TARGET_BIN_FILES:=$(addprefix $(BIN_DIR)/,$(notdir $(BIN_FILES)))
TARGET_LIB_FILES:=$(addprefix $(LIB_DIR)/,$(notdir $(LIB_FILES)))

.PRECIOUS: \
	$(PROTO_CXX_HDR)  \
	$(PROTO_CXX_SRC)  \
	$(OBJ_DIRS)  \
	$(GENERATED_FILES_DIRS)  \
	$(C_PCH_DEP)  \
	$(CXX_PCH_DEP)

ifdef TARGET_EXE
MAIN_TARGET:=$(BIN_DIR)/$(TARGET_EXE)
endif

ifdef TARGET_SO
ifdef TARGET_SO_VERSION
MAIN_TARGET:=$(BIN_DIR)/lib$(TARGET_SO).so.$(TARGET_SO_VERSION)
else
MAIN_TARGET:=$(BIN_DIR)/lib$(TARGET_SO).so
endif
endif

ifdef TARGET_LIB
MAIN_TARGET:=$(LIB_DIR)/lib$(TARGET_LIB).a
endif

ifdef TARGET_BIN_FILES
SUPPLEMENTARY_TARGETS+=$(TARGET_BIN_FILES)
endif

ifdef TARGET_LIB_FILES
SUPPLEMENTARY_TARGETS+=$(TARGET_LIB_FILES)
endif

all: \
	print-config \
	$(MAIN_TARGET) \
	$(SUPPLEMENTARY_TARGETS)

print-config:
	@echo -e "\n================================================================================\n"\
	"Build Settings:\n"\
	"\nDistro: $(DISTRO) $(DISTRO_VERSION)\n"\
	"Debug build: $(DEBUG)\n"\
	"Build unit tests: $(BUILD_UNIT_TESTS)\n"\
	"\nCC=$(CC)\n"\
	"CXX=$(LD)\n"\
	"LD=$(LD)\n"\
	"\nCFLAGS=$(CFLAGS)\n\n"\
	"CXXFLAGS=$(CXXFLAGS)\n\n"\
	"LDFLAGS=$(LDFLAGS)\n"\
	"================================================================================\n"

check-headers: print-config $(CXX_CHK) $(C_CHK)

# Targets with actions

clean:
	@echo Removing build files
	-$(NOECHO)rm -rf $(MAIN_TARGET)
	-$(NOECHO)rm -rf $(SUPPLEMENTARY_TARGETS)
	-$(NOECHO)rm -rf $(OBJ)
	-$(NOECHO)rm -rf $(DEP)
	-$(NOECHO)rm -rf $(PROTO_CXX_HDR) $(PROTO_CXX_SRC)
	-$(NOECHO)rm -rf $(EXTRA_C_DEPS) $(EXTRA_CXX_DEPS)
	-$(NOECHO)rm -rf $(CXX_CHK)
	-$(NOECHO)rm -rf $(CXX_CHK_TMP)
	-$(NOECHO)rm -rf $(C_CHK)
	-$(NOECHO)rm -rf $(C_CHK_TMP)

full-clean:
	@echo RM $(BUILD_CFG_DIR)
	-$(NOECHO)rm -rf $(BUILD_CFG_DIR)

# Some ideas are taken from here:
# http://ismail.badawi.io/blog/2017/03/28/automatic-directory-creation-in-make/
.SECONDEXPANSION:

$(OBJ_DIR)/.:
	@echo MKDIR $@
	$(NOECHO)mkdir -p $@

$(OBJ_DIR)%/.:
	@echo MKDIR $@
	$(NOECHO)mkdir -p $@

$(GENERATED_FILES_DIR)/.:
	@echo MKDIR $@
	$(NOECHO)mkdir -p $@

$(GENERATED_FILES_DIR)%/.:
	@echo MKDIR $@
	$(NOECHO)mkdir -p $@

$(BIN_DIR):
	@echo MKDIR $@
	$(NOECHO)mkdir -p $@

$(LIB_DIR):
	@echo MKDIR $@
	$(NOECHO)mkdir -p $@

$(GENERATED_FILES_DIR)/%.pb.cc $(GENERATED_FILES_DIR)/%.pb.h: $(ROOT)/%.proto | $$(@D)/.
	@echo PROTOC $@
	$(NOECHO)$(PROTOC) -I$(COMMON_PROTO_DIR) $(PROTOC_INCLUDE) --cpp_out=$(realpath $(dir $@)) $(realpath $<)

ifeq ("$(USE_PCH)","1")

$(C_PCH) : $(ROOT)/$(C_PCH_HDR) $(EXTRA_C_DEPS) | $$(@D)/.
	@echo CC $@
	$(NOECHO)$(CC) -o $@ $(PCH_CFLAGS) $(CPPFLAGS) -c $<

$(CXX_PCH) : $(ROOT)/$(CXX_PCH_HDR) $(PROTO_CXX_HDR) $(EXTRA_CXX_DEPS) | $$(@D)/.
	@echo CXX $@
	$(NOECHO)$(CXX) -o $@ $(PCH_CXXFLAGS) $(CPPFLAGS) -c $<

endif

$(OBJ_DIR)/%.pb.o : $(GENERATED_FILES_DIR)/%.pb.cc $(PROTO_CXX_HDR) $(CXX_PCH_DEP) | $$(@D)/.
	@echo CXX $@
	$(NOECHO)$(CXX) -o $@ $(CXXFLAGS) $(CPPFLAGS) -c $<

$(OBJ_DIR)/%.o : $(ROOT)/%.c $(EXTRA_C_DEPS) $(C_PCH_DEP) | $$(@D)/.
	@echo CC $@
	$(NOECHO)$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) -c $<

$(OBJ_DIR)/%.o : $(ROOT)/%.cpp $(PROTO_CXX_HDR) $(EXTRA_CXX_DEPS) $(CXX_PCH_DEP) | $$(@D)/.
	@echo CXX $@
	$(NOECHO)$(CXX) -o $@ $(CXXFLAGS) $(CPPFLAGS) -c $<

$(OBJ_DIR)/%.c-hdr-check : $(ROOT)/%.h $(EXTRA_C_DEPS) | $$(@D)/.
	@echo C_HEADER_CHECK $@
	$(NOECHO)echo "#include \"$<\"" >"$@.h"
	$(NOECHO)echo "#include <stdlib.h>" >>"$@.h"
	$(NOECHO)echo "static inline int test() { return rand(); }" >>"$@.h"
	$(NOECHO)$(CC) -o $@.o $(CFLAGS) $(CPPFLAGS) -c "$@.h"
	$(NOECHO)rm -f $@.h $@.o
	$(NOECHO)touch $@

$(OBJ_DIR)/%.cxx-hdr-check : $(ROOT)/%.h $(PROTO_CXX_HDR) $(EXTRA_CXX_DEPS) | $$(@D)/.
	@echo CXX_HEADER_CHECK $@
	$(NOECHO)echo "#include \"$<\"" >"$@.h"
	$(NOECHO)echo "#include <cstdlib>" >>"$@.h"
	$(NOECHO)echo "static inline int test() { return rand(); }" >>"$@.h"
	$(NOECHO)$(CXX) -o $@.o $(CXXFLAGS) $(CPPFLAGS) -c "$@.h"
	$(NOECHO)rm -f $@.h $@.o
	$(NOECHO)touch $@

ifneq ($(BIN_FILES),)
# Based on idea from here:
# https://stackoverflow.com/a/23790377/1540501
define COPY_TO_BIN
$(BIN_DIR)/$(notdir $1) : $(1)
	@echo COPY $$< to $$@
	$(NOECHO)cp -f $$< $$@
endef
$(foreach T,$(BIN_FILES),$(eval $(call COPY_TO_BIN,$T)))
endif

ifneq ($(LIB_FILES),)
# Based on idea from here:
# https://stackoverflow.com/a/23790377/1540501
define COPY_TO_LIB
$(LIB_DIR)/$(notdir $1) : $(1)
	@echo COPY $$< to $$@
	$(NOECHO)cp -f $$< $$@
endef
$(foreach T,$(LIB_FILES),$(eval $(call COPY_TO_LIB,$T)))
endif

ifdef TARGET_EXE
$(MAIN_TARGET): $(OBJ) $(OWN_LIBS_DEP) $(COMMON_LIBS_DEP) | $(BIN_DIR)
	@echo LD $@
	$(NOECHO)$(LD) -o $@ $(LDFLAGS) $(OBJ) $(LIBS)
	@echo DONE $@
endif

ifdef TARGET_SO
$(MAIN_TARGET): $(OBJ) $(OWN_LIBS_DEP) $(COMMON_LIBS_DEP) | $(BIN_DIR)
	@echo LD $@
	$(NOECHO)$(LD) -o $@ -shared $(LDFLAGS) $(OBJ) $(LIBS)
	@echo DONE $@
endif

ifdef TARGET_LIB
$(MAIN_TARGET): $(OBJ) | $(LIB_DIR)
	@echo AR $@
	-$(NOECHO)rm -f $@
	$(NOECHO)$(AR) rcs $@ $^
	@echo DONE $@
endif

# For debug purposes
$(THIS_OBJ_DIR)/debug_stamp: $$(@D)/.
	echo Proto SRC $(PROTO_CXX_SRC)
	echo Proto HDR $(PROTO_CXX_SRC)
	@touch $@

-include $(DEP)
