# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Recursive makefile for fake_rest_server

include ../mk/Prolog.mk
include $(MK)/MainTargets.mk

ifeq ($(BUILD_UNIT_TESTS),1)

$(MAIN_TARGETS):
	$(MAKE) $(MAKECMDGOALS) -C app

else

$(MAIN_TARGETS):
	$(MAKE) $(MAKECMDGOALS) -C fake_app

endif
