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
### 1. Create database
#### Loop:
#### 1. Grant user 1 to do X on database
grant create table on database XXX to user_name
grant drop any table on database XXX to user_name
grant alter any table on database XXX to user_name
grant show any table on database XXX to user_name
grant all on database XXX to user_name
#### 2. Test that user 1 can only do X and
#### 3. Test that others operations are not allowed
#### 4. Test that user 2 cannot do anything
#### 5. Test that user ROOT can do everything
#### 6. Revoke user 1 to do X on database
#### 7. Test that user 2 cannot do anything
#### 8. Test that user ROOT can do everything
#### 9. Test that user 1 cannot do anymore
### 2. Grant all to user 1 to do X on database
### 8. Drop database and recreate it
### 9. Test that 1 can do nothing
### 10. Drop database

## Table permissions

## Grant 1000 times the same permission and check logs

## Revoke 1000 times the same permission and check logs

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
