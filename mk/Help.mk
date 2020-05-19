# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# help target for Siodb

CYAN=\033[0;36m
# No Color
NC=\033[0m

help:
	@echo "${CYAN}make all${NC} - build all Siodb targets."
	@echo "${CYAN}make siodb${NC} - build Siodb server."
	@echo "${CYAN}make siocli${NC} - build Siodb client"
	@echo "${CYAN}make conn_worker${NC} - build Siodb connection worker."
	@echo "${CYAN}make iomgr${NC} - build IO manager."
	@echo "${CYAN}make clean${NC} - clean all Siodb targets."
	@echo "${CYAN}make clean-<siodb_target>${NC} - clean specific Siodb target, for example 'clean-siodb'."
	@echo "By default, debug version is built. To build release version add DEBUG=0 to the command."
	@echo "For example: ${CYAN}make all DEBUG=0${NC}"

