#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

#set -x
#SIOCLI_DEBUG=--debug

## Global
SCRIPT_DIR=$(dirname "$0")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"


## Specific test functions

## Specific test parameters

## Tests
_log "INFO" "Tests start"
_Prepare
_SetInstanceParameter "iomgr.max_json_payload_size" "10m"
#_SetInstanceParameter "log.file.severity" "debug"
_StartSiodb

### Create a database and test data model
database_name=db_gh_94_${UNIQUE_SUFFIX}
user_name=user_gh_94_${UNIQUE_SUFFIX}
_RunSql "create database ${database_name}"
_RunSql "create table ${database_name}.huge_test ( huge text )"
_RunSql "create user ${user_name}"
user_token=$(openssl rand -hex 64)
_RunSql "alter user ${user_name} add token user_test_token x'${user_token}'"

### Create a JSON with a huge text value that will span multiple blocks
test_data_dir=~/tmp/gh-94-${UNIQUE_SUFFIX}
mkdir -p "${test_data_dir}"
echo "Preparing huge JSON"
huge_json_file="${test_data_dir}/huge.json"
echo "[{ \"huge\": \"$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | head -c 8m)\" }]" \
> "${huge_json_file}"

### Post the JSON
for i in $(seq 1 3); do
  echo "Posting huge JSON: iteration #${i}"
  curl -d "@${huge_json_file}" \
  "http://${user_name}:${user_token}@localhost:50080/databases/${database_name}/tables/huge_test/rows"
  echo
done

## Validate data
_RunSql "select * from ${database_name}.huge_test"

_CheckLogFiles

### Restart instance
echo "Restarting instance..."
SIOTEST_KEEP_INSTANCE_UP_VALUE_SAVED=${SIOTEST_KEEP_INSTANCE_UP}
SIOTEST_KEEP_INSTANCE_UP=0
_StopSiodb
SIOTEST_KEEP_INSTANCE_UP=${SIOTEST_KEEP_INSTANCE_UP_VALUE_SAVED}
_StartSiodb
_CheckLogFiles

## Validate data
_RunSql "select * from ${database_name}.huge_test"
_CheckLogFiles

### Stop test
_StopSiodb
_log "INFO" "SUCCESS: Test passed"
