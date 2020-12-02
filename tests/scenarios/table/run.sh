#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

set -e

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions

## Specific test parameters
## TODO: change to 100 once gh-113
nbOfTableToCreate=1

## =============================================
## TEST HEADER
## =============================================
_TestBegin
_Prepare
#_SetInstanceParameter "data_dir" "${SOURCE_DATA_DIR}"
_StartSiodb

## =============================================
## TEST
## =============================================

# Test data
_RunSqlScript "${SHARED_DIR}/sql/test-db1.sql" 120
_RunSqlScript "${SHARED_DIR}/sql/test-db1-table-tablealldatatypes.sql"
_RestartSiodb
_CheckLogFiles

_RunSqlAndValidateOutput "create table db1.tablealldatatypes ( col text )" \
"Status .*: Table .* already exists"

# Uncomment when gh-114 ready
# _RunSql "create table db1.tableasselect as select * from sys_databases"
# _RunSqlAndValidateOutput "select name from db1.tableasselect where name = 'SYS'" "^SYS$"

_RunSql "drop table db1.tablealldatatypes"
_RestartSiodb
_CheckLogFiles

# check if (not) exists (uncomment when gh-116 is fixed)
# _RunSql "create table if not exists db1.tablealldatatypes ( col text )"
# _RunSql "drop table db1.tablealldatatypes"
_RunSql "drop table if exists db1.tablealldatatypes"

_RunSqlAndValidateOutput "drop table db1.tablealldatatypes" \
"Status .*: Table .* doesn't exist"

_RunSqlScript "${SHARED_DIR}/sql/test-db1-table-tablealldatatypes.sql"
_RestartSiodb
_CheckLogFiles

for ((i = 0; i < ${nbOfTableToCreate}; ++i)); do
  _RunSql "create table db1.table${i} ( free_text text )"
  _RunSql "insert into db1.table${i} values ( 'free text fro db1.table${i}' )"
  _RunSqlAndValidateOutput "use database db1; show tables;" \
  "*.TABLE${i}*."
done
_RestartSiodb
_CheckLogFiles

for ((i = 0; i < ${nbOfTableToCreate}; ++i)); do
  _RunSql "drop table db1.table${i}"
done

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
