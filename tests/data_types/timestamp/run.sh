#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions

## Specific test parameters
numberOfTSToTest=100

## =============================================
## TEST HEADER
## =============================================
_TestBegin
_Prepare
_StartSiodb

## =============================================
## TEST
## =============================================

# Create test data model
database_name=db_tst_timestamp
_RunSql "create database ${database_name}"
user_token=$(openssl rand -hex 64)
_RunSql "alter user root add token test_token x'${user_token}'"

##
## SQL: Insert random TS and check returned value
##
table_name=t1
_RunSql "create table ${database_name}.${table_name} ( cts1 timestamp )"
for ((i = 1; i < ${numberOfTSToTest}+1; ++i)); do
    CTIMESTAMP="$(date -d "${RANDOM:0:4}-01-01 12:00:00.$(date +"%N") AM + ${RANDOM:0:1} months + ${RANDOM:0:3} days + ${RANDOM:0:2} hours + ${RANDOM:0:2} minutes + ${RANDOM:0:2} seconds" +'%Y-%m-%d %0l:%0M:%0S.%-N %p')"
    CTIMESTAMP_SIOCLI_OUTPUT="$(date -d "${CTIMESTAMP}" +"%a %b %d %-Y %0l:%0M:%0S.%N %p")"
    CTIMESTAMP_REST_OUTPUT="$(date -d "${CTIMESTAMP}" +"%-Y-%m-%d %H:%0M:%0S.%-N")"
    _log "INFO" "Returned value should be: '${CTIMESTAMP_SIOCLI_OUTPUT}'"
    _RunSql "insert into ${database_name}.${table_name} ( cts1 ) values ( '${CTIMESTAMP}' )"
     ## Check returned value
    _RunSqlAndValidateOutput "select cts1 from ${database_name}.${table_name} where TRID = ${i}"  \
                             "^${CTIMESTAMP_SIOCLI_OUTPUT}$"
    CTIMESTAMP_REST_RETURNED="$(curl -s -k \
    https://root:${user_token}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/${i} \
    2>&1 \
    | awk -F '"' '{print $10}')"
    if [[ "${CTIMESTAMP_REST_OUTPUT}" != "${CTIMESTAMP_REST_RETURNED}" ]]; then
        _log "ERROR" "REST SHOULD BE: '${CTIMESTAMP_REST_OUTPUT}' RETURNED: '${CTIMESTAMP_REST_RETURNED}'"
        _failExit
    fi
done

##
## Check for nanoseconds with prepended "0"
##
table_name=t2
_RunSql "create table ${database_name}.${table_name} ( cts1 timestamp )"
CTIMESTAMP="$(date -d "${RANDOM:0:4}-01-01 11:00:00.000000123 AM" +'%Y-%m-%d %0l:%0M:%0S.%N %p')"
CTIMESTAMP_SIOCLI_OUTPUT="$(date -d "${CTIMESTAMP}" +"%a %b %d %-Y %0l:%0M:%0S.%N %p")"
CTIMESTAMP_REST_OUTPUT="$(date -d "${CTIMESTAMP}" +"%-Y-%m-%d %H:%0M:%0S.%-N")"
_log "INFO" "Returned value should be: '${CTIMESTAMP_SIOCLI_OUTPUT}'"
_RunSql "insert into ${database_name}.${table_name} ( cts1 ) values ( '${CTIMESTAMP}' )"
 ## Check returned value
_RunSqlAndValidateOutput "select cts1 from ${database_name}.${table_name} where TRID = 1"  \
                         "^${CTIMESTAMP_SIOCLI_OUTPUT}$"


##
## REST: Insert random TS and check returned value
##
table_name=t3
_RunSql "create table ${database_name}.${table_name} ( cts1 timestamp )"
for ((i = 1; i < ${numberOfTSToTest}+1; ++i)); do
    CTIMESTAMP="$(date -d "${RANDOM:0:4}-01-01 12:00:00.$(date +"%N") AM + ${RANDOM:0:1} months + ${RANDOM:0:3} days + ${RANDOM:0:2} hours + ${RANDOM:0:2} minutes + ${RANDOM:0:2} seconds" +'%Y-%m-%d %0l:%0M:%0S.%N %p')"
    CTIMESTAMP_SIOCLI_OUTPUT="$(date -d "${CTIMESTAMP}" +"%a %b %d %-Y %0l:%0M:%0S.%N %p")"
    CTIMESTAMP_REST_INPUT="$(date -d "${CTIMESTAMP}" +"%-Y-%m-%d %H:%0M:%0S.%N")"
    CTIMESTAMP_REST_OUTPUT="$(date -d "${CTIMESTAMP}" +"%-Y-%m-%d %H:%0M:%0S.%-N")"
    STATUS=$(curl -X POST -d "[{\"cts1\": \"${CTIMESTAMP_REST_INPUT}\"}]" \
    --write-out '%{http_code}' -o /dev/null \
    "http://root:${user_token}@localhost:50080/databases/${database_name}/tables/${table_name}/rows")
    if [[ "${STATUS}" != "201" ]]; then
        _log "ERROR" "curl returned ${STATUS}"
        _failExit
    fi
    ## Check returned value
    _log "INFO" "Returned value should be: '${CTIMESTAMP_SIOCLI_OUTPUT}'"
    _RunSqlAndValidateOutput "select cts1 from ${database_name}.${table_name} where TRID = ${i}"  \
                             "^${CTIMESTAMP_SIOCLI_OUTPUT}$"
    CTIMESTAMP_REST_RETURNED="$(curl -s -k \
    https://root:${user_token}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/${i} \
    2>&1 \
    | awk -F '"' '{print $10}')"
    if [[ "${CTIMESTAMP_REST_OUTPUT}" != "${CTIMESTAMP_REST_RETURNED}" ]]; then
        _log "ERROR" "REST SHOULD BE: '${CTIMESTAMP_REST_OUTPUT}' RETURNED: '${CTIMESTAMP_REST_RETURNED}'"
        _failExit
    fi
done

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
