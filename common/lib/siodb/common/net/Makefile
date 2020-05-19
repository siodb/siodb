# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "net"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_LIB:=siodb_common_net

CXX_SRC:= \
	EpollHelpers.cpp  \
	TcpConnection.cpp  \
	TcpServer.cpp  \
	UnixConnection.cpp  \
	UnixServer.cpp

C_SRC:= \
	NetUtils.c

CXX_HDR:= \
	ConnectionError.h  \
	EpollHelpers.h  \
	NetConstants.h  \
	TcpConnection.h  \
	TcpServer.h  \
	UnixConnection.h  \
	UnixServer.h  \
	internal/AddressInfoGuard.h

C_HDR:= \
	NetUtils.h

include $(MK)/Main.mk
