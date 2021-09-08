# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Versions of the third-party libraries and tools used in the Siodb
# BEGIN_THIRDPARTY_LIB_VERSIONS (do not rmeove this line, some scripts depend on it)
ANTLR4_VERSION:=4.9.2
ANTLR4_CPP_RUNTIME_VERSION:=4.9.2
LIBDATE_VERSION:=3.0.1-git~77bd6b9
GTEST_VERSION:=1.10.0
JSON_VERSION:=3.10.2
OPENSSL_VERSION:=1.1.1k
PROTOBUF_VERSION:=3.15.6
UTF8CPP_VERSION:=3.1.2
XXHASH_VERSION:=0.8.0
# END_THIRDPARTY_LIB_VERSIONS (do not remove this line, some scripts depend on it)

ifeq ($(RHEL),1)
ifeq ($(DISTRO_MAJOR),7)
USE_CUSTOM_OPENSSL:=1
endif
endif

# Standard installation directory for the third-part libraries and tools
THIRD_PARTY_ROOT:=/opt/siodb/lib

