#!/bin/bash

# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions
function test_value_from_siocli () {
    test_value_in_siocli="${1}"
    test_value_out_rest="${2}"
    _log "INFO" "SIOCLI update: >"${test_value_in_siocli}"<"
    _RunSql "update db1.table1 set ctext = '${test_value_in_siocli}' where trid = 1"
    returned=$(curl -s -k https://user1:${TOKEN}@localhost:50443/databases/db1/tables/table1/rows/1 | awk -F "CTEXT\":\"" '{print $2}' |  awk -F "\"\}" '{print $1}')
    if [[ "${returned}" != "${test_value_out_rest}" ]]; then
        _log "ERROR" "SIOCLI | Expected: >"${test_value_out_rest}"< and got >"${returned}"<"
        _failExit
    fi
    _log "INFO" "SIOCLI(SUCCESS) | Expected: >"${test_value_out_rest}"< and got >"${returned}"<"
}


function test_value_from_rest () {
    test_value_in_rest="${1}"
    test_value_out_rest="${2}"
    _log "INFO" "REST put: >"${test_value_in_rest}"<"
    curl -s -o /dev/null -X PUT -d "[{\"ctext\":\"${test_value_in_rest}\"}]" -k https://user1:${TOKEN}@localhost:50443/databases/db1/tables/table1/rows/1
    returned=$(curl -s -k https://user1:${TOKEN}@localhost:50443/databases/db1/tables/table1/rows/1 | awk -F "CTEXT\":\"" '{print $2}' |  awk -F "\"\}" '{print $1}')
    if [[ "${returned}" != "${test_value_out_rest}" ]]; then
        _log "ERROR" "REST | Expected: >"${test_value_out_rest}"< and got >"${returned}"<"
        _failExit
    fi
    _log "INFO" "REST(SUCCESS) | Expected: >"${test_value_out_rest}"< and got >"${returned}"<"
}

## Specific test parameters

## =============================================
## TEST HEADER
## =============================================
_TestBegin
_Prepare
_StartSiodb

# ## =============================================
# ## TEST
# ## =============================================


_log "INFO" "Creating test objects"
_RunSqlScript "${SCRIPT_DIR}/create_objects.sql" 90
TOKEN=$(openssl rand -hex 64)
_RunSql "create user user1"
_RunSql "alter user user1 add token token1 x'${TOKEN}'"

# # Post initial chars
_RunSql "insert into db1.table1 (ctext) values (' ')"

# test value
test_value_from_rest '' ''
test_value_from_siocli '' ''
test_value_from_rest ' ' ' '
test_value_from_siocli ' ' ' '

# Back Slash
test_value_from_siocli '\' '\\'
test_value_from_rest '\\' '\\'
test_value_from_siocli '-\-' '-\\-'
test_value_from_rest '-\\-' '-\\-'
test_value_from_siocli '\-' '\\-'
test_value_from_rest '\\-' '\\-'
test_value_from_siocli '-\' '-\\'
test_value_from_rest '-\\' '-\\'


# Double Quote
test_value_from_siocli '"' '\"'
test_value_from_rest '\"' '\"'
test_value_from_siocli '\"' '\\\"'
test_value_from_rest '\\\"' '\\\"'
test_value_from_siocli '""""""""""' '\"\"\"\"\"\"\"\"\"\"'
test_value_from_rest '\"\"\"\"\"\"\"\"\"\"' '\"\"\"\"\"\"\"\"\"\"'


# Escaped chars
test_value_from_siocli '
' '\n'
test_value_from_rest '\n' '\n'
test_value_from_siocli '\b' '\\b'
test_value_from_rest '\b' '\b'
test_value_from_siocli '\f' '\\f'
test_value_from_rest '\f' '\f'
test_value_from_siocli '\r' '\\r'
test_value_from_rest '\r' '\r'
test_value_from_siocli '\t' '\\t'
test_value_from_rest '\t' '\t'
test_value_from_siocli '\v' '\\v'
test_value_from_rest '\\v' '\\v'

# Mix
test_value_from_siocli '\t\\""\\\"
"\' '\\t\\\\\"\"\\\\\\\"\n\"\\'
test_value_from_rest '\\t\\\\\"\"\\\\\\\"\n\"\\' '\\t\\\\\"\"\\\\\\\"\n\"\\'

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
