# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "protobuf"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_COMMON_LIB:=siodb_common_protobuf

CXX_SRC:= \
	SiodbCodedInputStream.cpp  \
	SiodbProtobufInputStream.cpp  \
	SiodbProtobufOutputStream.cpp  \
	ProtobufJsonIO.cpp  \
	ProtobufMessageIO.cpp  \
	ProtobufUtils.cpp  \
	RawDateTimeIO.cpp

CXX_HDR:= \
	SiodbCodedInputStream.h  \
	SiodbProtobufInputStream.h  \
	SiodbProtobufOutputStream.h  \
	ProtobufJsonIO.h  \
	ProtobufMessageIO.h  \
	ProtobufUtils.h  \
	SiodbProtocolError.h  \
	SiodbProtocolMessageType.h  \
	SiodbProtocolTag.h  \
	RawDateTimeIO.h

include $(MK)/Main.mk
