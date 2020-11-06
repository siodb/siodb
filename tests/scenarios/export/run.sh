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
_SetInstanceParameter "iomgr.max_json_payload_size" "10m"
_StartSiodb

_RunSqlScript "${SCRIPT_DIR}/data.sql" 120
_CheckLogFiles

output_dir="${HOME}/tmp/export_$(date +%s)"
mkdir -p "${output_dir}"

echo "Exporting db1 ..."
"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -e db1 >"${output_dir}/db1.sql"

echo "Exporting db2 ..."
"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -e db2 >"${output_dir}/db2.sql"

echo "Exporting db3 ..."
"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -e db3 >"${output_dir}/db3.sql"

# echo "Exporting db2.tablealldatatypes ..."
# "${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
#     -e db2.tablealldatatypes >"${output_dir}/db2.tablealldatatypes.sql"

### Create database db4 and test data model
database_name=db4
_RunSql "create database ${database_name}"
_RunSql "create table ${database_name}.huge_test ( huge text )"
user_token=$(openssl rand -hex 64)
_RunSql "alter user root add token test_token x'${user_token}'"


### Post medium JSON
medium_json_file="${output_dir}/medium.json"
echo "[{ \"huge\": \"$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | head -c 1m)\" }]" \
> "${medium_json_file}"
STATUS=$(curl --write-out '%{http_code}' -o ${output_dir}/medium.log -d "@${medium_json_file}" \
"http://root:${user_token}@localhost:50080/databases/${database_name}/tables/huge_test/rows")
if [[ $STATUS -ne 200 ]]; then
  _log "ERROR" "curl returned ${STATUS}"
  ${output_dir}/medium.log
  _failExit
fi

### Post Huge JSON
huge_json_file="${output_dir}/huge.json"
echo "[{ \"huge\": \"$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | head -c 5m)\" }]" \
> "${huge_json_file}"
STATUS=$(curl --write-out '%{http_code}' -o ${output_dir}/huge.log -d "@${huge_json_file}" \
"http://root:${user_token}@localhost:50080/databases/${database_name}/tables/huge_test/rows")
if [[ $STATUS -ne 200 ]]; then
  _log "ERROR" "curl returned ${STATUS}"
  cat ${output_dir}/huge.log
  _failExit
fi

echo "Exporting all ..."
"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -E >"${output_dir}/all.sql"

echo "Purging database ..."
_RunSql "drop database db1"
_RunSql "drop database db2"
_RunSql "drop database db3"
_RunSql "drop database db4"

echo "Importing all ..."
_RunSqlScript "${output_dir}/all.sql" 180
_CheckLogFiles

echo "Exporting all ..."
"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -E >"${output_dir}/all2.sql"

echo "Comparing imported and exported data..."
DIFF_COUNT=$(diff "${output_dir}/all.sql" "${output_dir}/all2.sql" | wc -l)
if [[ ${DIFF_COUNT} -gt 6 ]]; then
  echo "ERROR: Differnce bewteen imported (<) and exported data (>):"
  diff "${output_dir}/all.sql" "${output_dir}/all2.sql"
  _failExit
fi

_StopSiodb
_CheckLogFiles
_log "INFO" "SUCCESS: All tests passed"
