#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR=$(dirname "$0")
source "${SCRIPT_DIR}/../share/CommonFunctions.sh"

_log "INFO" "Tests start"
_Prepare
_StartSiodb

_RunSqlScript "${SCRIPT_DIR}/data.sql" 90
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

echo "Exporting all ..."
"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -E >"${output_dir}/all.sql"

echo "Purging database ..."
_RunSql "drop database db1"
_RunSql "drop database db2"
_RunSql "drop database db3"

echo "Importing all ..."
_RunSqlScript "${output_dir}/all.sql" 90
_CheckLogFiles

echo "Exporting all ..."
"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -E >"${output_dir}/all2.sql"

echo "Comparing output ..."
diff "${output_dir}/all.sql" "${output_dir}/all2.sql"

_StopSiodb
_log "INFO" "SUCCESS: All tests passed"
