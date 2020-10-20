#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Uncomment for debugging this script
# set -x

source $(dirname "$0")/../share/CommonFunctions.sh

## Program

if [[ -z "${SIODB_BIN}" ]]; then
  if [ $# -ne 1 ]; then
    _log "ERROR" "Please, indicate the path to Siodb bin directory as a parameter 1."
    _failExit
  else
    SIODB_BIN="$1"
  fi
fi

if [[ "${SIODB_BIN}" == "debug" ]]; then
  SIODB_BIN=build/debug/bin
  SHORT_TEST=0
elif [[ "${SIODB_BIN}" == "release" ]]; then
  SIODB_BIN=build/release/bin
  SHORT_TEST=0
elif [[ "${SIODB_BIN}" == "sdebug" ]]; then
  SIODB_BIN=build/debug/bin
  SHORT_TEST=1
elif [[ "${SIODB_BIN}" == "srelease" ]]; then
  SIODB_BIN=build/release/bin
  SHORT_TEST=1
fi


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

  _RunSqlScript "${SCRIPT_DIR}/query_sys_tables.sql"
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/ddl_database.sql"
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/ddl_user.sql"
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/ddl_general.sql"
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/dml_general.sql"
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/dml_datetime.sql"
  _CheckLogFiles

fi

_StopSiodb
_CheckLogFiles
_log "INFO" "SUCCESS: All tests passed"
