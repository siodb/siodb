# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

include dbengine/crypto/ciphers/Files.mk

CXX_SRC+= \
	dbengine/crypto/KeyGenerator.cpp

CXX_HDR+= \
	dbengine/crypto/KeyGenerator.h
