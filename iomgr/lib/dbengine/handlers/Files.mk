# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

CXX_SRC+= \
	handlers/RequestHandler_Common.cpp \
	handlers/RequestHandler_DDL.cpp \
	handlers/RequestHandler_DML.cpp \
	handlers/RequestHandler_DQL.cpp \
	handlers/RequestHandler_RestDelete.cpp \
	handlers/RequestHandler_RestGet.cpp \
	handlers/RequestHandler_RestPatch.cpp \
	handlers/RequestHandler_RestPost.cpp \
	handlers/RequestHandler_TC.cpp \
	handlers/RequestHandler_UM.cpp

CXX_HDR+= \
	handlers/RequestHandler.h

