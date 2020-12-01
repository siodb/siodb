#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"/share/LogFunctions.sh
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_GOUPS="$(find ${SCRIPT_DIR} -maxdepth 1 -type d -not -path "./share" -not -path "${SCRIPT_DIR}" -not -path "./skeleton")"
OUTPUT_FILE="${HOME}/tmp/all_tests_$(date +%s)"
export SIODB_TEST_ALL="yes"

for d in ${TEST_GOUPS}; do
    _log "INFO" "Running test group $d"
    ${d}/run.sh "$@" | tee -a "${OUTPUT_FILE}"
done

echo ""
echo "## ======================================================="
echo "## Test results"
echo "## ======================================================="
echo ""
ERROR_COUNT=$(cat "${OUTPUT_FILE}" | egrep '\| TEST\:ERROR \|' | wc -l)
if [[ "${ERROR_COUNT}" -gt 0 ]]; then
  cat -v "${OUTPUT_FILE}" | egrep --color=never '\| TEST\:ERROR \||\| TEST\:SUCCESS \|'
  _log "ERROR" "Not all tests passed"
  exit 1
else
  cat -v "${OUTPUT_FILE}" | egrep --color=never '\| TEST\:ERROR \||\| TEST\:SUCCESS \|'
  _log "INFO" "All tests passed successfully"
  exit 0
fi

