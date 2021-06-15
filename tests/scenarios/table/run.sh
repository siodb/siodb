#!/bin/bash

# Copyright (C) 2020-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

set -e

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions

## Specific test parameters
nbOfTableToCreate=10
nbOfNextTRIDToTest=1000

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

## TODO: test rename (if exists) table once gh-11 fixed
_RunSqlAndValidateOutput "alter table db1.tablealldatatypes add column col100 text" \
"^Status .*: Not implemented yet$"
_RunSqlAndValidateOutput "alter table db1.tablealldatatypes alter column col100 rename to col200" \
"^Status .*: Not implemented yet$"
_RunSqlAndValidateOutput "alter table db1.tablealldatatypes alter column col100 rename if exists to col200" \
"^Status .*: Not implemented yet$"
_RunSqlAndValidateOutput "alter table db1.tablealldatatypes alter column col100 bigint" \
"^Status .*: Not implemented yet$"
_RunSqlAndValidateOutput "alter table db1.tablealldatatypes drop column col200" \
"^Status .*: Not implemented yet$"
_CheckLogFiles

# Attribute test
for ((i = 99999999; i < $((${nbOfNextTRIDToTest}*99999999)); i=i+99999999)); do
  _RunSql "alter table db1.tablealldatatypes set next_trid = ${i}"
  _CheckLogFiles
done
_CheckLogFiles

_RunSqlAndValidateOutput "create table db1.tablealldatatypes ( col text )" \
"Status .*: Table .* already exists"
_CheckLogFiles '\[2022\]'

# Uncomment when gh-26 ready
# _RunSql "create table db1.tableasselect as select * from sys_databases"
# _RunSqlAndValidateOutput "select name from db1.tableasselect where name = 'SYS'" "^SYS$"

_RunSql "drop table db1.tablealldatatypes"
_RestartSiodb
_CheckLogFiles

# check if (not) exists
_RunSql "create table if not exists db1.tablealldatatypes ( col text )"
_RunSql "drop table db1.tablealldatatypes"
_RunSql "drop table if exists db1.tablealldatatypes"

_RunSqlAndValidateOutput "drop table db1.tablealldatatypes" \
"Status .*: Table .* doesn't exist"
_CheckLogFiles '\[2002\]'

_RunSqlScript "${SHARED_DIR}/sql/test-db1-table-tablealldatatypes.sql"
_RestartSiodb
_CheckLogFiles

# Scalling test
for ((i = 0; i < ${nbOfTableToCreate}; ++i)); do
  _RunSql "create table db1.table${i} ( free_text text )"
  _RunSql "insert into db1.table${i} values ( 'free text fro db1.table${i}' )"
  _RunSqlAndValidateOutput "use database db1; show tables;" \
  "^TABLE${i}*."
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
