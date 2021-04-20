#!/bin/bash

# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions

## Specific test parameters
siodbUserTridStartingAt=4096
if [[ -z "${numberOfUsersToTest}" ]]; then
    numberOfUsersToTest=10
fi
if [[ -z "${numberOfUsersToTestMax}" ]]; then
    numberOfUsersToTestMax=10
fi
if [[ -z "${numberOfKeysToTest}" ]]; then
    numberOfKeysToTest=10
fi
if [[ -z "${numberOfTokensToTest}" ]]; then
    numberOfTokensToTest=10
fi

## Tests
_TestBegin
_Prepare
_StartSiodb

### Starting point users
_RunSqlAndValidateOutput "select NAME from SYS.SYS_USERS" 'ROOT'

## -------------------------------------------------------------------------
## Users
## -------------------------------------------------------------------------

#### Create users
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_1_${i}"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_2_${i}
         with STATE = ACTIVE"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_3_${i}
         with STATE = INACTIVE"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_4_${i}
         with REAL_NAME = 'TestUser ${i} 😀'"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_5_${i}
         with DESCRIPTION = 'User for developer ${i} 😀'"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_6_${i}
         with STATE = ACTIVE, REAL_NAME = 'TestUser ${i} 😀'"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_7_${i}
         with STATE = ACTIVE, DESCRIPTION = 'User for developer ${i} 😀'"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_8_${i}
         with REAL_NAME = 'TestUser ${i} 😀', DESCRIPTION = 'User for developer ${i} 😀'"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_9_${i}
         with STATE = ACTIVE, REAL_NAME = 'TestUser ${i} 😀', DESCRIPTION = 'User for developer ${i} 😀'"
done
#### Create max users
for ((i = $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); i < $(($((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1))+${numberOfUsersToTestMax}+1)); ++i)); do
_RunSql "create user user_test_10_${i}
         with STATE = ACTIVE, REAL_NAME = 'TestUser ${i} 😀', DESCRIPTION = 'User for developer ${i} 😀'"
done
#### Alter user
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i}
         set STATE = ACTIVE"
_RunSqlAndValidateOutput "select STATE from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" "1"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i}
         set STATE = INACTIVE"
_RunSqlAndValidateOutput "select STATE from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" "0"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i}
         set STATE = ACTIVE, REAL_NAME = 'TestUser updated 1 ${i} 😀'"
_RunSqlAndValidateOutput "select STATE from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" "1"
_RunSqlAndValidateOutput "select REAL_NAME from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" "TestUser updated 1 ${i} 😀"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i}
         set STATE = ACTIVE, REAL_NAME = 'TestUser updated 2 ${i} 😀', DESCRIPTION = 'User for developer updated 2 ${i} 😀'"
_RunSqlAndValidateOutput "select STATE from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" "1"
_RunSqlAndValidateOutput "select REAL_NAME from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" "TestUser updated 2 ${i} 😀"
_RunSqlAndValidateOutput "select DESCRIPTION from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" "User for developer updated 2 ${i} 😀"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i}
         set REAL_NAME = 'TestUser updated 3 ${i} 😀', DESCRIPTION = 'User for developer updated 3 ${i} 😀'"
_RunSqlAndValidateOutput "select REAL_NAME from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" "TestUser updated 3 ${i} 😀"
_RunSqlAndValidateOutput "select DESCRIPTION from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" "User for developer updated 3 ${i} 😀"
done

#### Create user with wrong chars
_RunSqlAndValidateOutput "create user 😀😀😀😀😀😀😀" 'Status .*: at .*: mismatched input'
_CheckLogFiles
_RunSqlAndValidateOutput "create user 123456789" 'Status .*: at .*: mismatched input'
_CheckLogFiles

## -------------------------------------------------------------------------
## Users keys
## -------------------------------------------------------------------------
SIOKEY1="$(cat ${SCRIPT_DIR}/../../share/public_key)"

#### Add key
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} add access key key1 '${SIOKEY1}'"
done
#### Add 2nd key
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} add access key key2 '${SIOKEY1}'"
done
#### Add 100 keys to one user
for ((i = 3; i < $((3+${numberOfKeysToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${siodbUserTridStartingAt} add access key key${i} '${SIOKEY1}'"
done
#### Alter 2nd key
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} alter access key key2 set STATE = INACTIVE, DESCRIPTION = 'TEST INACTIVE 😀'"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} alter access key key2 set STATE = ACTIVE, DESCRIPTION = 'TEST ACTIVE 😀'"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} alter access key key1 set STATE = INACTIVE"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} alter access key key1 set STATE = ACTIVE"
done
#### Drop access key 1 for all users
_RunSql "alter user user_test_1_4096 drop access key key1"
for ((i = ${siodbUserTridStartingAt}+1; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} drop access key if exists key1"
done
### Try dropping non-existing access keys with "IF EXISTS"
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
  _RunSql "alter user user_test_1_${i} drop access key if exists NOEXISTS"
done

#### Expected error with keys
_RunSqlAndValidateOutput "alter user user_test${siodbUserTridStartingAt} alter access key key2 rename to key3 " 'Status .*: Not implemented yet'
_CheckLogFiles
_RunSqlAndValidateOutput "alter user user_test${siodbUserTridStartingAt} alter access key key2 rename if exists to key3" 'Status .*: Not implemented yet'
_CheckLogFiles

## -------------------------------------------------------------------------
### Users Token creation
## -------------------------------------------------------------------------

#### Add Token to all users
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
  _RunSql "alter user user_test_1_${i} add token user_token_1_${i}"
done
#### Add Token with value to all users
USERTOKEN=$(openssl rand -hex 64)
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
  _RunSql "alter user user_test_1_${i} add token user_token_2_${i} x'${USERTOKEN}'"
done
#### Add Token with attribute to all users
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
  _RunSql "alter user user_test_1_${i} add token user_token_3_${i}
          with EXPIRATION_TIMESTAMP = '2035-02-23 12:34:23' "
done
#### Add Token with attribute to all users
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
  _RunSql "alter user user_test_1_${i} add token user_token_4_${i}
          with DESCRIPTION = 'user token 4 ${i}' "
done
#### Add Token with value and token attribute to all users
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
  _RunSql "alter user user_test_1_${i} add token user_token_5_${i}
          with EXPIRATION_TIMESTAMP = '2035-02-23 12:34:23', DESCRIPTION = 'user token 5 ${i}' "
done
#### Add Token with value and token attribute to all users
USERTOKEN=$(openssl rand -hex 64)
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
  _RunSql "alter user user_test_1_${i} add token user_token_6_${i} x'${USERTOKEN}'
          with EXPIRATION_TIMESTAMP = '2035-02-23 12:34:23', DESCRIPTION = 'user token 6 ${i}' "
done
#### Add many Tokens to user ${siodbUserTridStartingAt}
for ((i = ${numberOfTokensToTest}; i < $((${numberOfTokensToTest}+1)); ++i)); do
  USERTOKEN=$(openssl rand -hex 64)
  _RunSql "alter user user_test_1_${siodbUserTridStartingAt} add token user_token_7_${i} x'${USERTOKEN}'
          with EXPIRATION_TIMESTAMP = '2035-02-23 12:34:23', DESCRIPTION = 'user token 7 ${i}' "
done


## -------------------------------------------------------------------------
## Users Token Alteration
## -------------------------------------------------------------------------
#### Alter all tokens SET
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
  _RunSql "alter user user_test_1_${i} alter token user_token_1_${i}
          set DESCRIPTION = 'user token 1 altered 1 ${i}'"
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
  _RunSql "alter user user_test_1_${i} alter token user_token_1_${i}
          set EXPIRATION_TIMESTAMP = '2045-02-23 12:34:23' "
done
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
  _RunSql "alter user user_test_1_${i} alter token user_token_1_${i}
          set DESCRIPTION = 'user token 1 altered 2 ${i}', EXPIRATION_TIMESTAMP = '2055-02-23 12:34:23'"
done

#### Expected error with Token
_RunSqlAndValidateOutput "alter user user_test_1_${siodbUserTridStartingAt} alter token user_token_1_1
                          rename to user_token_1_1_renamed_1_if_exists" 'Status .*: Not implemented yet'
_RunSqlAndValidateOutput "alter user user_test_1_${siodbUserTridStartingAt} alter token user_token_1_1
                          rename if exists to user_token_1_1_renamed_1_if_exists" 'Status .*: Not implemented yet'
_RunSql "alter user user_test_1_${siodbUserTridStartingAt} drop token IF EXISTS NOEXISTS"
_RunSql "alter user user_test_1_${siodbUserTridStartingAt} add token user_token_8_1"
_RunSqlAndValidateOutput "alter user user_test_1_${siodbUserTridStartingAt} add token user_token_8_1" 'Status .*: User token'
_CheckLogFiles 'User token .* already exists'
USERTOKEN=$(openssl rand -hex 64)
_RunSql "alter user user_test_1_${siodbUserTridStartingAt} add token user_token_9_2 x'${USERTOKEN}'"
_RunSqlAndValidateOutput "alter user user_test_1_${siodbUserTridStartingAt} add token user_token_9_3 x'${USERTOKEN}'" 'Status .*: Duplicate user token'
_CheckLogFiles 'Duplicate user token'
_RunSqlAndValidateOutput "alter user user_test_1_${siodbUserTridStartingAt} add token user_token_2_${siodbUserTridStartingAt} x'FAKE'"\
  'Status .*: at .*: Invalid character in the hex literal'
_CheckLogFiles 'Invalid character in the hex literal'

## -------------------------------------------------------------------------
## Users Token deletion
## -------------------------------------------------------------------------
#### Drop all tokens
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} drop token IF EXISTS user_token_1_${i}"
_RunSql "alter user user_test_1_${i} drop token user_token_2_${i}"
_RunSql "alter user user_test_1_${i} drop token IF EXISTS user_token_3_${i}"
_RunSql "alter user user_test_1_${i} drop token user_token_4_${i}"
_RunSql "alter user user_test_1_${i} drop token IF EXISTS user_token_5_${i}"
_RunSql "alter user user_test_1_${i} drop token user_token_6_${i}"
done

## -------------------------------------------------------------------------
## Check Users Token
## -------------------------------------------------------------------------
USERTOKEN=$(openssl rand -hex 64)
_RunSql "alter user user_test_1_${siodbUserTridStartingAt} add token user_token_10_1 x'${USERTOKEN}'"
_RunSql "check token user_test_1_${siodbUserTridStartingAt}.user_token_10_1 x'${USERTOKEN}'"
_RunSqlAndValidateOutput "check token user_test_1_${siodbUserTridStartingAt}.user_token_10_1 x'FAKE'" 'Status .*: .* Invalid character in the hex literal'
USERTOKEN=$(openssl rand -hex 64)
_RunSqlAndValidateOutput "check token user_test_1_${siodbUserTridStartingAt}.user_token_10_1 x'${USERTOKEN}'" 'Status .*: User token .* check failed'
_CheckLogFiles 'User token .* check failed|Invalid character in the hex literal'


## -------------------------------------------------------------------------
## Users deletion
## -------------------------------------------------------------------------
#### Delete User of test 1
for ((i = ${siodbUserTridStartingAt}; i < $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "drop user user_test_1_${i}"
done
#### Delete max users
for ((i = $((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1)); i < $(($((${siodbUserTridStartingAt}+${numberOfUsersToTest}+1))+${numberOfUsersToTestMax}+1)); ++i)); do
_RunSql "drop user user_test_10_${i}"
done

_CheckLogFiles

### Stop test
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
