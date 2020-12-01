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

# YOUR TEST HERE

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
