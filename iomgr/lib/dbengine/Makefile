# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# IO manager library makefile

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../mk/Prolog.mk

TARGET_LIB:=iomgr_dbengine

include Files.mk

CXXFLAGS+=-I$(GENERATED_FILES_ROOT)

ANTLR_GRAMMAR:=parser/Siodb.g4

ANTLR_GENERATED_FILES_DIR:=$(THIS_GENERATED_FILES_DIR)/parser/antlr-generated

ANTLR_GENERATED_CXX_SRC:= \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbBaseListener.cpp  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbLexer.cpp  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbListener.cpp  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbParser.cpp  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbVisitor.cpp

ANTLR_GENERATED_CXX_HDR:= \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbBaseListener.h  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbLexer.h  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbListener.h  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbParser.h  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbVisitor.h

ANTLR_GENERATED_SUPPLEMENTARY_FILES:= \
	$(ANTLR_GENERATED_FILES_DIR)/Siodb.interp  \
	$(ANTLR_GENERATED_FILES_DIR)/Siodb.tokens  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbLexer.interp  \
	$(ANTLR_GENERATED_FILES_DIR)/SiodbLexer.tokens

ANTLR_GENERATED_FILES:= \
	$(ANTLR_GENERATED_CXX_SRC)  \
	$(ANTLR_GENERATED_CXX_HDR)  \
	$(ANTLR_GENERATED_SUPPLEMENTARY_FILES)

MESSAGES_HDR:=$(THIS_GENERATED_FILES_DIR)../messages/IOManagerMessageId.h

EXTRA_CXX_DEPS:= $(MESSAGES_HDR) $(ANTLR_GENERATED_CXX_HDR)

include $(MK)/Main.mk

$(MESSAGES_HDR): ../messages/iomgr.msg | $$(@D)/.
	@echo MC $@
	$(NOECHO)$(MESSAGE_COMPILER_TOOL) -H -b int -n siodb.iomgr -e IOManagerMessageId -gp -i $< -o $@

.PRECIOUS: $(MESSAGES_HDR) $(ANTLR_GENERATED_FILES)

# ANTLR build rules
ANTLR4_ROOT:=$(THIRD_PARTY_ROOT)/antlr-$(ANTLR4_VERSION)
ANTLR:=$(ANTLR4_ROOT)/bin/antlr4
ANTLR_FLAGS:=-Dlanguage=Cpp -Xexact-output-dir -visitor

$(ANTLR_GENERATED_FILES): $(ANTLR_GRAMMAR)
	$(ANTLR) $(ANTLR_FLAGS) -o $(ANTLR_GENERATED_FILES_DIR) $<
