# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

CXX_SRC+= \
	dbengine/crypto/ciphers/AesCipher.cpp \
	dbengine/crypto/ciphers/AesCipherContext.cpp \
	dbengine/crypto/ciphers/CamelliaCipher.cpp \
	dbengine/crypto/ciphers/CamelliaCipherContext.cpp \
	dbengine/crypto/ciphers/Cipher.cpp

CXX_HDR+= \
	dbengine/crypto/ciphers/AesCipher.h \
	dbengine/crypto/ciphers/AesCipherContext.h \
	dbengine/crypto/ciphers/CamelliaCipher.h \
	dbengine/crypto/ciphers/CamelliaCipherContext.h \
	dbengine/crypto/ciphers/Cipher.h \
	dbengine/crypto/KeyGenerator.h
