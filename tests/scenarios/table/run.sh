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
nbOfTableToCreate=100

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

_RunSql "drop table db1.tablealldatatypes"
_RestartSiodb
_CheckLogFiles

_RunSqlScript "${SHARED_DIR}/sql/test-db1-table-tablealldatatypes.sql"
_RestartSiodb
_CheckLogFiles

for ((i = 0; i < ${nbOfTableToCreate}; ++i)); do
  _RunSql "create table db1.table${i} ( free_text text )"
  _RunSql "insert into table db1.table${i} ( 'free text fro db1.table${i}' )"
done
_RestartSiodb
_CheckLogFiles

for ((i = 0; i < ${nbOfTableToCreate}; ++i)); do
  _RunSql "drop table db1.table${i} ( free_text text )"
done

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
