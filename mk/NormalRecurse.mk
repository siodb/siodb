# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Recursion through subdirs

T_SUBDIRS:=$(addsuffix /.,$(SUBDIRS))

.PHONY: $(T_SUBDIRS)

$(MAIN_TARGETS): $(T_SUBDIRS)

$(T_SUBDIRS):
	$(MAKE) $(MAKECMDGOALS) -C $@
