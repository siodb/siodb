#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
source $(dirname "$0")/../share/CommonFunctions.sh

## Specific test functions


## Tests
_log "INFO" "Tests start"
_Prepare
_StartSiodb

### Starting point users
_RunSqlAndValidateOutput "select NAME from SYS.SYS_USERS" 7 0 'ROOT'

### Users

#### Create users
for i in {4096..4106}; do
_RunSql "create user nico${i}"
done
for i in {4107..4116}; do
_RunSql "create user nico${i}
         with STATE = ACTIVE"
done
for i in {4117..4126}; do
_RunSql "create user nico${i}
         with STATE = INACTIVE"
done
for i in {4127..4136}; do
_RunSql "create user nico${i}
         with REAL_NAME = 'Nicolas ${i} 😀'"
done
for i in {4137..4146}; do
_RunSql "create user nico${i}
         with DESCRIPTION = 'User for developer ${i} 😀'"
done
for i in {4147..4156}; do
_RunSql "create user nico${i}
         with STATE = ACTIVE, REAL_NAME = 'Nicolas ${i} 😀'"
done
for i in {4157..4166}; do
_RunSql "create user nico${i}
         with STATE = ACTIVE, DESCRIPTION = 'User for developer ${i} 😀'"
done
for i in {4167..4186}; do
_RunSql "create user nico${i}
         with REAL_NAME = 'Nicolas ${i} 😀', DESCRIPTION = 'User for developer ${i} 😀'"
done
for i in {4187..4196}; do
_RunSql "create user nico${i}
         with STATE = ACTIVE, REAL_NAME = 'Nicolas ${i} 😀', DESCRIPTION = 'User for developer ${i} 😀'"
done
#### Alter user
for i in {4096..4116}; do
_RunSql "alter user nico${i}
         set STATE = ACTIVE"
done
for i in {4117..4136}; do
_RunSql "alter user nico${i}
         set STATE = INACTIVE"
done
for i in {4137..4156}; do
_RunSql "alter user nico${i}
         set STATE = ACTIVE, REAL_NAME = 'Nicolas ${i} 😀'"
done
for i in {4157..4176}; do
_RunSql "alter user nico${i}
         set STATE = ACTIVE, REAL_NAME = 'Nicolas ${i} 😀', DESCRIPTION = 'User for developer ${i} 😀'"
done
for i in {4177..4196}; do
_RunSql "alter user nico${i}
         set REAL_NAME = 'Nicolas ${i} 😀', DESCRIPTION = 'User for developer ${i} 😀'"
done
#### Check user data
for i in {4096..4196}; do
_RunSqlAndValidateOutput "select TRID from SYS.SYS_USERS where name = 'NICO${i}'" 7 0 "${i}"
done
for i in {4096..4196}; do
_RunSqlAndValidateOutput "select NAME from SYS.SYS_USERS where name = 'NICO${i}'" 7 0 "NICO${i}"
done
for i in {4096..4196}; do
_RunSqlAndValidateOutput "select REAL_NAME from SYS.SYS_USERS where name = 'NICO${i}'" 7 0 "Nicolas ${i} 😀"
done
for i in {4096..4196}; do
_RunSqlAndValidateOutput "select DESCRIPTION from SYS.SYS_USERS where name = 'NICO${i}'" 7 0 "User for developer ${i} 😀"
done

#### Create user with wrong chars
_RunSqlAndValidateOutput "create user 😀😀😀😀😀😀😀" 6 38 'Status 2: at (1, 12): mismatched input'
_CheckLogFiles 'common parse error'
_RunSqlAndValidateOutput "create user 123456789" 6 38 'Status 2: at (1, 12): mismatched input'
_CheckLogFiles 'common parse error'


### Users keys
SIOKEY1=$(cat $(dirname "$0")/../share/public_key)
#### Add key
for i in {4096..4196}; do
_RunSql "alter user nico${i} add access key key1 '${SIOKEY1}'"
done
#### Add 2nd key
for i in {4096..4196}; do
_RunSql "alter user nico${i} add access key key2 '${SIOKEY1}'"
done
#### Add 100 keys to one user
for i in {3..103}; do
_RunSql "alter user nico4096 add access key key${i} '${SIOKEY1}'"
done
#### Alter 2nd key
for i in {4096..4196}; do
_RunSql "alter user nico${i} alter access key key2 set STATE = INACTIVE, DESCRIPTION = 'TEST INACTIVE 😀'"
done
for i in {4096..4196}; do
_RunSql "alter user nico${i} alter access key key2 set STATE = ACTIVE, DESCRIPTION = 'TEST ACTIVE 😀'"
done
for i in {4096..4196}; do
_RunSql "alter user nico${i} alter access key key1 set STATE = INACTIVE"
done
for i in {4096..4196}; do
_RunSql "alter user nico${i} alter access key key1 set STATE = ACTIVE"
done
#### Drop access keys 1 for all users
for i in {4096..4150}; do
_RunSql "alter user nico${i} drop access key key1"
done
# BELOW FAILS
# for i in {4096..4196}; do
# _RunSql "alter user nico${i} drop access key if exists key1"
# done

#### Expected error with keys
_RunSqlAndValidateOutput "alter user nico4096 alter access key key2 rename  to key3 " 6 29 'Status 6: Not implemented yet'
_CheckLogFiles 'common parse error|Status 6: Not implemented yet'
_RunSqlAndValidateOutput "alter user nico4096 alter access key key2 rename if exists to key3" 6 29 'Status 6: Not implemented yet'
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
