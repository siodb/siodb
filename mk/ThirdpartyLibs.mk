# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# OS-specific lib directory
ifeq ($(RHEL),1)
OS_LIBDIR:=lib64
else
OS_LIBDIR=lib
endif

# ANTLR4 Runtime
ANTLR4_RUNTIME_ROOT:=$(THIRD_PARTY_ROOT)/antlr4-cpp-runtime-$(ANTLR4_CPP_RUNTIME_VERSION)
CXX_INCLUDE+=-isystem $(ANTLR4_RUNTIME_ROOT)/include/antlr4-runtime
# This one is always in "lib"
LDFLAGS+=-L$(ANTLR4_RUNTIME_ROOT)/lib -Wl,-rpath -Wl,$(ANTLR4_RUNTIME_ROOT)/lib

# Custom Boost
ifdef BOOST_ROOT
CXX_INCLUDE+=-isystem $(BOOST_ROOT)/include
LDFLAGS+=-L$(BOOST_ROOT)/lib -Wl,-rpath -Wl,$(BOOST_ROOT)/lib
else
ifdef BOOST_VERSION
CXX_INCLUDE+=-I/usr/include/boost$(BOOST_VERSION)
LDFLAGS+=-L/usr/$(OS_LIBDIR)/boost$(BOOST_VERSION)
endif
endif

# libdate
LIBDATE_ROOT:=$(THIRD_PARTY_ROOT)/date-$(LIBDATE_VERSION)
CXX_INCLUDE+=-isystem $(LIBDATE_ROOT)/include
LDFLAGS+=-L$(LIBDATE_ROOT)/$(OS_LIBDIR) -Wl,-rpath -Wl,$(LIBDATE_ROOT)/$(OS_LIBDIR)

# Google Test
GTEST_ROOT:=$(THIRD_PARTY_ROOT)/googletest-$(GTEST_VERSION)
CXX_INCLUDE+=-isystem $(GTEST_ROOT)/include

# JSON
JSON_ROOT:=$(THIRD_PARTY_ROOT)/json-$(JSON_VERSION)
CXX_INCLUDE+=-isystem $(JSON_ROOT)/include

# Custom OpenSSL
ifdef USE_CUSTOM_OPENSSL
OPENSSL_ROOT:=$(THIRD_PARTY_ROOT)/openssl-$(OPENSSL_VERSION)
endif
ifdef OPENSSL_ROOT
C_INCLUDE+=-isystem $(OPENSSL_ROOT)/include
CXX_INCLUDE+=-isystem $(OPENSSL_ROOT)/include
LDFLAGS+=-L$(OPENSSL_ROOT)/lib -Wl,-rpath -Wl,$(OPENSSL_ROOT)/lib
endif

# Protocol Buffers
PROTOBUF_ROOT:=$(THIRD_PARTY_ROOT)/protobuf-$(PROTOBUF_VERSION)
PROTOC:=$(PROTOBUF_ROOT)/bin/protoc
CXX_INCLUDE+=-isystem $(PROTOBUF_ROOT)/include
# Always in lib
LDFLAGS+=-L$(PROTOBUF_ROOT)/lib -Wl,-rpath -Wl,$(PROTOBUF_ROOT)/lib

# utf8cpp
UTF8CPP_ROOT:=$(THIRD_PARTY_ROOT)/utf8cpp-$(UTF8CPP_VERSION)
CXX_INCLUDE+=-isystem $(UTF8CPP_ROOT)/include
LDFLAGS+=-L$(UTF8CPP_ROOT)/$(OS_LIBDIR) -Wl,-rpath -Wl,$(UTF8CPP_ROOT)/$(OS_LIBDIR)

# xxHash
XXHASH_ROOT:=$(THIRD_PARTY_ROOT)/xxHash-$(XXHASH_VERSION)
C_INCLUDE+=-isystem $(XXHASH_ROOT)/include
CXX_INCLUDE+=-isystem $(XXHASH_ROOT)/include
LDFLAGS+=-L$(XXHASH_ROOT)/$(OS_LIBDIR) -Wl,-rpath -Wl,$(XXHASH_ROOT)/$(OS_LIBDIR)
