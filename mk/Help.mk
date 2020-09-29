# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# help target for Siodb

CYAN=\033[0;36m
# No Color
NC=\033[0m

help:
	@echo "${CYAN}make all${NC} - Build all targets."
	@echo "${CYAN}make conn_worker${NC} - Build Connection Worker."
	@echo "${CYAN}make iomgr${NC} - Build IO Manager."
	@echo "${CYAN}make siodb${NC} - Build main server executable."
	@echo "${CYAN}make rest_server${NC} - Build REST Server"
	@echo "${CYAN}make siocli${NC} - Build command-line client"
	@echo "${CYAN}make restcli${NC} - Build REST protocol test client"
	@echo "${CYAN}make clean${NC} - Clean all targets."
	@echo "${CYAN}make clean-<siodb_target>${NC} - Clean specific target, for example 'clean-siodb'."
	@echo "By default, debug version is built. To build release version add DEBUG=0 to the command."
	@echo "For example: ${CYAN}make all DEBUG=0${NC}"

