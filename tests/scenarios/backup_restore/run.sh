#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions

## Specific test parameters
SOURCE_DATA_DIR="${SIODB_DATA_DIR}/data"
TARGET_DATA_DIR="${SIODB_DATA_DIR}/data_restored"

## =============================================
## TEST HEADER
## =============================================
_TestBegin
_Prepare
_SetInstanceParameter "data_dir" "${SOURCE_DATA_DIR}"
_StartSiodb

## =============================================
## TEST
## =============================================

# Test data

_RunSqlScript "${SHARED_DIR}/sql/test-db1-2-3.sql" 120
_StopSiodb
_CheckLogFiles

# Backup
test_data_dir=~/tmp/backup_restore-${UNIQUE_SUFFIX}
mkdir -p "${test_data_dir}"
cd ${SOURCE_DATA_DIR}
tar -zcvf ${test_data_dir}/${SIODB_INSTANCE}.tar.gz ./
rm -rf ${SOURCE_DATA_DIR}/*

# Restore
rm -rf ${TARGET_DATA_DIR}
mkdir -p ${TARGET_DATA_DIR}
_SetInstanceParameter "data_dir" "${TARGET_DATA_DIR}"
cd ${TARGET_DATA_DIR}
tar -pxvzf ${test_data_dir}/${SIODB_INSTANCE}.tar.gz
_StartSiodb
_CheckLogFiles

# Test to Select recovered data
_RunSql "show databases"
for i in {1..3}; do
_RunSql "select * from db${i}.tablealldatatypes"
_CheckLogFiles
done

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
