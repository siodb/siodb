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

## =============================================
## TEST HEADER
## =============================================
_TestBegin
_Prepare
_StartSiodb

## =============================================
## TEST
## =============================================

echo "Creating test data..."
_RunSqlScript "${SCRIPT_DIR}/data.sql"
_CheckLogFiles

echo "List databases"
_RunSql "show databases"

echo "Stopping Siodb"
_StopSiodb

echo "Exporting database..."
output_dir="${HOME}/tmp/gh-101_$(date +%s)_$$"
backup_file="${output_dir}/${SIODB_INSTANCE}.tar.gz"
mkdir -p "${output_dir}"
tar cvfz "${backup_file}" "${SIODB_DATA_DIR}"

echo "Siodb data directory state before removal"
ls -la "${SIODB_DATA_DIR}"

echo "Removing Siodb data directory"
rm -rf "${SIODB_DATA_DIR}"

echo "Extracting archived data"
tar xaf "${backup_file}" -C /

echo "Siodb data directory state after resting backup"
ls -la "${SIODB_DATA_DIR}"

echo "Starting Siodb"
_StartSiodb

echo "List databases"
_RunSql "show databases"

## =============================================
## TEST FOOTER
## =============================================
#_StopSiodb
#_CheckLogFiles
_TestEnd
exit 0
