#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
current_directory="$(dirname "$0")"
. ${current_directory}/../global_functions.sh

## Specific test functions


## Test
_log "INFO" "Tests start"
_Prepare
_StartSiodb
_CheckLogFiles

_RunSql "select * from SYS.SYS_USERS"
_CheckLogFiles

_RunSql "SELECT * FROM SYS.SYS_USER_ACCESS_KEYS"
_CheckLogFiles

_RunSql "SELECT * FROM SYS.SYS_USER_PERMISSIONS"
_CheckLogFiles

_RunSql "SELECT * FROM SYS.SYS_USER_TOKENS"
_CheckLogFiles

_StopSiodb
_CheckLogFiles
_log "INFO" "All tests passed"
