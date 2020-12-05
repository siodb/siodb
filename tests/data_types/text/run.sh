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

## =============================================
## TEST HEADER
## =============================================
_TestBegin
_Prepare
_StartSiodb

## =============================================
## TEST
## =============================================

output_dir="${HOME}/tmp/export_$(date +%s)_$$"
mkdir -p "${output_dir}"

# Create test data model
database_name=db1
_RunSql "create database ${database_name}"
_RunSql "create table ${database_name}.t1 ( ctext text )"
user_token=$(openssl rand -hex 64)
_RunSql "alter user root add token test_token x'${user_token}'"

# insert utf8 string
_RunSql "insert into ${database_name}.t1 values ( '' )"

# Read utf8 string with REST
STATUS=$(curl --write-out '%{http_code}' -o ${output_dir}/output.log \
"http://root:${user_token}@localhost:50080/databases/${database_name}/tables/t1/rows")
if [[ $STATUS -ne 200 ]]; then
  _log "ERROR" "curl returned ${STATUS}"
  ${output_dir}/medium.log
  _failExit
fi

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
