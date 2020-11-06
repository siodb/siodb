#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

#set -x
#set -e

## Global
SCRIPT_DIR=$(dirname $(realpath "$0"))
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"
exitcode=0

_TestBegin
_Prepare
_StartSiodb

_RunSqlScript "${SCRIPT_DIR}/data.sql"
_CheckLogFiles

echo "Exporting database..."
output_dir="${HOME}/tmp/gh-101_$(date +%s)"
mkdir -p "${output_dir}"

dbname=testdb
"${SIODB_BIN}/siocli" -a ${SIODB_INSTANCE} -u root \
    -i "${ROOT_DIR}/tests/share/private_key" \
    -e "${dbname}" -o "${output_dir}/${dbname}.sql"

exitcode=$?

_StopSiodb

if [[ ${exitcode} -eq 0 ]]; then
    _TestEnd
fi

exit ${exitcode}
