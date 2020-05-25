# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Recursive makefile for Siodb IO Manager unit tests

# Based on some ideas taken from
# https://stackoverflow.com/a/17845120/1540501

include ../../mk/Prolog.mk
include $(MK)/MainTargets.mk

# List of all subdirs to recurse into
SUBDIRS:= \
	builtin_cipher_test  \
	dbengine_startup_test  \
	encrypted_file_test  \
	expression_test  \
	key_generator_test  \
	registry_test  \
	request_handler_test  \
	sql_parser_test  \
	variant_test

include $(MK)/ParallelRecurse.mk
