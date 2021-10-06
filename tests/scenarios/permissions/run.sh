#!/bin/bash

# Copyright (C) 2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
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

## Create test users
_RunSql "create database TEST_DB1"
_RunSql "create table TEST_DB1.TABLE_01 (COL1 TEXT)"
_RunSql "create user TEST_USER1"
_RunSql "create user TEST_USER2"

## Instance permissions
instance_permissions="create-database drop-any-database alter-any-database attach-database detach-any-database create-user alter-user drop-user create-user-access-key alter-user-access-key drop-user-access-key create-user-access-token alter-user-access-token drop-user-access-token"
for permission in ${instance_permissions}; do
    permission="$(echo ${permission}|tr '-' ' ')"
    _log "INFO" "Testing table permission '${permission}'"
    _RunSqlAndValidateOutput "grant create database to TEST_USER1" \
                             "^Status [0-9]*: Unsupported statement type [0-9]*$"
    _RunSqlAndValidateOutput "grant create database to TEST_USER1 with grant option" \
                             "^Status [0-9]*: Unsupported statement type [0-9]*$"
    _RunSqlAndValidateOutput "revoke create database from TEST_USER1" \
                             "^Status [0-9]*: Unsupported statement type [0-9]*$"
    _CheckLogFiles "SQL parse error: Unsupported statement type [0-9]*"
done

## Database permissions
database_permissions="all create-table alter-any-table drop-any-table show-any-table"
for permission in ${database_permissions}; do
    permission="$(echo ${permission}|tr '-' ' ')"
    _log "INFO" "Testing table permission '${permission}'"
    _RunSqlAndValidateOutput "grant all on database TEST_DB1 to TEST_USER1n" \
                             "^Status [0-9]*: Unsupported statement type [0-9]*$"
    _RunSqlAndValidateOutput "grant all on database TEST_DB1 to TEST_USER1 with grant option" \
                             "^Status [0-9]*: Unsupported statement type [0-9]*$"
    _RunSqlAndValidateOutput "revoke all on database TEST_DB1 from TEST_USER1" \
                             "^Status [0-9]*: Unsupported statement type [0-9]*$"
    _CheckLogFiles "SQL parse error: Unsupported statement type [0-9]*"
done

## Table permissions
table_permissions="all read_only read_write select insert update delete drop alter show"
for permission in ${table_permissions}; do
    permission="$(echo ${permission}|tr '-' ' ')"
    _log "INFO" "Testing table permission '${permission}'"
    _RunSql "grant all on TEST_DB1.TABLE_01 to TEST_USER1"
    _RunSql "use database TEST_DB1; grant all on TABLE_01 to TEST_USER1"
    _RunSql "grant all on table TEST_DB1.TABLE_01 to TEST_USER1 with grant option"
    _RunSql "use database TEST_DB1; grant all on table TABLE_01 to TEST_USER1 with grant option"
    _RunSql "grant all on TEST_DB1.TABLE_01 to TEST_USER1"
    _RunSql "use database TEST_DB1; grant all on TABLE_01 to TEST_USER1"
    _RunSql "grant all on table TEST_DB1.TABLE_01 to TEST_USER1 with grant option"
    _RunSql "use database TEST_DB1; grant all on table TABLE_01 to TEST_USER1 with grant option"
    _RunSql "revoke all on TEST_DB1.TABLE_01 from TEST_USER1"
    _RunSql "use database TEST_DB1; revoke all on TABLE_01 from TEST_USER1"
    _RunSql "revoke all on table TEST_DB1.TABLE_01 from TEST_USER1"
    _RunSql "use database TEST_DB1; revoke all on table TABLE_01 from TEST_USER1"
    _CheckLogFiles
done

## Grant 1000 times the same permission and check logs

## Revoke 1000 times the same permission and check logs

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
