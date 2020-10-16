#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# --------------------------------------------------------------
# set
# --------------------------------------------------------------
if [ "${SIOTEST_TRACE}" == "1" ]; then
set -x
fi
set -e


# --------------------------------------------------------------
# External Parameters
# --------------------------------------------------------------
if [[ -z "${SIODB_BIN}" ]]; then
    SIODB_BIN="build/debug/bin"
fi
if [[ -z "${SIODB_INSTANCE}" ]]; then
    SIODB_INSTANCE=siodb
fi
if [[ -z "${SIOTEST_KEEP_INSTANCE_UP}" ]]; then
    SIOTEST_KEEP_INSTANCE_UP=0
fi


# --------------------------------------------------------------
# Derive parameters
# --------------------------------------------------------------
DATA_DIR=$(cat /etc/siodb/instances/${SIODB_INSTANCE}/config | egrep '^data_dir' \
    | awk -F "=" '{print $2}' | sed -e 's/^[[:space:]]*//')
LOG_DIR=$(cat /etc/siodb/instances/${SIODB_INSTANCE}/config  | egrep '^log.file.destination' \
    | awk -F "=" '{print $2}' | sed -e 's/^[[:space:]]*//')
SCRIPT=$(realpath $0)
SCRIPT_DIR=$(dirname $SCRIPT)
instanceStartupTimeout=45

# --------------------------------------------------------------
# Trapping
# --------------------------------------------------------------
if [[ "${SIOTEST_KEEP_INSTANCE_UP}" == "0" ]]; then
  trap _testfails ERR
fi


# --------------------------------------------------------------
# Global functions
# --------------------------------------------------------------
function _testfails {
  _killSiodb
}

function _killSiodb {
  if [[ `pkill -c siodb` -gt 0 ]]; then
    pkill -9 siodb
  fi
}


function _Prepare {
  _log "INFO" "Cleanup traces of previous default instance"

  if [ ! -f "/etc/siodb/instances/${SIODB_INSTANCE}/config" ]; then
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

  _log "INFO" "Preparing default Siodb instance"
  dd if=/dev/urandom of=/etc/siodb/instances/${SIODB_INSTANCE}/master_key bs=16 count=1
  cp -f ${SCRIPT_DIR}/../share/public_key /etc/siodb/instances/${SIODB_INSTANCE}/initial_access_key
}

function _ShowSiodbProcesses {
  echo "===== Siodb Processes ====="
  if [[ $(pgrep -c siodb) -gt 0 ]]; then
    pgrep -a siodb
  fi
  echo "==========================="
}

function _StartSiodb {
  _log "INFO" "Starting default Siodb instance"
  ${SIODB_BIN}/siodb --instance ${SIODB_INSTANCE} --daemon
  numberEntriesInLog=0
  counterTimeout=0
  while [[ $numberEntriesInLog -eq 0 ]]; do
    _log "INFO" "Waiting for instance to be ready..."
    numberEntriesInLog=$(egrep 'Listening for (TCP|UNIX) connections' ${LOG_DIR}/siodb_*.log | wc -l | bc)
    if [[ ${counterTimeout} -gt ${instanceStartupTimeout} ]]; then
    _log "ERROR" "Timeout (${instanceStartupTimeout} seconds) reached while starting the instance..."
    fi
    counterTimeout=$((counterTimeout+1))
    sleep 1
  done
  _ShowSiodbProcesses
}

function _StopSiodb {
  if [[ "${SIOTEST_KEEP_INSTANCE_UP}" == "0" ]]; then
    _log "INFO" "Stopping Siodb process on default instance"
    _ShowSiodbProcesses
    SIODB_PROCESS_ID=$(ps -ef | grep "siodb --instance ${SIODB_INSTANCE} --daemon" | grep -v grep \
      | awk '{print $2}')
    if [[ -z "${SIODB_PROCESS_ID}" ]]; then
      echo "Siodb is already not running."
    else
      kill -SIGINT ${SIODB_PROCESS_ID}
      counterSiodbProcesses=$(ps -ef | grep "siodb --instance ${SIODB_INSTANCE} --daemon" | grep -v grep \
        | awk '{print $2}' | wc -l | bc)
      counterTimeout=0
      while [[ $counterSiodbProcesses -ne 0 ]]; do
        _log "INFO" "Waiting for instance to be stopped..."
        counterSiodbProcesses=$(ps -ef | grep "siodb --instance ${SIODB_INSTANCE} --daemon" | grep -v grep \
        | awk '{print $2}' | wc -l | bc)
        if [[ ${counterTimeout} -gt ${instanceStartupTimeout} ]]; then
          _log "ERROR" "Timeout (${instanceStartupTimeout} seconds) reached while stopping the instance..."
        fi
        counterTimeout=$((counterTimeout+1))
        sleep 1
      done
    fi
    echo "After stopping:"
    _ShowSiodbProcesses
  else
    _log "INFO" "Siodb process kept in running state"
  fi
}

function _CheckLogFiles {
  # $1: exclude these patterns because expected
  _log "INFO" "Checking for errors in the log files"
  ERROR_COUNT=$(cat ${LOG_DIR}/* | grep error | egrep -v "${1}" | wc -l)
  if [ "${ERROR_COUNT}" == "0" ]; then
    _log "INFO" "No error detected the in log files"
  else
    echo "## ================================================="
    echo "`cat ${LOG_DIR}/* | grep -n error | egrep -v "${1}"`"
    echo "## ================================================="
    _log "ERROR" "I found an issue in the log files"
  fi
}

function _log {
  echo "## `date "+%Y-%m-%dT%H:%M:%S"` | $1 | $2"
  if [[ "$1" == 'ERROR' ]]; then
    if [[ "${SIOTEST_KEEP_INSTANCE_UP}" == "0" ]]; then
      _testfails
    fi
    exit 1
  fi
}

function _RunSqlScript {
  _log "INFO" "Executing SQL script $1"
  ${SIODB_BIN}/siocli ${SIOCLI_DEBUG} --nologo --admin ${SIODB_INSTANCE} -u root \
    -i ${SCRIPT_DIR}/../share/private_key < $1
}

function _RunSql {
  _log "INFO" "Executing SQL: $1"
  ${SIODB_BIN}/siocli ${SIOCLI_DEBUG} --nologo --admin ${SIODB_INSTANCE} -u root \
    -i ${SCRIPT_DIR}/../share/private_key <<< ''"$1"''
}

function _RunSqlAndValidateOutput {
  # $1: the SQL to execute
  # $2: The expected output
  _log "INFO" "Executing SQL: $1"
  SIOCLI_OUTPUT=$(${SIODB_BIN}/siocli ${SIOCLI_DEBUG} --nologo --admin ${SIODB_INSTANCE} -u root \
    --keep-going -i ${SCRIPT_DIR}/../share/private_key <<< ''"${1}"'')
  EXPECTED_RESULT_COUNT=$(echo "${SIOCLI_OUTPUT}" | egrep "${2}" | wc -l | bc)
  if [[ ${EXPECTED_RESULT_COUNT} -eq 0 ]]; then
    _log "ERROR" "Siocli output does not match expected output. Output is: ${SIOCLI_OUTPUT}"
  else
    _log "INFO" "Siocli output matched expected output."
  fi
}

function _RunRestRequest3 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -u $5 -T $6
}

function _RunRestRequest4 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -P ''"$4"'' -u $5 -T $6
}

function _RunRestRequest5 {
  _log "INFO" "Executing REST request: $1 $2 $3 @$4"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -f "$4" -u $5 -T $6
}

function _RunRestRequest6 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 $5"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -P ''"$5"'' -u $6 -T $7
}

function _RunRestRequest6d {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 $5"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -P ''"$5"'' -u $6 -T $7 --drop
}

function _RunRestRequest7 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 @$5"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -f "$5" -u $6 -T $7
}

function _RunRestRequest7d {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 @$5"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -f "$5" -u $6 -T $7 --drop
}

function _RunCurlGetDatabasesRequest {
  _log "INFO" "Executing CurlGetDatabasesRequest"
  auth=$(echo "$1:$2" | base64 -w0)
  _log "DEBUG" "auth=${auth}"
  curl -v -H "Authorization: Basic ${auth}" http://localhost:50080/databases
  #echo "Running: curl -v http://$1:$2@localhost:50080/databases"
  #curl -v http://$1:$2@localhost:50080/databases
  echo ""
}

function _RunCurlGetTablesRequest {
  _log "INFO" "Executing CurlGetTablesRequest: $1"
  auth=$(echo "$2:$3" | base64 -w0)
  _log "DEBUG" "auth=${auth}"
  curl -v -H "Authorization: Basic ${auth}" http://localhost:50080/databases/$1/tables
  echo ""
}

function _TestExternalAbort {
  _log "INFO" "Testing an external abort $1"
  pkill -9 siodb
}
