# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "protobuf"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_COMMON_LIB:=siodb_common_protobuf

CXX_SRC:= \
	ExtendedCodedInputStream.cpp  \
	ProtobufMessageIO.cpp  \
	ProtobufUtils.cpp  \
	RawDateTimeIO.cpp  \
	StreamInputStream.cpp  \
	StreamOutputStream.cpp

CXX_HDR:= \
	ExtendedCodedInputStream.h  \
	ProtobufMessageIO.h  \
	ProtobufUtils.h  \
	ProtocolError.h  \
	ProtocolMessageType.h  \
	ProtocolTag.h  \
	RawDateTimeIO.h  \
	StreamInputStream.h  \
	StreamOutputStream.h

include $(MK)/Main.mk
