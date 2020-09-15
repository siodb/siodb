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
startup_timeout=15

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
  dd if=/dev/urandom of=/etc/siodb/instances/siodb/master_key bs=16 count=1
  cp -f ${SCRIPT_DIR}/../share/public_key /etc/siodb/instances/siodb/initial_access_key
}

function _StartSiodb {
  _log "INFO" "Starting default Siodb instance"
  ${SIODB_BIN}/siodb --instance siodb --daemon
  sleep ${startup_timeout}
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

function _RunRestRequest1 {
  _log "INFO" "Executing REST request: $1 $2"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -u $3 -T $4
}

function _RunRestRequest2 {
  _log "INFO" "Executing REST request: $1 $2 $3"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -u $4 -T $5
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

function _RunRestRequest7 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 @$5"
  ${SIODB_BIN}/restcli ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -f "$5" -u $6 -T $7
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

export RESTCLI_DEBUG=--debug
##export SIOCLI_DEBUG=--debug

if [[ "${SHORT_TEST}" == "1" ]]; then

  _log "INFO" "There is no short REST test"

else

  _log "INFO" "Creating test objects"
  _RunSqlScript "${SCRIPT_DIR}/create_objects.sql"
  _CheckLogFileError
  
  _log "INFO" "Creating ROOT token"
  _token_file=/tmp/siodbresttest-$(date +%s)-${RANDOM}
  _RunSql "ALTER USER root ADD TOKEN token1" | grep "token:" | cut -d ' ' -f 3 >${_token_file}
  _CheckLogFileError
  root_token=$(cat ${_token_file})
  rm -f ${_token_file}
  echo "ROOT Token: ${root_token}"

  _log "INFO" "Creating USER1 token"
  _token_file=/tmp/siodbresttest-$(date +%s)-${RANDOM}
  _RunSql "ALTER USER user1 ADD TOKEN token1" | grep "token:" | cut -d ' ' -f 3 >${_token_file}
  _CheckLogFileError
  user1_token=$(cat ${_token_file})
  rm -f ${_token_file}
  echo "USER1 Token: ${user1_token}"

  _log "INFO" "Running ROOT tests"

  _RunRestRequest1 get db root ${root_token}
  _CheckLogFileError

  _RunRestRequest2 get table sys root ${root_token}
  _CheckLogFileError

  _RunRestRequest2 get rows sys.sys_tables root ${root_token}
  _CheckLogFileError

  _RunRestRequest3 get row sys.sys_tables 1 root ${root_token}
  _CheckLogFileError

  _log "INFO" "Running USER1 tests"

  _RunRestRequest1 get db user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest2 get table db1 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest2 get table db2 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest2 get table db3 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest2 get rows db1.t1_1 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest2 get rows db1.t1_2 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest2 get rows db2.t2_1 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest3 get row db1.t1_2 1 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest3 get row db1.t1_2 2 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest3 get row db2.t2_1 1 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest4 post row db2.t2_1 "[{\"a\":1, \"b\": \"text 1\"}]" user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest3 get row db2.t2_1 1 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest2 get rows db2.t2_1 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest4 post row db2.t2_1 \
    "[{\"a\":2, \"b\": \"text 2\"}, {\"a\":3, \"b\": \"text 3\"}]" \
    user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest2 get rows db2.t2_1 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest3 delete row db1.t1_2 1 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest3 delete row db1.t1_2 1001 user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest6 patch row db2.t2_1 1 \
    "[{\"a\":10, \"b\": \"text 2 updated\"}]" \
    user1 ${user1_token}
  _CheckLogFileError

  _RunRestRequest6 patch row db2.t2_1 101 \
    "[{\"a\":10, \"b\": \"text 2 updated\"}]" \
    user1 ${user1_token}
  _CheckLogFileError
fi

_StopSiodb
_CheckLogFileError
_log "INFO" "All tests passed"
