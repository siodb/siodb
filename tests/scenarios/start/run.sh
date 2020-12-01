#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR=$(dirname $(realpath "$0"))
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Program
_TestBegin
_Prepare
_StartSiodb
_CheckLogFiles

echo "Siodb started."

_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
