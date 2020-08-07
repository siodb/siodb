# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Common part of all makefiles.
# Contains common make rules

.PHONY: all clean full-clean check-headers print-config

include $(MK)/Toolchain.mk

##### BUILD SETTINGS #####

# Sources
PROTO_CXX_SRC_N:=$(PROTO_SRC:.proto=.pb.cc)
PROTO_CXX_HDR_N:=$(PROTO_SRC:.proto=.pb.h)
PROTO_CXX_SRC:=$(addprefix $(THIS_GENERATED_FILES_DIR), $(PROTO_CXX_SRC_N))
PROTO_CXX_HDR:=$(addprefix $(THIS_GENERATED_FILES_DIR), $(PROTO_CXX_HDR_N))

# Objects
OBJ:=$(addprefix $(THIS_OBJ_DIR),$(PROTO_CXX_SRC_N:.pb.cc=.pb.o) $(C_SRC:.c=.o) $(CXX_SRC:.cpp=.o))

# Generated dependencies
DEP:=$(OBJ:.o=.d)

# Header check
CXX_CHK:=$(addprefix $(THIS_OBJ_DIR), $(CXX_HDR:.h=.cxx-hdr-check))
C_CHK:=$(addprefix $(THIS_OBJ_DIR), $(C_HDR:.h=.c-hdr-check))

OBJ_DIRS:=$(addsuffix .,$(sort $(dir $(OBJ))))
GENERATED_FILES_DIRS:=$(addsuffix .,$(sort $(dir $(PROTO_CXX_SRC))))

INCLUDE+=-I$(COMMON_LIB_ROOT) -I$(GENERATED_FILES_COMMON_LIB_ROOT)
C_INCLUDE+=
CXX_INCLUDE+=

include $(MK)/ThirdpartyLibs.mk

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

DEFAULT_CPPFLAGS:=-MMD -MP

DEFAULT_CFLAGS:=-pthread -g3 \
	-std=gnu11 -Wall -Wextra -Werror -Wpedantic -Wno-unused-value \
	$(ERROR_LIMIT) $(C_INCLUDE) $(INCLUDE) $(DEFS) $(C_DEFS) \
	-ffunction-sections -fdata-sections

DEFAULT_CXXFLAGS:=-pthread -g3 \
	-std=gnu++17 -Wall -Wextra -Werror -Wpedantic -Wno-unused-value \
	$(ERROR_LIMIT) $(CXX_INCLUDE) $(INCLUDE) $(DEFS) $(CXX_DEFS) \
	-ffunction-sections -fdata-sections

DEFAULT_LDFLAGS:=-pthread -g3 -rdynamic -Wl,--gc-sections

CHECK_HEADERS_CFLAGS:=-Wno-unused-function
CHECK_HEADERS_CXXFLAGS:=-Wno-unused-function

# PIE flags
ifdef TARGET_SO
DEFAULT_CFLAGS+=-fPIC
DEFAULT_CXXFLAGS+=-fPIC
DEFAULT_LDFLAGS:=-shared
else
ifdef TARGET_COMMON_LIB
DEFAULT_CFLAGS+=-fPIC
DEFAULT_CXXFLAGS+=-fPIC
else
DEFAULT_CFLAGS+=-fPIE
DEFAULT_CXXFLAGS+=-fPIE
ifneq ($(RHEL),1) # Broken on the RHEL
DEFAULT_LDFLAGS+=-Wl,-pie
endif
endif
endif

# Recommended by RedHat for building packages
# See https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc/

RH_CFLAGS:=-pipe -fexceptions -fasynchronous-unwind-tables \
	-fstack-clash-protection -fstack-protector-strong -grecord-gcc-switches \
	-fcf-protection=full

RH_CXXFLAGS:=-pipe -fexceptions -fasynchronous-unwind-tables \
	-fstack-clash-protection -fstack-protector-strong -grecord-gcc-switches \
	-fcf-protection=full -D_GLIBCXX_ASSERTIONS

RH_LDFLAGS:=-Wl,-z,defs -Wl,-z,now -Wl,-z,relro

# Depends on the optimizations, so effective only for the non-debug builds
ifneq ($(DEBUG),1)
RH_CFLAGS+=-D_FORTIFY_SOURCE=2
RH_CXXFLAGS+=-D_FORTIFY_SOURCE=2
endif

# RHEL/CentOS only options
ifeq ($(RHEL),1)
# CentOS 7/RHEL 7 + DTS9 only
# DEFAULT_CXXFLAGS+=-Wno-error=deprecated-copy

# CentOS 7/RHEL 7 + DTS8 only
ifeq ($(DISTRO_MAJOR),7)
RH_CFLAGS+=-mcet
RH_CXXFLAGS+=-mcet
endif

ECHO_E_OPTION:=-e
endif

CPPFLAGS+=$(DEFAULT_CPPFLAGS)
CFLAGS+=$(DEFAULT_CFLAGS) $(RH_CFLAGS)
CXXFLAGS+=$(DEFAULT_CXXFLAGS) $(RH_CXXFLAGS)
LDFLAGS+=$(DEFAULT_LDFLAGS) $(RH_LDFLAGS)

SYSTEM_LIBS:=-ldl -lrt
OWN_LIBS:=-L$(LIB_DIR) $(addprefix -l,$(TARGET_OWN_LIBS))
COMMON_LIBS:=-L$(LIB_DIR) $(addprefix -lsiodb_common_,$(TARGET_COMMON_LIBS))
LIBS+=$(OWN_LIBS) $(COMMON_LIBS) $(TARGET_LIBS) $(SYSTEM_LIBS)

OWN_LIBS_DEP:=$(addprefix $(LIB_DIR)/lib, $(addsuffix .a, $(TARGET_OWN_LIBS)))
COMMON_LIBS_DEP:=$(addprefix $(LIB_DIR)/libsiodb_common_, $(addsuffix .a, $(TARGET_COMMON_LIBS)))

# Debug build
ifeq ($(DEBUG),1)
CFLAGS+=-DDEBUG -D_DEBUG -O0
CXXFLAGS+=-DDEBUG -D_DEBUG -O0
else
CFLAGS+=-O2 -fno-omit-frame-pointer
CXXFLAGS+=-O2 -fno-omit-frame-pointer
ifeq ($(ENABLE_LTO),1)
CFLAGS+=-flto
CXXFLAGS+=-flto
LDFLAGS+=-flto
endif
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
PCH_CFLAGS:=$(CFLAGS)
PCH_CXXFLAGS:=$(CXXFLAGS)
CFLAGS+=-include $(ROOT)/$(C_PCH_HDR)
CXXFLAGS+=-include $(ROOT)/$(CXX_PCH_HDR)
endif

##### TARGETS #####

TARGET_BIN_FILES:=$(addprefix $(BIN_DIR)/,$(notdir $(BIN_FILES)))
TARGET_LIB_FILES:=$(addprefix $(LIB_DIR)/,$(notdir $(LIB_FILES)))

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

ifdef TARGET_COMMON_LIB
MAIN_TARGET:=$(LIB_DIR)/lib$(TARGET_COMMON_LIB).a
endif

ifdef TARGET_BIN_FILES
SUPPLEMENTARY_TARGETS+=$(TARGET_BIN_FILES)
endif

ifdef TARGET_LIB_FILES
SUPPLEMENTARY_TARGETS+=$(TARGET_LIB_FILES)
endif

.PRECIOUS: \
	$(PROTO_CXX_HDR)  \
	$(PROTO_CXX_SRC)  \
	$(OBJ_DIRS)  \
	$(GENERATED_FILES_DIRS)  \
	$(C_PCH_DEP)  \
	$(CXX_PCH_DEP)  \
	$(MAIN_TARGET).debug

all: \
	print-config \
	$(MAIN_TARGET) \
	$(SUPPLEMENTARY_TARGETS)

print-config:
	@echo $(ECHO_E_OPTION) "\n================================================================================\n"\
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
	-$(NOECHO)rm -rf $(MAIN_TARGET) $(MAIN_TARGET).debug
	-$(NOECHO)rm -rf $(SUPPLEMENTARY_TARGETS)
	-$(NOECHO)rm -rf $(OBJ)
	-$(NOECHO)rm -rf $(DEP)
	-$(NOECHO)rm -rf $(PROTO_CXX_HDR) $(PROTO_CXX_SRC)
	-$(NOECHO)rm -rf $(EXTRA_C_DEPS) $(EXTRA_CXX_DEPS)
	-$(NOECHO)rm -rf $(CXX_CHK)
	-$(NOECHO)rm -rf $(C_CHK)

full-clean:
	@echo RM $(BUILD_CFG_DIR)
	-$(NOECHO)rm -rf $(BUILD_CFG_DIR)

# Some ideas are taken from here:
# http://ismail.badawi.io/blog/2017/03/28/automatic-directory-creation-in-make/
.SECONDEXPANSION:

# Directories

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


# Protobuf compilation
$(GENERATED_FILES_DIR)/%.pb.cc $(GENERATED_FILES_DIR)/%.pb.h: $(ROOT)/%.proto | $$(@D)/.
	@echo PROTOC $@
	$(NOECHO)$(PROTOC) -I$(COMMON_PROTO_DIR) $(PROTOC_INCLUDE) --cpp_out=$(realpath $(dir $@)) $(realpath $<)


# Precompiled headers
ifeq ("$(USE_PCH)","1")

$(C_PCH) : $(ROOT)/$(C_PCH_HDR) $(EXTRA_C_DEPS) | $$(@D)/.
	@echo CC $@
	$(NOECHO)$(CC) -o $@ $(PCH_CFLAGS) $(CPPFLAGS) -c $<

$(CXX_PCH) : $(ROOT)/$(CXX_PCH_HDR) $(PROTO_CXX_HDR) $(EXTRA_CXX_DEPS) | $$(@D)/.
	@echo CXX $@
	$(NOECHO)$(CXX) -o $@ $(PCH_CXXFLAGS) $(CPPFLAGS) -c $<

endif


# C and C++ compilation

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
	$(NOECHO)echo "#include \"$<\"" >"$@.c"
	$(NOECHO)echo "int c_header_check_dummy(void);" >>"$@.c"
	$(NOECHO)echo "int test() { return c_header_check_dummy() + 1; }" >>"$@.c"
	$(NOECHO)$(CC) -o $@.o $(CFLAGS) $(CHECK_HEADERS_CFLAGS) $(CPPFLAGS) -c "$@.c"
	$(NOECHO)rm -f $@.c $@.o
	$(NOECHO)touch $@

$(OBJ_DIR)/%.cxx-hdr-check : $(ROOT)/%.h $(PROTO_CXX_HDR) $(EXTRA_CXX_DEPS) | $$(@D)/.
	@echo CXX_HEADER_CHECK $@
	$(NOECHO)echo "#include \"$<\"" >"$@.cpp"
	$(NOECHO)echo "int cxx_header_check_dummy(void);" >>"$@.cpp"
	$(NOECHO)echo "int test() { return cxx_header_check_dummy() + 1; }" >>"$@.cpp"
	$(NOECHO)$(CXX) -o $@.o $(CXXFLAGS) $(CHECK_HEADERS_CXXFLAGS) $(CPPFLAGS) -c "$@.cpp"
	$(NOECHO)rm -f $@.cpp $@.o
	$(NOECHO)touch $@


# Extra files

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


# Executables and shared libraries

ifdef TARGET_EXE

$(MAIN_TARGET): $(OBJ) $(OWN_LIBS_DEP) $(COMMON_LIBS_DEP) | $(BIN_DIR)
	@echo LD $@
	-$(NOECHO)rm -f $@.tmp1
	$(NOECHO)$(LD) -o $@.tmp1 $(LDFLAGS) $(OBJ) $(LIBS)
	$(NOECHO)objcopy --only-keep-debug $@.tmp1 $@.tmp2
	$(NOECHO)chmod -x $@.tmp2
	$(NOECHO)mv -f $@.tmp2 $@.debug
	-$(NOECHO)rm -f $@
	$(NOECHO)strip --strip-debug --strip-unneeded $@.tmp1
	$(NOECHO)objcopy --add-gnu-debuglink=$@.debug $@.tmp1
	$(NOECHO)mv -f $@.tmp1 $@
	@echo DONE $@

endif # TARGET_EXE


ifdef TARGET_SO

$(MAIN_TARGET): $(OBJ) $(OWN_LIBS_DEP) $(COMMON_LIBS_DEP) | $(BIN_DIR)
	@echo LDSO $@
	-$(NOECHO)rm -f $@.tmp1
	$(NOECHO)$(LD) -o $@.tmp1 $(LDFLAGS) $(OBJ) $(LIBS)
	$(NOECHO)objcopy --only-keep-debug $@.tmp1 $@.tmp2
	$(NOECHO)chmod -x $@.tmp2
	$(NOECHO)mv -f $@.tmp2 $@.debug
	-$(NOECHO)rm -f $@
	$(NOECHO)strip --strip-debug --strip-unneeded $@.tmp1
	$(NOECHO)objcopy --add-gnu-debuglink=$@.debug $@.tmp1
	$(NOECHO)mv -f $@.tmp1 $@
	@echo DONE $@

endif # TARGET_SO


ifdef TARGET_LIB

$(MAIN_TARGET): $(OBJ) | $(LIB_DIR)
	@echo AR $@
	-$(NOECHO)rm -f $@
	$(NOECHO)$(AR) rcs $@ $^
	@echo DONE $@

endif # TARGET_LIB


ifdef TARGET_COMMON_LIB

$(MAIN_TARGET): $(OBJ) | $(LIB_DIR)
	@echo AR $@
	-$(NOECHO)rm -f $@
	$(NOECHO)$(AR) rcs $@ $^
	@echo DONE $@

endif # TARGET_COMMON_LIB


# For debug purposes
$(THIS_OBJ_DIR)/debug_stamp: $$(@D)/.
	echo Proto SRC $(PROTO_CXX_SRC)
	echo Proto HDR $(PROTO_CXX_SRC)
	@touch $@


# Generated dependencies
-include $(DEP)
