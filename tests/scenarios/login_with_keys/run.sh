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

## Tests
_TestBegin
_Prepare
_StartSiodb

### Create a database and test data model
_RunSql "create user user_test_key"

for key in $(ls "${ROOT_DIR}/config/sample_keys/"*.pub); do
    KEY=$(cat ${key})
    PUBLIC_KEY_NAME=$(basename ${key} | awk -F "." '{print $1}')
    _RunSql "alter user user_test_key add access key ${PUBLIC_KEY_NAME} '${KEY}'"
done

### Restart instance
_RestartSiodb

### Login with keys Unix socket
for key in $(ls "${ROOT_DIR}/config/sample_keys/"*.pub); do
    PRIVATE_KEY_NAME=${key%.*}
    _RunSqlThroughUserUnixSocket "user_test_key" "${PRIVATE_KEY_NAME}" "show databases"
done

### Login with keys TCP socket
for key in $(ls "${ROOT_DIR}/config/sample_keys/"*.pub); do
    PRIVATE_KEY_NAME=${key%.*}
    _RunSqlThroughUserTCPSocket "user_test_key" "${PRIVATE_KEY_NAME}" "show databases"
done

### Stop test
_StopSiodb
_CheckLogFiles
_TestEnd
exit 0
