#!/bin/bash

# Copyright (C) 2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## =============================================
## TEST HEADER
## =============================================
_TestBegin
_Prepare
SOURCE_DATA_DIR="${HOME}/tmp/database_uuid_$(date +%s)_$$"
_SetInstanceParameter "data_dir" "${SOURCE_DATA_DIR}"
_StartSiodb

## =============================================
## TEST
## =============================================

# Verify OK on creating db with UUID
UUID="$(cat /proc/sys/kernel/random/uuid)"
_RunSql "create database DB1
        with uuid = '${UUID}'"
_RunSqlAndValidateOutput "select UUID from sys_databases where NAME = 'DB1'" \
                         "^${UUID}$"

# Verify OK on creating db when directory NOT exists
# and data_directory_must_exist is set to false
UUID="$(cat /proc/sys/kernel/random/uuid)"
_RunSql "create database DB2
        with uuid = '${UUID}',
        data_directory_must_exist = false"
_RunSqlAndValidateOutput "select UUID from sys_databases where NAME = 'DB2'" \
                         "^${UUID}$"

# Verify OK on creating db when directory exists
# and data_directory_must_exist is set to false
UUID="$(cat /proc/sys/kernel/random/uuid)"
mkdir "${SOURCE_DATA_DIR}/db-${UUID}"
_RunSql "create database DB3
         with uuid = '${UUID}',
         data_directory_must_exist = false"
_RunSqlAndValidateOutput "select UUID from sys_databases where NAME = 'DB3'" \
                         "^${UUID}$"

# Verify OK on creating db when directory exists
# and data_directory_must_exist is set to true
UUID="$(cat /proc/sys/kernel/random/uuid)"
mkdir "${SOURCE_DATA_DIR}/db-${UUID}"
_RunSql "create database DB4
        with uuid = '${UUID}',
        data_directory_must_exist = true"
_RunSqlAndValidateOutput "select UUID from sys_databases where NAME = 'DB4'" \
                         "^${UUID}$"

# Verify ERROR for kErrorDatabaseUuidAlreadyExists
_RunSqlAndValidateOutput "create database DB5
                          with uuid = '${UUID}',
                          data_directory_must_exist = true" \
                         "Status .*: Database with UUID .* already exists"
_CheckLogFiles "Database with UUID .* already exists"

# Verify ERROR for kErrorDatabaseDataDirDoesNotExist
UUID="$(cat /proc/sys/kernel/random/uuid)"
_RunSqlAndValidateOutput "create database DB6
                          with uuid = '${UUID}',
                          data_directory_must_exist = true" \
                         "Status .*: Data directory .* for the database .* doesn't exist"
_CheckLogFiles "Data directory .* for the database .* doesn't exist"

# Verify ERROR for kErrorInvalidAttributeValue "UUID"
UUID="abcdefghijklmnop"
_RunSqlAndValidateOutput "create database DB7 with uuid = '${UUID}'" \
                         "Status .*: Invalid value of the attribute .*"
_CheckLogFiles "Invalid value of the attribute .*"

# Verify ERROR for kErrorWrongAttributeType, "UUID", "STRING"
UUID="abcdefghijklmnop"
_RunSqlAndValidateOutput "create database DB8 with uuid = 9" \
                         "Status .*: Attribute .* must be 'STRING'"
_CheckLogFiles "Attribute .* must be 'STRING'"

# Verify ERROR for kErrorWrongAttributeType,
# "DATA_DIRECTORY_MUST_EXIST", "BOOLEAN"
UUID="$(cat /proc/sys/kernel/random/uuid)"
_RunSqlAndValidateOutput "create database DB9
                          with uuid = '${UUID}',
                          data_directory_must_exist = 'abcdefghijklmnop'" \
                         "Status .*: Attribute .* must be 'BOOLEAN'"
_CheckLogFiles "Attribute .* must be 'BOOLEAN'"

# Verify ERROR for wrong permissions on directory
UUID="$(cat /proc/sys/kernel/random/uuid)"
mkdir "${SOURCE_DATA_DIR}/db-${UUID}"
chmod 550 "${SOURCE_DATA_DIR}/db-${UUID}"
_RunSqlAndValidateOutput "create database DB10
                          with uuid = '${UUID}',
                          data_directory_must_exist = true" \
                         "Status .*: Can't create metadata file .* Permission denied"
_CheckLogFiles "error .* Permission denied"

# purging
for db in db1 db2 db3 db4 db5 db6 db7 db8 db9 db10; do
    _RunSql "drop database if exists ${db}"
done

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
