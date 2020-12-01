#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

#set -x
#set -e

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"
exitcode=0

_TestBegin
_Prepare
_StartSiodb

echo "Creating test data..."
_RunSqlScript "${SCRIPT_DIR}/data.sql"
_CheckLogFiles

echo "Exporting table..."
output_dir="${HOME}/tmp/gh-101_$(date +%s)_$$"
mkdir -p "${output_dir}"

db_name=testdb
table_name=table_with_all_data_types
"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root \
    -i "${ROOT_DIR}/tests/share/private_key" \
    -e "${db_name}.${table_name}" \
    -o "${output_dir}/${db_name}.${table_name}.sql"

exitcode=$?

_StopSiodb
_CheckLogFiles

if [[ ${exitcode} -eq 0 ]]; then
    _TestEnd
fi

exit ${exitcode}
