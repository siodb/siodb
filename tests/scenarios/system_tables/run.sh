#!/bin/bash

# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
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

_RunSql "create database test_system_table"
_RunSqlAndValidateOutput "insert into sys_tables (name) values ('PROHIBITED')" "^Status "
_RunSqlAndValidateOutput "insert into test_system_table.sys_tables (name) values ('PROHIBITED')" "^Status "
_RunSqlAndValidateOutput "update sys_tables set name = 'PROHIBITED'" "^Status "
_RunSqlAndValidateOutput "update test_system_table.sys_tables set name = 'PROHIBITED'" "^Status "
_RunSqlAndValidateOutput "update sys_tables set name = 'PROHIBITED' where trid = 1" "^Status "
_RunSqlAndValidateOutput "update test_system_table.sys_tables set name = 'PROHIBITED' where trid = 1" "^Status "
_RunSqlAndValidateOutput "delete from sys_tables where trid = 1" "^Status "
_RunSqlAndValidateOutput "delete from test_system_table.sys_tables where trid = 1" "^Status "
_RunSqlAndValidateOutput "delete from sys_tables" "^Status "
_RunSqlAndValidateOutput "delete from test_system_table.sys_tables" "^Status "
_RunSqlAndValidateOutput "drop table sys_tables" "^Status "
_RunSqlAndValidateOutput "drop table test_system_table.sys_tables" "^Status "
_CheckLogFiles "is not allowed$|Can't drop system table$"

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
