#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR=$(dirname "$0")
source "${SCRIPT_DIR}/tests/share/CommonFunctions.sh"

_log "INFO" "Tests start"
_Prepare
_StartSiodb

_RunSqlScript "${SCRIPT_DIR}/data.sql"
_CheckLogFiles

echo "Exporting database..."
output_dir="${HOME}/tmp/export_$(date +%s)"
mkdir -p "${output_dir}"

"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -e db1 >"${output_dir}/db1.sql"

"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -e db2 >"${output_dir}/db2.sql"

"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -e db2.t1_db2 >"${output_dir}/db2.t1_db2.sql"

"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root -i "${ROOT_DIR}/tests/share/private_key" \
    -E >"${output_dir}/all.sql"

_StopSiodb
_log "INFO" "SUCCESS: All tests passed"
