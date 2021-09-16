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

## Database permissions
_RunSqlAndValidateOutput \
    "grant all on database XXX to user_name with grant option" \
    "^Status [0-9]*: Unsupported statement type [0-9]*$"
# _RunSqlAndValidateOutput \
#     "revoke all on database XXX from user_name" \
#     "^Status [0-9]*: Unsupported statement type [0-9]*$"
_RunSqlAndValidateOutput \
    "grant create table on database XXX to user_name with grant option" \
    "^Status [0-9]*: Unsupported statement type [0-9]*$"
# _RunSqlAndValidateOutput \
#     "revoke create table on database XXX from user_name" \
#     "^Status [0-9]*: Unsupported statement type [0-9]*$"
_RunSqlAndValidateOutput \
    "grant alter any table on database XXX to user_name with grant option" \
    "^Status [0-9]*: Unsupported statement type [0-9]*$"
# _RunSqlAndValidateOutput \
#     "revoke alter any table on database XXX from user_name" \
#     "^Status [0-9]*: Unsupported statement type [0-9]*$"
_RunSqlAndValidateOutput \
    "grant drop any table on database XXX to user_name with grant option" \
    "^Status [0-9]*: Unsupported statement type [0-9]*$"
# _RunSqlAndValidateOutput \
#     "revoke drop any table on database XXX from user_name" \
#     "^Status [0-9]*: Unsupported statement type [0-9]*$"
_RunSqlAndValidateOutput \
    "grant show any table on database XXX to user_name with grant option" \
    "^Status [0-9]*: Unsupported statement type [0-9]*$"
# _RunSqlAndValidateOutput \
#     "revoke show any table on database XXX from user_name" \
#     "^Status [0-9]*: Unsupported statement type [0-9]*$"

## Table permissions

## Instance permissions

## Grant 1000 times the same permission and check logs

## Revoke 1000 times the same permission and check logs

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
