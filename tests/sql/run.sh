#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# Uncomment for debugging this script
# set -x

set -e
trap _testfails ERR

function _testfails {
  _killSiodb
}

function _killSiodb {
  if [[ `pkill -c siodb` -gt 0 ]]; then
    pkill -9 siodb
  fi
}


## Parameters
DATAFILE_DIR=$(cat /etc/siodb/instances/siodb/config | egrep '^data_dir' \
    | awk -F "=" '{print $2}' | sed -e 's/^[[:space:]]*//')
LOGFILE_DIR=$(cat /etc/siodb/instances/siodb/config  | egrep '^log.file.destination' \
    | awk -F "=" '{print $2}' | sed -e 's/^[[:space:]]*//')
SCRIPT=$(realpath $0)
SCRIPT_DIR=$(dirname $SCRIPT)


function _Prepare {
  _log "INFO" "Cleanup traces of previous default instance"

  if [ ! -f "/etc/siodb/instances/siodb/config" ]; then
    _log "ERROR" "Configuration file not found."
  fi

  _killSiodb

  if [ -d "${DATAFILE_DIR}" ]; then
    _log "INFO"  "Purging directory '${DATAFILE_DIR}'"
    rm -rf ${DATAFILE_DIR}/*
  else
    _log "INFO" "Directory '${DATAFILE_DIR}' must exist."
  fi

  if [ -d "${LOGFILE_DIR}" ]; then
    _log "INFO" "Purging directory '${LOGFILE_DIR}'"
    rm -rf ${LOGFILE_DIR}/*
  else
    _log "INFO" "Directory '${LOGFILE_DIR}' must exist."
  fi

  sleep 5

  _log "INFO" "Preparing default Siodb instance"
  dd if=/dev/urandom of=/etc/siodb/instances/siodb/system_db_key bs=16 count=1
  cp -f ${SCRIPT_DIR}/../share/public_key /etc/siodb/instances/siodb/initial_access_key
}

function _StartSiodb {
  _log "INFO" "Starting default Siodb instance"
  ${SIODB_BIN}/siodb --instance siodb --daemon
  sleep 20
}

function _StopSiodb {
  _log "INFO" "Stopping Siodb process on default instance"
  SIODB_PROCESS_ID=$(ps -ef | grep 'siodb --instance siodb --daemon' | grep -v grep \
    | awk '{print $2}')
  if [[ -z "${SIODB_PROCESS_ID}" ]]; then
    echo "Siodb is already not running."
  else
    kill -SIGINT ${SIODB_PROCESS_ID}
    sleep 10
  fi
}

function _CheckLogFileError {
  _log "INFO" "Checking error in the log files"
  ERROR_COUNT=$(cat ${LOGFILE_DIR}/* | grep error | wc -l)
  if [ "${ERROR_COUNT}" == "0" ]; then
    _log "INFO" "No error detected the log file(s)"
  else
    echo "## ================================================="
    echo "`cat ${LOGFILE_DIR}/* | grep error`"
    echo "## ================================================="
    _log "ERROR" "I found an issue in the log file(s)"
  fi
}

function _log {
  echo "## `date "+%Y-%m-%dT%H:%M:%S"` | $1 | $2"
  if [[ "$1" == 'ERROR' ]]; then
    _testfails
    exit 1
  fi
}

function _RunSqlScript {
  _log "INFO" "Executing SQL script $1"
  ${SIODB_BIN}/siocli ${SIOCLI_DEBUG} --nologo --admin siodb -u root \
    -i ${SCRIPT_DIR}/../share/private_key < $1
}

function _RunSql {
  _log "INFO" "Executing SQL: $1"
  ${SIODB_BIN}/siocli ${SIOCLI_DEBUG} --nologo --admin siodb -u root \
    -i ${SCRIPT_DIR}/../share/private_key -c ''"$1"''
}

function _TestExternalAbort {
  _log "INFO" "Testing an external abort $1"
  pkill -9 siodb
}

## Program

if [[ -z "${SIODB_BIN}" ]]; then
  if [ $# -ne 1 ]; then
    _log "ERROR" "Please, indicate the path to Siodb bin directory as a parameter 1."
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

if [[ ! -d "${SIODB_BIN}" ]]; then
  _log "ERROR" "Invalid Siodb binary directory."
fi

_log "INFO" "Tests start"
_Prepare
_StartSiodb
_CheckLogFileError

##export SIOCLI_DEBUG=--debug

if [[ "${SHORT_TEST}" == "1" ]]; then

  _RunSql "SELECT * FROM SYS.SYS_TABLES"
  _CheckLogFileError

else

  _RunSqlScript "${SCRIPT_DIR}/query_sys_tables.sql"
  _CheckLogFileError

  _RunSqlScript "${SCRIPT_DIR}/ddl_database.sql"
  _CheckLogFileError

  _RunSqlScript "${SCRIPT_DIR}/ddl_user.sql"
  _CheckLogFileError

  _RunSqlScript "${SCRIPT_DIR}/ddl_general.sql"
  _CheckLogFileError

  _RunSqlScript "${SCRIPT_DIR}/dml_general.sql"
  _CheckLogFileError

  _RunSqlScript "${SCRIPT_DIR}/dml_datetime.sql"
  _CheckLogFileError

fi

_StopSiodb
_CheckLogFileError
_log "INFO" "All tests passed"
