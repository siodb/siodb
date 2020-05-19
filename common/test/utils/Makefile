# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Recursive makefile for Siodb common code "utils" unit tests

# Based on some ideas taken from
# https://stackoverflow.com/a/17845120/1540501

include ../../../mk/Prolog.mk
include $(MK)/MainTargets.mk

# List of all subdirs to recurse into
SUBDIRS:= lru_cache_test plain_binary_encoding_test string_scanner_test

include $(MK)/ParallelRecurse.mk
