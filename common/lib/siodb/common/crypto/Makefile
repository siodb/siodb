# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "crypto"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_COMMON_LIB:=siodb_common_crypto

CXX_SRC:= \
	DigitalSignatureKey.cpp  \
	RandomGenerator.cpp  \
	TlsClient.cpp  \
	TlsConnection.cpp  \
	TlsServer.cpp

CXX_HDR:= \
	DigitalSignatureKey.h  \
	RandomGenerator.h  \
	OpenSslError.h  \
	TlsClient.h  \
	TlsConnection.h  \
	TlsServer.h  \
	openssl_wrappers/BigNum.h  \
	openssl_wrappers/BioMemBuf.h  \
	openssl_wrappers/DsaKey.h  \
	openssl_wrappers/EcKey.h  \
	openssl_wrappers/EcPoint.h  \
	openssl_wrappers/EvpKey.h  \
	openssl_wrappers/EvpMdCtx.h  \
	openssl_wrappers/EvpPkeyCtx.h  \
	openssl_wrappers/RsaKey.h  \
	openssl_wrappers/Ssl.h  \
	openssl_wrappers/SslContext.h

include $(MK)/Main.mk
