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

# Create database db_default and check that it has the cipher id value
# of parameter 'encryption.default_cipher_id'
_GetInstanceParameter 'encryption.default_cipher_id'
encryption_default_cipher_id="${_GetInstanceParameterReturnedValue}"
_RunSql "create database db_default"
_RunSqlAndValidateOutput "select cipher_id from sys_databases
                          where name = 'DB_DEFAULT'" \
                          "^${encryption_default_cipher_id}$"

# Test all create command with all cipher IDs
SUPPORTED_CIPHER_IDS="none aes128 aes256 camellia128 camellia256"

for cipher_id in ${SUPPORTED_CIPHER_IDS}; do
  _RunSqlAndValidateOutput "create temporary database tmp_db_${cipher_id}_with_cipher_id
                            with cipher_id = '${cipher_id}'" \
                            'Status 2092: Temporary databases are not supported yet'
  _RunSqlAndValidateOutput "create temporary database tmp_db_${cipher_id}_with_cipher_id_and_key_seed
                            with cipher_id = '${cipher_id}',
                            cipher_key_seed = 'myencryptionkey${cipher_id}'" \
                            'Status 2092: Temporary databases are not supported yet'
  _RunSql "create database db_${cipher_id}_with_cipher_id
           with cipher_id = '${cipher_id}'"
  _RunSqlAndValidateOutput "select cipher_id from sys_databases
                            where name = 'DB_${cipher_id^^}_WITH_CIPHER_ID'" \
                            "^${cipher_id}$"
  _RunSql "create database db_${cipher_id}_with_cipher_id_and_key_seed
           with cipher_id = '${cipher_id}',
           cipher_key_seed = 'myencryptionkey${cipher_id}'"
  _RunSqlAndValidateOutput "select cipher_id from sys_databases
                            where name = 'DB_${cipher_id^^}_WITH_CIPHER_ID_AND_KEY_SEED'" \
                            "^${cipher_id}$"
done
_CheckLogFiles 'Temporary databases are not supported yet'


# For each database, create a table with TEXT, BIGINT and TIMESTAMP
# and insert values
COLUMNS_LIST='ctext text, cbigint bigint, ctimestamp timestamp'
CTEXT="ȑڴ˗Ӿ݁Ď襾ↆܬطց녠ƈ깉ܔⶄɈޅڭɇùԒ㖷ㄗ捫آլڨȱ蒋Ŏ͜"
CBIGINT="-4611686018427387904"
CTIMESTAMP="$(date +'%Y-%m-%d %0l:%0M:%0S.%N %p')"
CTIMESTAMP_OUTPUT="$(date +"%a %b %d %Y") ${CTIMESTAMP:11}"
for cipher_id in ${SUPPORTED_CIPHER_IDS}; do
  _RunSql "create table db_${cipher_id}_with_cipher_id.test ( ${COLUMNS_LIST} )"
  _RunSql "insert into db_${cipher_id}_with_cipher_id.test
           values ( '${CTEXT}', ${CBIGINT}, '${CTIMESTAMP}' )"
  _RunSql "create table db_${cipher_id}_with_cipher_id_and_key_seed.test ( ${COLUMNS_LIST} )"
  _RunSql "insert into db_${cipher_id}_with_cipher_id_and_key_seed.test
           values ( '${CTEXT}', ${CBIGINT}, '${CTIMESTAMP}' )"
done
_CheckLogFiles

# Restart the database
_RestartSiodb
_CheckLogFiles

## Check value from each database, create a table with TEXT, BIGINT and TIMESTAMP
for cipher_id in ${SUPPORTED_CIPHER_IDS}; do
  _RunSqlAndValidateOutput "select CTEXT from db_${cipher_id}_with_cipher_id.test" \
                           "^${CTEXT}$"
  _RunSqlAndValidateOutput "select CTEXT from db_${cipher_id}_with_cipher_id_and_key_seed.test"  \
                           "^${CTEXT}$"
  _RunSqlAndValidateOutput "select CBIGINT from db_${cipher_id}_with_cipher_id.test"  \
                           "^${CBIGINT}$"
  _RunSqlAndValidateOutput "select CBIGINT from db_${cipher_id}_with_cipher_id_and_key_seed.test"  \
                           "^${CBIGINT}$"
  _RunSqlAndValidateOutput "select CTIMESTAMP from db_${cipher_id}_with_cipher_id.test"  \
                           "^${CTIMESTAMP_OUTPUT}$"
  _RunSqlAndValidateOutput "select CTIMESTAMP from db_${cipher_id}_with_cipher_id_and_key_seed.test"  \
                           "^${CTIMESTAMP_OUTPUT}$"
done
_CheckLogFiles

# Check unknown cipher id
_RunSqlAndValidateOutput "create database db_unknown_cipher_id with cipher_id = 'unknown_cipher_id'"  \
                         "^Status 2103: Cipher .* is unknown$"
# Check database already exists
_RunSqlAndValidateOutput "create database db_default"  \
                         "^Status 2021: Database .* already exists$"
# Check database already exists
_RunSqlAndValidateOutput "create database sys"  \
                         "^Status 2021: Database .* already exists$"

### Alter database: rename
for cipher_id in ${SUPPORTED_CIPHER_IDS}; do
  _RunSqlAndValidateOutput "alter database db_${cipher_id}_with_cipher_id
                            rename to db_${cipher_id}_with_cipher_id_renamed" \
                            "^Status 6: Not implemented yet$"
  _RunSqlAndValidateOutput "alter database db_${cipher_id}_with_cipher_id_and_key_seed
                            rename if exists to db_${cipher_id}_with_cipher_id_and_key_seed_renamed" \
                            "^Status 6: Not implemented yet$"
done

### Alter database: set description
for cipher_id in ${SUPPORTED_CIPHER_IDS}; do
  _RunSqlAndValidateOutput "alter database db_${cipher_id}_with_cipher_id
                            set description = 'Description for db_${cipher_id}_with_cipher_id'" \
                            "^Status 6: Not implemented yet$"
  _RunSqlAndValidateOutput "alter database db_${cipher_id}_with_cipher_id_and_key_seed
                            set description = 'Description for db_${cipher_id}_with_cipher_id_and_key_seed'" \
                            "^Status 6: Not implemented yet$"
done

# Drop database that no exists
_RunSqlAndValidateOutput "alter database noexists set description = 'noexists'"  \
                         "^Status 6: Not implemented yet$"
_RunSqlAndValidateOutput "drop database noexists"  \
                         "^Status 2001: Database .* doesn't exist$"
_CheckLogFiles '\[2103\]|\[2021\]|\[2001\]'

### Drop each database
for cipher_id in ${SUPPORTED_CIPHER_IDS}; do
  _RunSql "drop database db_${cipher_id}_with_cipher_id"
done
_CheckLogFiles
for cipher_id in ${SUPPORTED_CIPHER_IDS}; do
  _RunSql "drop database if exists db_${cipher_id}_with_cipher_id"
  _RunSql "drop database if exists db_${cipher_id}_with_cipher_id_and_key_seed"
done
_CheckLogFiles


## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
