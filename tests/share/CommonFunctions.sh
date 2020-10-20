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
    SIODB_BIN="../../build/debug/bin"
fi
if [[ ! -d "${SIODB_BIN}" ]]; then
  echo "## `date "+%Y-%m-%dT%H:%M:%S"` | ERROR | Invalid Siodb binary directory."
  exit 2
fi
if [[ -z "${SIODB_INSTANCE}" ]]; then
    SIODB_INSTANCE=siodb
fi
if [[ -z "${SIOTEST_KEEP_INSTANCE_UP}" ]]; then
    SIOTEST_KEEP_INSTANCE_UP=0
fi


# --------------------------------------------------------------
# Global parameters
# --------------------------------------------------------------
DATA_DIR=""
LOG_DIR=""
SCRIPT=$(realpath $0)
SCRIPT_DIR=$(dirname $SCRIPT)
instanceStartupTimeout=45
previousTestStartedAtTimestamp=0
previousInstanceStartTimestamp=0

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


function _SetInstanceParameter {
  _log "INFO" "setting parameter '${1}' to '${2}'"
  sed -i -e "s#.*${1}[ ]*=.*#${1} = ${2}#g" \
  /etc/siodb/instances/${SIODB_INSTANCE}/config
  cat /etc/siodb/instances/${SIODB_INSTANCE}/config | grep "${1}"
}

function _SetInitialInstanceConfig {
  # Copy default config file
  mkdir -p /etc/siodb/instances/${SIODB_INSTANCE}
  cp ${SCRIPT_DIR}/../../config/siodb.conf /etc/siodb/instances/${SIODB_INSTANCE}/config
  chmod 660 /etc/siodb/instances/${SIODB_INSTANCE}/config

  # Overload default config file
  _SetInstanceParameter "data_dir" "/var/lib/siodb/${SIODB_INSTANCE}/data"
  DATA_DIR="/var/lib/siodb/${SIODB_INSTANCE}/data"
  _SetInstanceParameter "log.file.destination" "/var/log/siodb/${SIODB_INSTANCE}"
  LOG_DIR="/var/log/siodb/${SIODB_INSTANCE}"
  _SetInstanceParameter "enable_rest_server" "yes"
  _SetInstanceParameter "client.enable_encryption" "yes"
  _SetInstanceParameter "client.tls_certificate" "cert.pem"
  _SetInstanceParameter "client.tls_private_key" "key.pem"
  _SetInstanceParameter "rest_server.tls_certificate" "cert.pem"
  _SetInstanceParameter "rest_server.tls_private_key" "key.pem"
  _SetInstanceParameter "rest_server.iomgr_read_timeout" "60"

  cp ${SCRIPT_DIR}/../../config/sample_cert/cert.pem /etc/siodb/instances/${SIODB_INSTANCE}/cert.pem
  chmod 660 /etc/siodb/instances/${SIODB_INSTANCE}/cert.pem
  cp ${SCRIPT_DIR}/../../config/sample_cert/key.pem /etc/siodb/instances/${SIODB_INSTANCE}/key.pem
  chmod 660 /etc/siodb/instances/${SIODB_INSTANCE}/key.pem
}

function _Prepare {
  _log "INFO" "Cleanup traces of previous default instance"

  _SetInitialInstanceConfig

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
  chmod 660 /etc/siodb/instances/${SIODB_INSTANCE}/master_key
  cp -f ${SCRIPT_DIR}/../share/public_key /etc/siodb/instances/${SIODB_INSTANCE}/initial_access_key
  chmod 660 /etc/siodb/instances/${SIODB_INSTANCE}/initial_access_key
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
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/siodb --instance ${SIODB_INSTANCE} --daemon
  numberEntriesInLog=0
  counterTimeout=0
  while [[ $numberEntriesInLog -eq 0 ]]; do
    _log "INFO" "Waiting for instance to be ready..."
    LOG_STARTUP=$(cat ${LOG_DIR}/*.log \
    | awk -v previousInstanceStartTimestamp=${previousInstanceStartTimestamp} \
    '
    function ltrim(s) { sub(/^[ \t\r\n]+/, "", s); return s }
    function rtrim(s) { sub(/[ \t\r\n]+$/, "", s); return s }
    function trim(s)  { return rtrim(ltrim(s)); }
    {
      lineTimestamp = substr($1,1,4)substr($1,6,2)substr($1,9,2)substr($2,1,2)substr($2,4,2)substr($2,7,2)trim(substr($2,10,6))
      if (lineTimestamp > previousInstanceStartTimestamp) {
        print $0;
      }
    }')
    numberEntriesInLog=$(echo "${LOG_STARTUP}" | egrep 'Listening for (TCP|UNIX) connections' | wc -l | bc)
    if [[ ${counterTimeout} -gt ${instanceStartupTimeout} ]]; then
    _log "ERROR" "Timeout (${instanceStartupTimeout} seconds) reached while starting the instance..."
    _failExit
    fi
    counterTimeout=$((counterTimeout+1))
    _CheckLogFiles
    sleep 1
  done
  previousInstanceStartTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  _ShowSiodbProcesses
}

function _StopSiodb {
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
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
          _failExit
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
  LOG_ERROR=$(cat ${LOG_DIR}/*.log \
  | awk -v previousTestStartedAtTimestamp=${previousTestStartedAtTimestamp} \
  '
  function ltrim(s) { sub(/^[ \t\r\n]+/, "", s); return s }
  function rtrim(s) { sub(/[ \t\r\n]+$/, "", s); return s }
  function trim(s)  { return rtrim(ltrim(s)); }
  {
    lineTimestamp = substr($1,1,4)substr($1,6,2)substr($1,9,2)substr($2,1,2)substr($2,4,2)substr($2,7,2)trim(substr($2,10,6))
    if (lineTimestamp > previousTestStartedAtTimestamp && $3 == "error") {
      print $0;
    }
  }')
  if [[ "${1}" == "" ]]; then
    ERROR_COUNT=$(echo "${LOG_ERROR}" | grep error | wc -l)
  else
    ERROR_COUNT=$(echo "${LOG_ERROR}" | grep error | egrep -v "${1}" | wc -l)
  fi
  if [ "${ERROR_COUNT}" == "0" ]; then
    _log "INFO" "No error detected the in log files"
  else
    echo "## ================================================="
    echo "${LOG_ERROR}"
    echo "## ================================================="
    _log "ERROR" "I found an issue in the log files"
    _failExit
  fi
}

function _log {
    echo "## `date "+%Y-%m-%dT%H:%M:%S"` | $1 | $2"
}

function _failExit {
    if [[ "${SIOTEST_KEEP_INSTANCE_UP}" == "0" ]]; then
      _testfails
    fi
    exit 3
}

function _RunSqlScript {
  _log "INFO" "Executing SQL script $1"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/siocli ${SIOCLI_DEBUG} --nologo --admin ${SIODB_INSTANCE} -u root \
    -i ${SCRIPT_DIR}/../share/private_key < $1
}

function _RunSql {
  _log "INFO" "Executing SQL: $1"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/siocli ${SIOCLI_DEBUG} --nologo --admin ${SIODB_INSTANCE} -u root \
    -i ${SCRIPT_DIR}/../share/private_key <<< ''"$1"''
}

function _RunSqlAndValidateOutput {
  # $1: the SQL to execute
  # $2: The expected output
  _log "INFO" "Executing SQL: $1"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  SIOCLI_OUTPUT=$(${SIODB_BIN}/siocli ${SIOCLI_DEBUG} --nologo --admin ${SIODB_INSTANCE} -u root \
    --keep-going -i ${SCRIPT_DIR}/../share/private_key <<< ''"${1}"'')
  EXPECTED_RESULT_COUNT=$(echo "${SIOCLI_OUTPUT}" | egrep "${2}" | wc -l | bc)
  if [[ ${EXPECTED_RESULT_COUNT} -eq 0 ]]; then
    _log "ERROR" "Siocli output does not match expected output. Output is: ${SIOCLI_OUTPUT}"
    _failExit
  else
    _log "INFO" "Siocli output matched expected output."
  fi
}

function _RunRestRequest1 {
  _log "INFO" "Executing REST request: $1 $2"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -u $3 -T $4
}

function _RunRestRequest2 {
  _log "INFO" "Executing REST request: $1 $2 $3"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -u $4 -T $5
}

function _RunRestRequest3 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -u $5 -T $6
}

function _RunRestRequest4 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -P ''"$4"'' -u $5 -T $6
}

function _RunRestRequest5 {
  _log "INFO" "Executing REST request: $1 $2 $3 @$4"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -f "$4" -u $5 -T $6
}

function _RunRestRequest6 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 $5"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -P ''"$5"'' -u $6 -T $7
}

function _RunRestRequest6d {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 $5"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -P ''"$5"'' -u $6 -T $7 --drop
}

function _RunRestRequest7 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 @$5"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -f "$5" -u $6 -T $7
}

function _RunRestRequest7d {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 @$5"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -f "$5" -u $6 -T $7 --drop
}

function _RunCurlGetDatabasesRequest {
  _log "INFO" "Executing CurlGetDatabasesRequest"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  auth=$(echo "$1:$2" | base64 -w0)
  _log "DEBUG" "auth=${auth}"
  curl -v -H "Authorization: Basic ${auth}" http://localhost:50080/databases
  #echo "Running: curl -v http://$1:$2@localhost:50080/databases"
  #curl -v http://$1:$2@localhost:50080/databases
  echo ""
}

function _RunCurlGetTablesRequest {
  _log "INFO" "Executing CurlGetTablesRequest: $1"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  auth=$(echo "$2:$3" | base64 -w0)
  _log "DEBUG" "auth=${auth}"
  curl -v -H "Authorization: Basic ${auth}" http://localhost:50080/databases/$1/tables
  echo ""
}

function _TestExternalAbort {
  _log "INFO" "Testing an external abort $1"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  pkill -9 siodb
}
