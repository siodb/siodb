# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Recursive makefile for iomgr

include ../mk/Prolog.mk
include $(MK)/MainTargets.mk

$(MAIN_TARGETS):
	$(MAKE) $(MAKECMDGOALS) -C lib
	$(MAKE) $(MAKECMDGOALS) -C app
	$(MAKE) $(MAKECMDGOALS) -C test
