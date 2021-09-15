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

## Database permissions
# 1. Create database
# 2. Grant user 1 to do X on database
# 3. Test that user 1 can only do X and
#    Test that others operations are not allowed
# 4. Test that user 2 cannot do anything
# 5. Revoke user 1 to do X on database
# 6. Test that user 1 cannot do anymore
# 7. Grant user 1 to do X on database
# 8. Drop database and recreate it
# 9. Test that 1 can do nothing
# 10. Drop database

## Table permissions

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
