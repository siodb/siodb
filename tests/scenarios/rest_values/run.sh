#!/bin/bash

# Copyright (C) 2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions
function test_value_from_siocli () {
    data_type_to_test="${1^^}"
    test_value_in_siocli="${2}"
    test_value_out_rest="${3}"
    _log "INFO" "SIOCLI update: >"${test_value_in_siocli}"<"
    _RunSql "update db1.table1 set COL_${data_type_to_test} = '${test_value_in_siocli}' where trid = 1"
    query="select COL_${data_type_to_test} from db1.table1 where trid = 1"
    if [[ "${data_type_to_test}" == "TEXT" ]]; then filter='"'; else filter=''; fi
    returned=$(curl -s -k -G --data-urlencode "q=${query}" https://user1:${TOKEN}@localhost:50443/query | awk -F "COL_${data_type_to_test}\":${filter}" '{print $2}' |  awk -F "${filter}\}" '{print $1}')
    if [[ "${returned}" != "${test_value_out_rest}" ]]; then
        _log "ERROR" "SIOCLI | Expected: >"${test_value_out_rest}"< and got >"${returned}"<"
        _failExit
    fi
    _log "INFO" "SIOCLI(SUCCESS) | Expected: >"${test_value_out_rest}"< and got >"${returned}"<"
}


function test_value_from_rest () {
    data_type_to_test="${1^^}"
    test_value_in_rest="${2}"
    test_value_out_rest="${3}"
    _log "INFO" "REST put: >"${test_value_in_rest}"<"
    curl -s -o /dev/null -X PUT -d "[{\"COL_${data_type_to_test}\":\"${test_value_in_rest}\"}]" -k https://user1:${TOKEN}@localhost:50443/databases/db1/tables/table1/rows/1
    query="select COL_${data_type_to_test} from db1.table1 where trid = 1"
    if [[ "${data_type_to_test}" == "TEXT" ]]; then filter='"'; else filter=''; fi
    returned=$(curl -s -k -G --data-urlencode "q=${query}" https://user1:${TOKEN}@localhost:50443/query | awk -F "COL_${data_type_to_test}\":${filter}" '{print $2}' |  awk -F "${filter}\}" '{print $1}')
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

## =============================================
## TEST
## =============================================

_log "INFO" "Creating test objects"
_RunSqlScript "${SCRIPT_DIR}/create_objects.sql" 90
TOKEN=$(openssl rand -hex 64)
_RunSql "create user user1"
_RunSql "alter user user1 add token token1 x'${TOKEN}'"
_RunSql "grant all on db1.* to user1"

# Post initial chars
_RunSql "insert into db1.table1 (col_text, col_float, col_double) values (' ', 0, 0)"

# # test value
# test_value_from_rest text '' ''
# test_value_from_siocli text '' ''
# test_value_from_rest text ' ' ' '
# test_value_from_siocli text ' ' ' '

# # Back Slash
# test_value_from_siocli text '\' '\\'
# test_value_from_rest text '\\' '\\'
# test_value_from_siocli text '-\-' '-\\-'
# test_value_from_rest text '-\\-' '-\\-'
# test_value_from_siocli text '\-' '\\-'
# test_value_from_rest text '\\-' '\\-'
# test_value_from_siocli text '-\' '-\\'
# test_value_from_rest text '-\\' '-\\'


# # Double Quote
# test_value_from_siocli text '"' '\"'
# test_value_from_rest text '\"' '\"'
# test_value_from_siocli text '\"' '\\\"'
# test_value_from_rest text '\\\"' '\\\"'
# test_value_from_siocli text '""""""""""' '\"\"\"\"\"\"\"\"\"\"'
# test_value_from_rest text '\"\"\"\"\"\"\"\"\"\"' '\"\"\"\"\"\"\"\"\"\"'


# # # Escaped chars
# test_value_from_siocli text '
# ' '\n'
# test_value_from_rest text '\n' '\n'
# test_value_from_siocli text '\b' '\\b'
# test_value_from_rest text '\b' '\b'
# test_value_from_siocli text '\f' '\\f'
# test_value_from_rest text '\f' '\f'
# test_value_from_siocli text '\r' '\\r'
# test_value_from_rest text '\r' '\r'
# test_value_from_siocli text '\t' '\\t'
# test_value_from_rest text '\t' '\t'
# test_value_from_rest text '\u000B' '\u000B'

# # Mix
# test_value_from_siocli text '\t\\""\\\"
# "\' '\\t\\\\\"\"\\\\\\\"\n\"\\'
# test_value_from_rest text '\\t\\\\\"\"\\\\\\\"\n\"\\' '\\t\\\\\"\"\\\\\\\"\n\"\\'

# # http://www.unicode.org/emoji/charts/full-emoji-list.html
# test_value_from_siocli text '😀🤣' '😀🤣'
# test_value_from_rest text '😀🤣' '😀🤣'

## Float
test_value_from_siocli float '1.175494351E-38' '1.175494351E-38'
test_value_from_rest float '1.175494351E-38' '1.175494351E-38'
test_value_from_siocli float '3.402823466E+38' '3.402823466E+38'
test_value_from_rest float '3.402823466E+38' '3.402823466E+38'
test_value_from_rest float '"1.175494351E-38"' '1.175494351E-38'

## Double
test_value_from_siocli double '2.2250738585072014E-308' '2.2250738585072014E-308'
test_value_from_rest double '2.2250738585072014E-308' '2.2250738585072014E-308'
test_value_from_siocli double '1.7976931348623158E+308' '1.7976931348623158E+308'
test_value_from_rest double '1.7976931348623158E+308' '1.7976931348623158E+308'
test_value_from_rest double '"1.7976931348623158E+308"' '1.7976931348623158E+308'


## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
