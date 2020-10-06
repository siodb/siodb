# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Recursive makefile for tools
# Only tools required for build are included in this Makefile

include ../mk/Prolog.mk
include $(MK)/MainTargets.mk

$(MAIN_TARGETS):
	$(MAKE) $(MAKECMDGOALS) -C message_compiler
	$(MAKE) $(MAKECMDGOALS) -C generate_version_go
