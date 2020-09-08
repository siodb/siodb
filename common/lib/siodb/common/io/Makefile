# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "io"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_COMMON_LIB:=siodb_common_io

CXX_SRC:= \
	BufferedChunkedOutputStream.cpp  \
	BufferedOutputStream.cpp  \
	ChunkedInputStream.cpp  \
	DynamicMemoryOutputStream.cpp  \
	FDStream.cpp  \
	InputStream.cpp  \
	InputStreamUtils.cpp  \
	InputStreamWrapperStream.cpp  \
	JsonWriter.cpp  \
	MemoryInputStream.cpp  \
	MemoryOutputStream.cpp  \
	MemoryMappedFile.cpp  \
	OutputStreamWrapperStream.cpp

C_SRC:= \
	FileIO.c

CXX_HDR:= \
	BufferedChunkedOutputStream.h  \
	BufferedOutputStream.h  \
	ChunkedInputStream.h  \
	DynamicMemoryOutputStream.h  \
	FDStream.h  \
	InputOutputStream.h  \
	InputStream.h  \
	InputStreamStdStreamBuffer.h  \
	InputStreamUtils.h  \
	InputStreamWrapperStream.h  \
	JsonWriter.h  \
	MemoryInputStream.h  \
	MemoryMappedFile.h \
	MemoryOutputStream.h  \
	OutputStream.h  \
	OutputStreamWrapperStream.h  \
	Stream.h  \
	StreamFormatGuard.h

C_HDR:= \
	FileIO.h \

include $(MK)/Main.mk
