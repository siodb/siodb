# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Makefile for the common library "proto"

SRC_DIR:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
include ../../../../../mk/Prolog.mk

TARGET_COMMON_LIB:=siodb_common_proto

PROTO_SRC:= \
	ClientProtocol.proto  \
	ColumnDataType.proto  \
	CommonTypes.proto  \
	IOManagerProtocol.proto  \
	SystemTables.proto \

include $(MK)/Main.mk

# Extra dependencies
$(THIS_OBJ_DIR)/ClientProtocol.pb.o: \
	$(THIS_GENERATED_FILES_DIR)/ColumnDataType.pb.h  \
	$(THIS_GENERATED_FILES_DIR)/CommonTypes.pb.h

$(THIS_OBJ_DIR)/CommonTypes.pb.o: $(THIS_GENERATED_FILES_DIR)/ColumnDataType.pb.h

$(THIS_OBJ_DIR)/IOManagerProtocol.pb.o: \
	$(THIS_GENERATED_FILES_DIR)/ColumnDataType.pb.h  \
	$(THIS_GENERATED_FILES_DIR)/CommonTypes.pb.h

$(THIS_OBJ_DIR)/SystemTables.pb.o: $(THIS_GENERATED_FILES_DIR)/ColumnDataType.pb.h
