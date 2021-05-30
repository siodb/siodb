#!/bin/bash

# Copyright (C) 2020-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
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
    echo "Connecting via UNIX socket using ${PRIVATE_KEY_NAME}"
    _RunSqlThroughUserUnixSocket "user_test_key" "${PRIVATE_KEY_NAME}" "show databases"
done

### Login with keys TCP socket
for key in $(ls "${ROOT_DIR}/config/sample_keys/"*.pub); do
    PRIVATE_KEY_NAME=${key%.*}
    echo "Connecting via TCP socket using ${PRIVATE_KEY_NAME}"
    _RunSqlThroughUserTCPSocket "user_test_key" "${PRIVATE_KEY_NAME}" "show databases"
done

### Stop test
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
