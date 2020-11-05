#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR=$(dirname $(realpath "$0"))
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Program
_log "INFO" "Tests start"
_Prepare
_StartSiodb
_CheckLogFiles

##export SIOCLI_DEBUG=--debug

if [[ "${SHORT_TEST}" == "1" ]]; then

  _RunSql "SELECT * FROM SYS.SYS_TABLES"
  _CheckLogFiles

  _RunSql "SELECT * FROM SYS.SYS_DATABASES"
  _CheckLogFiles

else

  _RunSqlScript "${SCRIPT_DIR}/query_sys_tables.sql" 120
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/ddl_database.sql" 120
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/ddl_user.sql" 120
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/ddl_general.sql" 120
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/dml_general.sql" 120
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/dml_datetime.sql" 120
  _CheckLogFiles

fi

_StopSiodb
_CheckLogFiles
_log "INFO" "SUCCESS: All tests passed"
