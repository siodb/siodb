#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
source $(dirname "$0")/../share/CommonFunctions.sh

## Specific test functions

## Specific test parameters
siodbUserTridStartingAt=4096
if [[ -z "${numberOfUsersToTest}" ]]; then
    numberOfUsersToTest=10
fi
if [[ -z "${numberOfUsersToTestMax}" ]]; then
    numberOfUsersToTestMax=10
fi
if [[ -z "${numberOfKeysToTest}" ]]; then
    numberOfKeysToTest=10
fi
if [[ -z "${numberOfTokensToTest}" ]]; then
    numberOfTokensToTest=10
fi

## Tests
_log "INFO" "Tests start"
_Prepare
_SetInstanceParameter "iomgr.max_json_payload_size" "10m"
_StartSiodb

### Create a database and test data model
_RunSql "create database test_huge_text_value"
_RunSql "create table test_huge_text_value.test_huge_text_value_table ( huge_text_col text )"
_RunSql "create user user_test"
USERTOKEN=$(openssl rand -hex 64)
_RunSql "alter user user_test add token user_test_token x'${USERTOKEN}'"

### Create a JSON with a huge text value that will span multiple blocks
echo "[{ \"huge_text_col\": \"$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | head -c 8m)\" }]" \
> /tmp/test_huge_text_value_table.json

### Post the JSON
curl -X POST -d @/tmp/test_huge_text_value_table.json \
http://user_test:${USERTOKEN}@localhost:50080/databases/test_huge_text_value/tables/test_huge_text_value_table/rows
curl -X POST -d @/tmp/test_huge_text_value_table.json \
http://user_test:${USERTOKEN}@localhost:50080/databases/test_huge_text_value/tables/test_huge_text_value_table/rows
curl -X POST -d @/tmp/test_huge_text_value_table.json \
http://user_test:${USERTOKEN}@localhost:50080/databases/test_huge_text_value/tables/test_huge_text_value_table/rows

_CheckLogFiles

### Restart instance
_StopSiodb
_StartSiodb

### Stop test
_StopSiodb
_log "INFO" "SUCCESS: All tests passed"
