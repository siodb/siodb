#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR=$(dirname $(realpath "$0"))
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions

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

# Create test data model
database_name=db_tst_timestamp
_RunSql "create database ${database_name}"
_RunSql "create table ${database_name}.t1 ( cts1 timestamp )"
user_token=$(openssl rand -hex 64)
_RunSql "alter user root add token test_token x'${user_token}'"

# Insert random TS and check returned value
for i in {1..100}; do
  # Uncomment below when gh-104 fixed
  # CTIMESTAMP="$(date -d "${RANDOM:0:4}-01-01 12:00:00.$(date +"%N") AM + ${RANDOM:0:1} months + ${RANDOM:0:3} days + ${RANDOM:0:2} hours + ${RANDOM:0:2} minutes + ${RANDOM:0:2} seconds" +'%Y-%m-%d %0l:%0M:%0S.%N %p')"
  CTIMESTAMP="$(date -d "${RANDOM:0:4}-01-01 11:00:00.$(date +"%N") AM" +'%Y-%m-%d %0l:%0M:%0S.%N %p')"
  CTIMESTAMP_OUTPUT="$(date -d "${CTIMESTAMP}" +"%a %b %d %-Y %0l:%0M:%0S.%N %p")"
  _log "INFO" "Returned value should be: '${CTIMESTAMP_OUTPUT}'"
  _RunSql "insert into ${database_name}.t1 ( cts1 ) values ( '${CTIMESTAMP}' )"
  _RunSqlAndValidateOutput "select cts1 from ${database_name}.t1 where TRID = ${i}"  \
                         "^${CTIMESTAMP_OUTPUT}$"
done


## =============================================
## TEST FOOTER
## =============================================
_StopSiodb
_CheckLogFiles
_TestEnd
exit 0
