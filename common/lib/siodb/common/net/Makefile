# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "net"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_COMMON_LIB:=siodb_common_net

CXX_SRC:= \
	EpollHelpers.cpp \
	SocketDomain.cpp \
	TcpConnection.cpp \
	TcpServer.cpp \
	UnixConnection.cpp \
	UnixServer.cpp

C_SRC:= \
	NetUtils.c

CXX_HDR:= \
	detail/AddressInfoGuard.h \
	ConnectionError.h \
	EpollHelpers.h \
	NetConstants.h \
	SocketDomain.h \
	TcpConnection.h \
	TcpServer.h \
	UnixConnection.h \
	UnixServer.h

C_HDR:= \
	NetUtils.h

include $(MK)/Main.mk
