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
DATA_DIR=$(cat /etc/siodb/instances/siodb/config | egrep '^data_dir' \
    | awk -F "=" '{print $2}' | sed -e 's/^[[:space:]]*//')
LOG_DIR=$(cat /etc/siodb/instances/siodb/config  | egrep '^log.file.destination' \
    | awk -F "=" '{print $2}' | sed -e 's/^[[:space:]]*//')
SCRIPT=$(realpath $0)
SCRIPT_DIR=$(dirname $SCRIPT)
startup_timeout=15

function _Prepare {
  _log "INFO" "Cleanup traces of previous default instance"

  if [ ! -f "/etc/siodb/instances/siodb/config" ]; then
    _log "ERROR" "Configuration file not found."
  fi

  _killSiodb

  if [ -d "${DATA_DIR}" ]; then
    _log "INFO"  "Purging directory '${DATA_DIR}'"
    rm -rf ${DATA_DIR}/*
    rm -rf ${DATA_DIR}/.initialized
    echo "Contents of the ${DATA_DIR}  after cleanup"
    ls -la ${DATA_DIR}
  else
    _log "INFO" "Directory '${DATA_DIR}' must exist."
  fi

  if [ -d "${LOG_DIR}" ]; then
    _log "INFO" "Purging directory '${LOG_DIR}'"
    rm -rf ${LOG_DIR}/*
    echo "Contents of the ${LOG_DIR} after cleanup"
    ls -la ${LOG_DIR}
  else
    _log "INFO" "Directory '${LOG_DIR}' must exist."
  fi

  sleep 5

  _log "INFO" "Preparing default Siodb instance"
  dd if=/dev/urandom of=/etc/siodb/instances/siodb/master_key bs=16 count=1
  cp -f ${SCRIPT_DIR}/../share/public_key /etc/siodb/instances/siodb/initial_access_key
}

function _ShowSiodbProcesses {
  echo "===== Siodb Processes ====="
  ps -aux | grep siodb
  echo "==========================="
}

function _StartSiodb {
  _log "INFO" "Starting default Siodb instance"
  ${SIODB_BIN}/siodb --instance siodb --daemon
  sleep ${startup_timeout}
  _ShowSiodbProcesses
  sleep 3
}

function _StopSiodb {
  _log "INFO" "Stopping Siodb process on default instance"
  _ShowSiodbProcesses
  SIODB_PROCESS_ID=$(ps -ef | grep 'siodb --instance siodb --daemon' | grep -v grep \
    | awk '{print $2}')
  if [[ -z "${SIODB_PROCESS_ID}" ]]; then
    echo "Siodb is already not running."
  else
    kill -SIGINT ${SIODB_PROCESS_ID}
    sleep 10
  fi
  echo "After stopping:"
  _ShowSiodbProcesses
}

function _CheckLogFiles {
  _log "INFO" "Checking for errors in the log files"
  ERROR_COUNT=$(cat ${LOG_DIR}/* | grep error | wc -l)
  if [ "${ERROR_COUNT}" == "0" ]; then
    _log "INFO" "No error detected the in log files"
  else
    echo "## ================================================="
    echo "`cat ${LOG_DIR}/* | grep -n error`"
    echo "## ================================================="
    _log "ERROR" "I found an issue in the log files"
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
_log "INFO" "All tests passed"
