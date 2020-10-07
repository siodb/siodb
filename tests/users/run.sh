#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
source $(dirname "$0")/../share/CommonFunctions.sh

## Specific test functions


## Test
_log "INFO" "Tests start"
_Prepare
_StartSiodb

### Starting point users
_RunSqlAndValidateOutput "select NAME from SYS.SYS_USERS" 7 0 'ROOT'

### Create users
for i in {4096..4196}; do
_RunSql "create user nico${i}"
done
for i in {4096..4196}; do
_RunSqlAndValidateOutput "select TRID from SYS.SYS_USERS where name = 'NICO${i}'" 7 0 "${i}"
done
for i in {4096..4196}; do
_RunSqlAndValidateOutput "select NAME from SYS.SYS_USERS where name = 'NICO${i}'" 7 0 "NICO${i}"
done

### Users keys
for i in {4096..4196}; do
_RunSql "alter user nico${i} add access key main 'ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDBOzgmO6E8xJAWz0CyzG8/FWJ+0oTTbPqX1c0JEKufxyHdS8VyTl6BuL7aIYt5RiUc+V1bzOKt0guPCu8WKIgeb1nq3qtvWaswJBod6iWs6iN1y+6+/oT47CrgWZUi9LLseGxit8DQHeCshTvaB8e6ZFH2sZdTpS8Z7U86znNnfX/7qoXUqEXVLawBKC8NGgWpvjvi0ZK9AF8ckD9p4Tdcoy+8m3+aFitbv1i7mVY+hJ7pDlRp6YeJYKC3kC46Bp41G2x0tpgls0HzpIEMBedV95aVxECrPxcAEkooIMWCJBbTEP7mc6Sb0H82p1QE2zrluW/L/S82NCLLWqsqm5An' ;
"
done

### Wrong username
_RunSqlAndValidateOutput "create user 😀😀😀😀😀😀😀" 6 38 'Status 2: at (1, 12): mismatched input'
_CheckLogFiles 'common parse error'

### Other
_RunSql "SELECT * FROM SYS.SYS_USER_PERMISSIONS"
_RunSql "SELECT * FROM SYS.SYS_USER_TOKENS"


### Stop test
_StopSiodb
_log "INFO" "All tests passed"
