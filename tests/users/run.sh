#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
source $(dirname "$0")/../share/CommonFunctions.sh

## Specific test functions

## Specific test parameters
numberOfUsersToTest=10
numberOfKeysToTest=100

## Tests
_log "INFO" "Tests start"
_Prepare
_StartSiodb

### Starting point users
_RunSqlAndValidateOutput "select NAME from SYS.SYS_USERS" 7 0 'ROOT'

### Users

#### Create users
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_1_${i}"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_2_${i}
         with STATE = ACTIVE"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_3_${i}
         with STATE = INACTIVE"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_4_${i}
         with REAL_NAME = 'TestUser ${i} 😀'"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_5_${i}
         with DESCRIPTION = 'User for developer ${i} 😀'"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_6_${i}
         with STATE = ACTIVE, REAL_NAME = 'TestUser ${i} 😀'"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_7_${i}
         with STATE = ACTIVE, DESCRIPTION = 'User for developer ${i} 😀'"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_8_${i}
         with REAL_NAME = 'TestUser ${i} 😀', DESCRIPTION = 'User for developer ${i} 😀'"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "create user user_test_9_${i}
         with STATE = ACTIVE, REAL_NAME = 'TestUser ${i} 😀', DESCRIPTION = 'User for developer ${i} 😀'"
done
#### Alter user
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i}
         set STATE = ACTIVE"
_RunSqlAndValidateOutput "select STATE from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" 7 0 "1"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i}
         set STATE = INACTIVE"
_RunSqlAndValidateOutput "select STATE from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" 7 0 "0"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i}
         set STATE = ACTIVE, REAL_NAME = 'TestUser updated 1 ${i} 😀'"
_RunSqlAndValidateOutput "select STATE from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" 7 0 "1"
_RunSqlAndValidateOutput "select REAL_NAME from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" 7 0 "TestUser updated 1 ${i} 😀"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i}
         set STATE = ACTIVE, REAL_NAME = 'TestUser updated 2 ${i} 😀', DESCRIPTION = 'User for developer updated 2 ${i} 😀'"
_RunSqlAndValidateOutput "select STATE from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" 7 0 "1"
_RunSqlAndValidateOutput "select REAL_NAME from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" 7 0 "TestUser updated 2 ${i} 😀"
_RunSqlAndValidateOutput "select DESCRIPTION from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" 7 0 "User for developer updated 2 ${i} 😀"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i}
         set REAL_NAME = 'TestUser updated 3 ${i} 😀', DESCRIPTION = 'User for developer updated 3 ${i} 😀'"
_RunSqlAndValidateOutput "select REAL_NAME from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" 7 0 "TestUser updated 3 ${i} 😀"
_RunSqlAndValidateOutput "select DESCRIPTION from SYS.SYS_USERS where name = 'USER_TEST_1_${i}'" 7 0 "User for developer updated 3 ${i} 😀"
done

#### Create user with wrong chars
_RunSqlAndValidateOutput "create user 😀😀😀😀😀😀😀" 6 38 'Status 2: at (1, 12): mismatched input'
_CheckLogFiles 'common parse error'
_RunSqlAndValidateOutput "create user 123456789" 6 38 'Status 2: at (1, 12): mismatched input'
_CheckLogFiles 'common parse error'


### Users keys
SIOKEY1=$(cat $(dirname "$0")/../share/public_key)
#### Add key
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} add access key key1 '${SIOKEY1}'"
done
#### Add 2nd key
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} add access key key2 '${SIOKEY1}'"
done
#### Add 100 keys to one user
for ((i = 3; i < $((3+${numberOfKeysToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_4096 add access key key${i} '${SIOKEY1}'"
done
#### Alter 2nd key
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} alter access key key2 set STATE = INACTIVE, DESCRIPTION = 'TEST INACTIVE 😀'"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} alter access key key2 set STATE = ACTIVE, DESCRIPTION = 'TEST ACTIVE 😀'"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} alter access key key1 set STATE = INACTIVE"
done
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} alter access key key1 set STATE = ACTIVE"
done
#### Drop access keys 1 for all users
for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
_RunSql "alter user user_test_1_${i} drop access key key1"
done
# BELOW FAILS
# for ((i = 4096; i < $((4096+${numberOfUsersToTest}+1)); ++i)); do
# _RunSql "alter user user_test_1_${i} drop access key if exists key1"
# done

#### Expected error with keys
_RunSqlAndValidateOutput "alter user user_test4096 alter access key key2 rename  to key3 " 6 29 'Status 6: Not implemented yet'
_CheckLogFiles 'common parse error|Status 6: Not implemented yet'
_RunSqlAndValidateOutput "alter user user_test4096 alter access key key2 rename if exists to key3" 6 29 'Status 6: Not implemented yet'
_CheckLogFiles 'common parse error|Status 6: Not implemented yet'



### Login key
## TODO: login with 100 keys with user 4096
## TODO: login with all users at the same time

### Users Token creation
## TODO: add a second Token to all user
## TODO: add 100 Token to user 4096
## TODO: alter all tokey

### Login REST
## TODO: login with 100 Token with user 4096
## TODO: login with all users at the same time

### Users Token modification
## TODO: drop all tokens

### Users Modifications
## TODO: alter all users

### Users deletion
## TODO: drop all users




### Stop test
_StopSiodb
_log "INFO" "SUCCESS: All tests passed"
