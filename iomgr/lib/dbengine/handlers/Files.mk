# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

CXX_SRC+= \
	handlers/JsonOutput.cpp \
	handlers/RequestHandler_AC.cpp \
	handlers/RequestHandler_Common.cpp \
	handlers/RequestHandler_DDL.cpp \
	handlers/RequestHandler_DML.cpp \
	handlers/RequestHandler_DQL.cpp \
	handlers/RequestHandler_Rest.cpp \
	handlers/RequestHandler_TC.cpp \
	handlers/RequestHandler_UM.cpp \
	handlers/RestProtocolRowsetWriter.cpp \
	handlers/RestProtocolRowsetWriterFactory.cpp \
	handlers/SqlClientProtocolRowsetWriter.cpp \
	handlers/SqlClientProtocolRowsetWriterFactory.cpp \
	handlers/VariantOutput.cpp

CXX_HDR+= \
	handlers/JsonOutput.h \
	handlers/RequestHandler.h \
	handlers/RequestHandlerSharedConstants.h \
	handlers/RestProtocolRowsetWriter.h \
	handlers/RestProtocolRowsetWriterFactory.h \
	handlers/RowsetWriter.h \
	handlers/RowsetWriterFactory.h \
	handlers/SqlClientProtocolRowsetWriter.h \
	handlers/SqlClientProtocolRowsetWriterFactory.h \
	handlers/VariantOutput.h
