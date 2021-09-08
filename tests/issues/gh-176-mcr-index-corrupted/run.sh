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

_RunSql "create database db_default;"
_RunSqlScript "${SCRIPT_DIR}/script.sql" 120
_CheckLogFiles
_RunSql "select * from db_default.sys_columns;"
_CheckLogFiles
_RunSql "drop database db_default;"
_CheckLogFiles

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
