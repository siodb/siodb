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

export RESTCLI_DEBUG=--debug
##export SIOCLI_DEBUG=--debug

_log "INFO" "Creating ROOT token"
_token_file=/tmp/siodbresttest-$(date +%s)-${RANDOM}
_RunSql "ALTER USER root ADD TOKEN token1" | grep "token:" | cut -d ' ' -f 3 >${_token_file}
_CheckLogFiles
root_token=$(cat ${_token_file})
rm -f ${_token_file}
echo "ROOT Token: ${root_token}"

if [[ "${SHORT_TEST}" == "1" ]]; then

  _log "INFO" "Running short test"

  _RunCurlGetDatabasesRequest root ${root_token}
  _CheckLogFiles

  _RunCurlGetTablesRequest sys root ${root_token}
  _CheckLogFiles

else

  _log "INFO" "Creating test objects"
  _RunSqlScript "${SCRIPT_DIR}/create_objects.sql"
  _CheckLogFiles

  _log "INFO" "Creating USER1 token"
  _token_file=/tmp/siodbresttest-$(date +%s)-${RANDOM}
  _RunSql "ALTER USER user1 ADD TOKEN token1" | grep "token:" | cut -d ' ' -f 3 >${_token_file}
  _CheckLogFiles
  user1_token=$(cat ${_token_file})
  rm -f ${_token_file}
  echo "USER1 Token: ${user1_token}"

  _log "INFO" "Running ROOT tests"

  _RunRestRequest1 get db root ${root_token}
  _CheckLogFiles

  _RunCurlGetDatabasesRequest root ${root_token}
  _CheckLogFiles

  _RunRestRequest2 get table sys root ${root_token}
  _CheckLogFiles

  _RunCurlGetTablesRequest sys root ${root_token}
  _CheckLogFiles

  _RunCurlGetTablesRequest sys root ${root_token}
  _CheckLogFiles

  _RunRestRequest2 get rows sys.sys_tables root ${root_token}
  _CheckLogFiles

  _RunRestRequest3 get row sys.sys_tables 1 root ${root_token}
  _CheckLogFiles


  _log "INFO" "Running USER1 tests"

  _RunRestRequest1 get db user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest1 get db user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest2 get table db1 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest2 get table db2 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest2 get table db3 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest2 get rows db1.t1_1 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest2 get rows db1.t1_2 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest2 get rows db2.t2_1 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest3 get row db1.t1_2 1 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest3 get row db1.t1_2 2 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest3 get row db2.t2_1 1 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest4 post row db2.t2_1 "[{\"a\":1, \"b\": \"text 1\"}]" user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest3 get row db2.t2_1 1 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest2 get rows db2.t2_1 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest4 post row db2.t2_1 \
    "[{\"a\":2, \"b\": \"text 2\"}, {\"a\":3, \"b\": \"text 3\"}]" \
    user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest2 get rows db2.t2_1 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest3 delete row db1.t1_2 1 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest3 delete row db1.t1_2 1001 user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest6 patch row db2.t2_1 1 \
    "[{\"a\":10, \"b\": \"text 2 updated\"}]" \
    user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest6 patch row db2.t2_1 101 \
    "[{\"a\":10, \"b\": \"text 2 updated\"}]" \
    user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest6 put row db2.t2_1 1 \
    "[{\"a\":10, \"b\": \"text 2 updated\"}]" \
    user1 ${user1_token}
  _CheckLogFiles

  _RunRestRequest6 put row db2.t2_1 101 \
    "[{\"a\":10, \"b\": \"text 2 updated\"}]" \
    user1 ${user1_token}
  _CheckLogFiles


  _log "INFO" "Running special tests"

  # Comment out meanwhile
  # Drop connection test
  #_RunRestRequest6d put row db2.t2_1 101 \
  #  "[{\"a\":10, \"b\": \"text 2 updated\"}]" \
  #  user1 ${user1_token}
  ##_CheckLogFiles

fi

_StopSiodb
_CheckLogFiles

_log "INFO" "All tests passed"
