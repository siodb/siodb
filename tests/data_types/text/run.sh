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
_log "INFO" "Tests start"
_Prepare
_StartSiodb

## =============================================
## TEST
## =============================================
echo "TODO"

## =============================================
## TEST FOOTER
## =============================================
_StopSiodb
_CheckLogFiles
_log "INFO" "SUCCESS: Test passed"
exit 0