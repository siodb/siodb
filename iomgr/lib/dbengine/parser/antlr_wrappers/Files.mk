# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

CXX_SRC+= \
	parser/antlr_wrappers/SiodbBaseListenerWrapper.cpp \
	parser/antlr_wrappers/SiodbLexerWrapper.cpp \
	parser/antlr_wrappers/SiodbListenerWrapper.cpp \
	parser/antlr_wrappers/SiodbParserWrapper.cpp \
	parser/antlr_wrappers/SiodbVisitorWrapper.cpp

CXX_HDR+= \
	parser/antlr_wrappers/Antlr4RuntimeWrapper.h \
	parser/antlr_wrappers/SiodbBaseListenerWrapper.h \
	parser/antlr_wrappers/SiodbLexerWrapper.h \
	parser/antlr_wrappers/SiodbListenerWrapper.h \
	parser/antlr_wrappers/SiodbParserWrapper.h \
	parser/antlr_wrappers/SiodbVisitorWrapper.h
