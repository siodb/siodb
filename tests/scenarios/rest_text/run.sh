#!/bin/bash

# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions
function test_value () {
    test_value_siocli="${1}"
    test_value_rest="${2}"
    # PUT chars siocli / get char curl / compare
    _log "INFO" "SIOCLI update: >"${test_value_siocli}"<"
    _RunSql "update db1.table1 set ctext = '${test_value_siocli}' where trid = 1"
    returned=$(curl -s -k https://user1:${TOKEN}@localhost:50443/databases/db1/tables/table1/rows/1 | awk -F "CTEXT\":\"" '{print $2}' |  awk -F "\"\}" '{print $1}')

    curl -k https://user1:${TOKEN}@localhost:50443/databases/db1/tables/table1/rows/1
    if [[ "${returned}" != "${test_value_rest}" ]]; then
        _log "ERROR" "SIOCLI | Expected: >"${test_value_rest}"< and got >"${returned}"<"
        _failExit
    fi
    _log "INFO" "SIOCLI(SUCCESS) | Expected: >"${test_value_rest}"< and got >"${returned}"<"
    # PUT chars curl / get char curl / compare
    _log "INFO" "REST put: >"${test_value_rest}"<"
    curl -s -o /dev/null -X PUT -d "[{\"ctext\":\"${test_value_rest}\"}]" -k https://user1:${TOKEN}@localhost:50443/databases/db1/tables/table1/rows/1
    returned=$(curl -s -k https://user1:${TOKEN}@localhost:50443/databases/db1/tables/table1/rows/1 | awk -F "CTEXT\":\"" '{print $2}' |  awk -F "\"\}" '{print $1}')
    if [[ "${returned}" != "${test_value_rest}" ]]; then
        _log "ERROR" "REST | Expected: >"${test_value_rest}"< and got >"${returned}"<"
        _failExit
    fi
    _log "INFO" "REST(SUCCESS) | Expected: >"${test_value_rest}"< and got >"${returned}"<"
}

## Specific test parameters

## =============================================
## TEST HEADER
## =============================================
# _TestBegin
# _Prepare
# _StartSiodb

# ## =============================================
# ## TEST
# ## =============================================


# _log "INFO" "Creating test objects"
# _RunSqlScript "${SCRIPT_DIR}/create_objects.sql" 90
# TOKEN=$(openssl rand -hex 64)
TOKEN="cfd5265d75fa0a6c4a22e9a230442d82a145badb3591b76879d243f8a1973738078516844fa9bf83d4cc8e127b1395067e727fa8e4c96c684254e4b8052be90f"
# _RunSql "create user user1"
# _RunSql "alter user user1 add token token1 x'${TOKEN}'"

# # Post initial chars
# _RunSql "insert into db1.table1 (ctext) values (' ')"

# test value
test_value '' ''
test_value ' ' ' '
test_value '\' '\\'
test_value '-\-' '-\\-'
test_value '\-' '\\-'
test_value '-\' '-\\'

test_value '"' '\"'
# test_value '\"' '\"' {"error":"Can't write message to IOMgr: write tcp 127.0.0.1:47442-\u003e127.0.0.1:50002: write: broken pipe"}
test_value '"' '\"'


test_value '\b' '\b'
test_value '\f' '\f'
test_value '\r' '\r'
test_value '\t' '\t'
test_value '\r' '\r'


## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
