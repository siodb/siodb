#!/bin/bash
# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

trap _testfails ERR
set -x
set -e

## Parameters
DATAFILE_DIR=$(cat /etc/siodb/instances/siodb/config  | egrep '^data_dir' | awk -F "=" '{print $2}' | sed -e 's/^[[:space:]]*//')
LOGFILE_DIR=$(cat /etc/siodb/instances/siodb/config  | egrep '^log.file.destination' | awk -F "=" '{print $2}' | sed -e 's/^[[:space:]]*//')
SCRIPT=$(readlink -f $0)
SCRIPTPATH=$(dirname $SCRIPT)

function _testfails {
  _killSiodb
}

function _killSiodb {
  if [[ `pkill -c siodb` -gt 0 ]]; then
    pkill -9 siodb
  fi
}

function _prepare {
  _log "INFO" "Cleanup traces of previous default instance..."

  if [ ! -f "/etc/siodb/instances/siodb/config" ]; then
    _log "ERROR" "Configuration file not found."
  fi

  _killSiodb

  if [ -d "${DATAFILE_DIR}" ]; then
    _log "INFO"  "Purging directory '${DATAFILE_DIR}'..."
    rm -rf ${DATAFILE_DIR}/*
  else
    _log "INFO" "Directory '${DATAFILE_DIR}' must exist."
  fi

  if [ -d "${LOGFILE_DIR}" ]; then
    _log "INFO" "Purging directory '${LOGFILE_DIR}'..."
    rm -rf ${LOGFILE_DIR}/*
  else
    _log "INFO" "Directory '${LOGFILE_DIR}' must exist."
  fi

  sleep 5

  _log "INFO" "Preparing the Siodb default instance..."
  openssl rand -out /etc/siodb/instances/siodb/system_db_key 16 >\
     /etc/siodb/instances/siodb/system_db_key
  echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDoBVv3EJHcAasNU4nYdJtdfCVeSH4+5iTQEfx4xGrc0cA4TM5VwGdxTfyUU8wREsTuDi7GsWunFEKsPGZmHH+d/NNfDitK9esnG5QqdFgYEnKvWu9wHijoQHaEIKk+A6vCJrPRwfullOMPQV+R1ItRxLJY/BSO89tOBbD1+E+GMz9K0XRm1a3hegAmPq/nJSAjdyafKVk/8CXwFHCeMAlmFiI3iJ0Na/J4Qq6Xx5DW/bHcgum8LFDHrCT+GS1opoSLvoqC6C5k5vNkefBOYg3I3yd55XWYn5aaME0R63IyIyaf2WWYaljSlK73uI/GHBG9BLyr87X9p8ce1HlV0qWl" > /etc/siodb/instances/siodb/initial_access_key
}

function _StartSiodb {
  _log "INFO" "Starting default Siodb instance..."
  ${SIODB_PATH}/siodb --instance siodb --daemon
  sleep 30
}

function _StopSiodb {
  _log "INFO" "Stopping Siodb process on default instance..."
  SIODB_PROCESS_ID=$(ps -ef | grep 'siodb --instance siodb --daemon' | grep -v grep | awk '{print $2}')
  if [[ -z "${SIODB_PROCESS_ID}" ]]; then
    echo "Siodb is already not running."
  else
    kill -SIGINT ${SIODB_PROCESS_ID}
    sleep 10
  fi
}

function _CheckLogFileError {
  _log "INFO" "Checking error in the log files..."
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
  echo "## `date "+%Y-%m-%dT%H:%M:%S"` | ${1} | ${2}"
  if [[ "${1}" == 'ERROR' ]]; then
    _testfails
    exit 1
  fi
}

function _CallUnitTests {
  _log "INFO" "Executing script ${1} in siocli..."
  ${SIODB_PATH}/siocli --admin siodb -u root -i ${SCRIPTPATH}/unencrypted_rsa < ${1}
}

function _TestExternalAbort {
  _log "INFO" "Testing an external abort ${1}..."
  pkill -9 siodb
}

## Program

if [[ -z "${SIODB_PATH}" ]]; then
  if [ $# -ne 1 ]; then
    _log "ERROR" "Please, indicate the path to Siodb bin directory as a parameter 1."
  else
    SIODB_PATH="${1}"
  fi
fi

if [ ! -d "${SIODB_PATH}" ]; then
  _log "ERROR" "Invalid directory."
fi

_log "INFO" "Tests start"
_prepare
_StartSiodb
_CheckLogFileError
_CallUnitTests ${SCRIPTPATH}/query_sys_tables.sql
_CheckLogFileError
_CallUnitTests ${SCRIPTPATH}/ddl_database.sql
_CheckLogFileError
_CallUnitTests ${SCRIPTPATH}/ddl_user.sql
_CheckLogFileError
_CallUnitTests ${SCRIPTPATH}/ddl_general.sql
_CheckLogFileError
_CallUnitTests ${SCRIPTPATH}/dml_general.sql
_CheckLogFileError
_CallUnitTests ${SCRIPTPATH}/dml_datetime.sql
_CheckLogFileError
_StopSiodb
_CheckLogFileError
_log "INFO" "All SQL tests passed successfully!"
