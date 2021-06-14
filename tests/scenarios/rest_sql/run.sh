#!/bin/bash

# Copyright (C) 2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"


## Specific test parameters

## =============================================
## TEST HEADER
## =============================================
_TestBegin
_Prepare
_StartSiodb

## =============================================
## TEST
## =============================================

_log "INFO" "Creating test objects"
_RunSqlScript "${SCRIPT_DIR}/create_objects.sql" 90
TOKEN=$(openssl rand -hex 64)
_RunSql "create user user1"
_RunSql "alter user user1 add token token1 x'${TOKEN}'"

# TODO: This test generated parse error and that's expected,
# but we need to treat this error as expected
#_log "INFO" "Running SQL #0 via REST (using q)"
#_RunCurlGetSqlQuery user1 ${TOKEN} 'select * from db1.t2; whatever;'

_log "INFO" "Running SQL #1 via REST (using q)"
_RunCurlGetSqlQuery user1 ${TOKEN} 'select * from db1.t1'

_log "INFO" "Running SQL #2 via REST (using q1)"
_RunCurlGetSqlQueries user1 ${TOKEN} 1 'select * from db1.t2'

_log "INFO" "Running SQL #3 via REST (multiple q1+q2)"
_RunCurlGetSqlQueries user1 ${TOKEN} 2 'select * from db1.t1' 'select * from db1.t2'

# TODO: Now this test fails because error in the iomgr log is treated unexpected
# although this is what we actually expect
# "Table 'DB1'.'TTTTT' doesn't exist"
# _log "INFO" "Running SQL #4 via REST (invalid table name)"
#_RunCurlGetSqlQuery user1 ${TOKEN} 'select * from db1.ttttt'

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
