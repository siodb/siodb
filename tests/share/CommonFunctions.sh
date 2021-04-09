# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# --------------------------------------------------------------
# set
# --------------------------------------------------------------
if [[ "${SIOTEST_TRACE}" == "1" ]]; then
set -x
fi
set -e

# --------------------------------------------------------------
# Global parameters
# --------------------------------------------------------------
UNIQUE_SUFFIX=$(date +%s)_$$
COMMON_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="${COMMON_SCRIPT_DIR}/../"
SHARED_DIR="${COMMON_SCRIPT_DIR}/../share/"
while [[ ! -f "${ROOT_DIR}/.rootdir" ]]; do
  ROOT_DIR=$(realpath "${ROOT_DIR}/..")
done
instanceStartupTimeout=45
previousCheckLogFileEndedAtTimestamp=0
previousInstanceStartTimestamp=0
doInstanceConfiguration=1
defaultClientTimeoutSecond=30

# timeout(1) on Ubuntu 18.04 doesn't support option -v
# Try to detect this.
_timeout_verbose=-v
set +e
timeout -v 10000 "cat /etc/issue" >/dev/null 2>&1
if [[ $? -ne 0 ]]; then unset _timeout_verbose; fi
set -e

# --------------------------------------------------------------
# Include
# --------------------------------------------------------------
source "${SHARED_DIR}/LogFunctions.sh"

# --------------------------------------------------------------
# Command line
# --------------------------------------------------------------
while [[ $# -gt 0 ]];
do
  echo $1
  if [[ "$1" == "--" ]]; then
    break
  fi
  if [[ "$1" == "-b" ]]; then
    if [[ $# -lt 2 ]]; then
      echo "Missing parameter for the option: $1" >&2
      exit 1
    fi
    SIODB_BIN=$2
    shift 2
    continue
  fi
  if [[ "$1" == "-x" ]]; then
    doInstanceConfiguration=0
    instanceStartupTimeout=15
    shift 1
    continue
  fi
  if [[ "$1" == "-e" ]]; then
    if [[ $# -lt 3 ]]; then
      echo "Missing parameters for the option: $1" >&2
      exit 1
    fi
    declare "$2"="$3"
    shift 3
    continue
  fi
  echo "Unrecognized option: $1" >&2
  exit 1
done

# --------------------------------------------------------------
# External Parameters
# --------------------------------------------------------------
if [[ -z "${SIODB_BIN}" ]]; then
    SIODB_BIN=debug
fi
if [[ "${SIODB_BIN}" == "debug" ]]; then
  SIODB_BIN="${ROOT_DIR}/build/debug/bin"
  SHORT_TEST=0
elif [[ "${SIODB_BIN}" == "release" ]]; then
  SIODB_BIN="${ROOT_DIR}/build/release/bin"
  SHORT_TEST=0
elif [[ "${SIODB_BIN}" == "sdebug" ]]; then
  SIODB_BIN="${ROOT_DIR}/build/debug/bin"
  SHORT_TEST=1
elif [[ "${SIODB_BIN}" == "srelease" ]]; then
  SIODB_BIN="${ROOT_DIR}/build/release/bin"
  SHORT_TEST=1
fi
if [[ ! -d "${SIODB_BIN}" ]]; then
  echo "## `date "+%Y-%m-%dT%H:%M:%S"` | ERROR | Invalid Siodb binary directory."
  exit 2
fi
w
if [[ -z "${SIODB_INSTANCE}" ]]; then
  SIODB_INSTANCE=siodb
fi
if [[ -z "${SIODB_DATA_DIR}" ]]; then
  SIODB_DATA_DIR="/var/lib/siodb/${SIODB_INSTANCE}"
fi
if [[ -z "${SIODB_LOG_DIR}" ]]; then
  SIODB_LOG_DIR="/var/log/siodb/${SIODB_INSTANCE}"
fi
if [[ -z "${SIOTEST_KEEP_INSTANCE_UP}" ]]; then
  SIOTEST_KEEP_INSTANCE_UP=0
fi

# --------------------------------------------------------------
# Trapping
# --------------------------------------------------------------
if [[ "${SIOTEST_KEEP_INSTANCE_UP}" == "0" ]]; then
  trap _testfails ERR
fi

# --------------------------------------------------------------
# Global functions
# --------------------------------------------------------------
function _TestBegin {
  _log "INFO" "Test ${TEST_NAME} begins..."
  _TestBeginTimeStamp=$(date +%s)
}

function _TestEnd {
  _TestEndTimeStamp=$(date +%s)
  TestElapsedTime="$(echo "scale=2; ($_TestEndTimeStamp-$_TestBeginTimeStamp)/60" | bc -l)"
  _log "INFO" "SUCCESS: Test passed in ${TestElapsedTime} minutes"
}

function _testfails {
  _killSiodb
}

function _killSiodb {
  if [[ $(ps -eo pid,cmd | egrep "\-\-instance ${SIODB_INSTANCE}$" | wc -l) -gt 0 ]]; then
    for siodb_process in $(ps -eo pid,cmd | egrep "\-\-instance ${SIODB_INSTANCE}$" | awk '{print $1}'); do
      kill -9 ${siodb_process}
    done
  fi
}

function _SetInstanceParameter {
  _log "INFO" "setting parameter '${1}' to '${2}'"
  sed -i -e "s#.*${1}[ ]*=.*#${1} = ${2}#g" \
  /etc/siodb/instances/${SIODB_INSTANCE}/config
  cat /etc/siodb/instances/${SIODB_INSTANCE}/config | grep "${1}"
}

function _GetInstanceParameter {
  _log "INFO" "getting value of parameter '${1}'"
  _GetInstanceParameterReturnedValue=$(
  cat /etc/siodb/instances/${SIODB_INSTANCE}/config \
  | sed 's/^ *//;s/ *$//' \
  | egrep -v '^#|^$' \
  | grep "^${1}" \
  | awk -F '=' '{print $2}' \
  | sed 's/^ *//;s/ *$//'
  )
}

function _ConfigureInstance {
  SIODB_DATA_DIR="/var/lib/siodb/${SIODB_INSTANCE}/data"
  if [[ ! -d "${SIODB_DATA_DIR}" ]]; then
    _log "INFO" "Creating instance data directory ${SIODB_DATA_DIR}"
    mkdir -p ${SIODB_DATA_DIR}
    chmod 770 ${SIODB_DATA_DIR}
  fi

  if [[ "${doInstanceConfiguration}" != "1" ]]; then
    _log "INFO" "Instance configuration parts skipped."
    return
  fi

  _log "INFO" "Configuring instance ${SIODB_INSTANCE}..."

  # Copy default config file
  mkdir -p "/etc/siodb/instances/${SIODB_INSTANCE}"
  cp "${ROOT_DIR}/config/siodb.conf" "/etc/siodb/instances/${SIODB_INSTANCE}/config"
  chmod 660 "/etc/siodb/instances/${SIODB_INSTANCE}/config"

  # Overload default config file
  _SetInstanceParameter "data_dir" "/var/lib/siodb/${SIODB_INSTANCE}/data"
  _SetInstanceParameter "log.file.destination" "/var/log/siodb/${SIODB_INSTANCE}"
  SIODB_LOG_DIR="/var/log/siodb/${SIODB_INSTANCE}"
  mkdir -p ${SIODB_LOG_DIR}
  chmod 770 ${SIODB_LOG_DIR}
  _SetInstanceParameter "enable_rest_server" "yes"
  _SetInstanceParameter "client.enable_encryption" "yes"
  _SetInstanceParameter "client.tls_certificate" "/etc/siodb/instances/${SIODB_INSTANCE}/cert.pem"
  _SetInstanceParameter "client.tls_private_key" "/etc/siodb/instances/${SIODB_INSTANCE}/key.pem"
  _SetInstanceParameter "rest_server.tls_certificate" "/etc/siodb/instances/${SIODB_INSTANCE}/cert.pem"
  _SetInstanceParameter "rest_server.tls_private_key" "/etc/siodb/instances/${SIODB_INSTANCE}/key.pem"
  _SetInstanceParameter "rest_server.iomgr_read_timeout" "60"
  _SetInstanceParameter "log.file.severity" "debug"

  cp "${ROOT_DIR}/config/sample_cert/cert.pem" \
      "/etc/siodb/instances/${SIODB_INSTANCE}/cert.pem"
  chmod 660 "/etc/siodb/instances/${SIODB_INSTANCE}/cert.pem"
  cp "${ROOT_DIR}/config/sample_cert/key.pem" \
      "/etc/siodb/instances/${SIODB_INSTANCE}/key.pem"
  chmod 660 "/etc/siodb/instances/${SIODB_INSTANCE}/key.pem"

  dd if=/dev/urandom of="/etc/siodb/instances/${SIODB_INSTANCE}/master_key" bs=16 count=1
  chmod 660 "/etc/siodb/instances/${SIODB_INSTANCE}/master_key"
  cp -f "${ROOT_DIR}/tests/share/public_key" \
    "/etc/siodb/instances/${SIODB_INSTANCE}/initial_access_key"
  chmod 660 "/etc/siodb/instances/${SIODB_INSTANCE}/initial_access_key"
}

function _Prepare {
  _log "INFO" "Cleanup traces of previous default instance"

  _ConfigureInstance

  _killSiodb

  if [[ -d "${SIODB_DATA_DIR}" ]]; then
    _log "INFO"  "Purging directory '${SIODB_DATA_DIR}'"
    rm -rf "${SIODB_DATA_DIR}"/*
    rm -rf "${SIODB_DATA_DIR}/.initialized"
    echo "Contents of the ${SIODB_DATA_DIR} after cleanup"
    ls -la "${SIODB_DATA_DIR}"
  else
    _log "ERROR" "Data directory '${SIODB_DATA_DIR}' doesn't exist or not a directory."
    _failExit
  fi

  if [[ -d "${SIODB_LOG_DIR}" ]]; then
    _log "INFO" "Purging directory '${SIODB_LOG_DIR}'"
    rm -rf "${SIODB_LOG_DIR}"/*
    echo "Contents of the ${SIODB_LOG_DIR} after cleanup"
    ls -la "${SIODB_LOG_DIR}"
  else
    _log "ERROR" "Log directory '${SIODB_LOG_DIR}' doesn't exist or not a directory."
    _failExit
  fi
}

function _ShowSiodbProcesses {
  echo "===== Siodb Processes ====="
  if [[ $(pgrep -c siodb) -gt 0 ]]; then
    pgrep -a siodb
  fi
  echo "==========================="
}

function _StartSiodb {
  _log "INFO" "Starting Siodb instance '${SIODB_INSTANCE}'"
  "${SIODB_BIN}/siodb" --instance ${SIODB_INSTANCE} --daemon
  numberEntriesInLog=0
  counterTimeout=0
  _log "INFO" "Waiting for Siodb instance to start..."
  while [[ $numberEntriesInLog -eq 0 ]]; do
    LOG_STARTUP=$(cat ${SIODB_LOG_DIR}/*.log \
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
          }
        ')
    numberEntriesInLog=$(echo "${LOG_STARTUP}" | egrep 'Listening for (TCP|UNIX) connections' \
        | wc -l | bc)
    if [[ ${counterTimeout} -gt ${instanceStartupTimeout} ]]; then
      _log "ERROR" \
          "Timeout (${instanceStartupTimeout} seconds) reached while starting the instance..."
      _failExit
    fi
    counterTimeout=$((counterTimeout+1))
    sleep 1
  done
  previousInstanceStartTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  _ShowSiodbProcesses
}

function _RestartSiodb {
  _log "INFO" "Restarting instance..."
  _StopSiodb
  _StartSiodb
}

function _StopSiodb {
  _log "INFO" "Stoping instance..."
  SIOTEST_KEEP_INSTANCE_UP_VALUE_SAVED=${SIOTEST_KEEP_INSTANCE_UP}
  SIOTEST_KEEP_INSTANCE_UP=0
  _StopSiodbAndWaitUntilStopped
  SIOTEST_KEEP_INSTANCE_UP=${SIOTEST_KEEP_INSTANCE_UP_VALUE_SAVED}
}

function _FinalStopOfSiodb {
  if [[ "${SIOTEST_KEEP_INSTANCE_UP}" == "0" ]]; then
    _StopSiodbAndWaitUntilStopped
  else
    _log "INFO" "Siodb process kept in running state"
  fi
}

function _StopSiodbAndWaitUntilStopped {
  _log "INFO" "Stopping Siodb process on default instance"
  _ShowSiodbProcesses
  SIODB_PROCESS_ID=$(ps -ef | grep "siodb --instance ${SIODB_INSTANCE} --daemon" \
      | grep -v grep | awk '{print $2}')
  if [[ -z "${SIODB_PROCESS_ID}" ]]; then
    echo "Siodb is already not running."
  else
    kill -SIGINT ${SIODB_PROCESS_ID}
    counterSiodbProcesses=$(ps -ef | grep "siodb --instance ${SIODB_INSTANCE} --daemon" \
        | grep -v grep | awk '{print $2}' | wc -l | bc)
    counterTimeout=0
    _log "INFO" "Waiting for Siodb instance to stop..."
    while [[ $counterSiodbProcesses -ne 0 ]]; do
      counterSiodbProcesses=$(ps -ef | grep "siodb --instance ${SIODB_INSTANCE} --daemon" \
          | grep -v grep | awk '{print $2}' | wc -l | bc)
      if [[ ${counterTimeout} -gt ${instanceStartupTimeout} ]]; then
        _log "ERROR" \
            "Timeout (${instanceStartupTimeout} seconds) reached while stopping the instance..."
        _failExit
      fi
      counterTimeout=$((counterTimeout+1))
      sleep 1
    done
  fi
  echo "After stopping:"
  _ShowSiodbProcesses
}

function _CheckLogFiles {
  # $1: exclude these patterns because expected
  _log "INFO" "Checking for errors in the log files"
  for logFile in $(ls "${SIODB_LOG_DIR}"); do
    LOG_ERROR=$(cat "${SIODB_LOG_DIR}/${logFile}" \
    | awk -v previousCheckLogFileEndedAtTimestamp=${previousCheckLogFileEndedAtTimestamp} \
    '
      function ltrim(s) { sub(/^[ \t\r\n]+/, "", s); return s }
      function rtrim(s) { sub(/[ \t\r\n]+$/, "", s); return s }
      function trim(s)  { return rtrim(ltrim(s)); }
      {
        lineTimestamp = substr($1,1,4)substr($1,6,2)substr($1,9,2)substr($2,1,2)substr($2,4,2)substr($2,7,2)trim(substr($2,10,6))
        if (lineTimestamp > previousCheckLogFileEndedAtTimestamp && $3 == "error") {
          print $0;
        }
      }
    ')

    if [[ "${1}" == "" ]]; then
      ERROR_COUNT=$(echo "${LOG_ERROR}" | grep error | wc -l)
    else
      ERROR_COUNT=$(echo "${LOG_ERROR}" | grep error | egrep -v "${1}" | wc -l)
    fi

    if [[ ${ERROR_COUNT} -ne 0 ]]; then
      _log "ERROR" "Found an issue in the log file ${SIODB_LOG_DIR}/${logFile}"
      echo "## ================================================="
      echo "${LOG_ERROR}"
      echo "## ================================================="
      _failExit
    fi
  done

  _log "INFO" "No error detected the in log files"
  previousCheckLogFileEndedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
}

function _failExit {
    echo "Test failed."
    if [[ "${SIOTEST_KEEP_INSTANCE_UP}" == "0" ]]; then
      _testfails
    fi
    exit 3
}

function _RunSqlScript {
  # $1: Path to SQL script
  # $2: Optional timeout
  if [[ ! -z "${2}" ]]; then TIMEOUT_SECOND="${2}"; else TIMEOUT_SECOND=${defaultClientTimeoutSecond}; fi
  _log "INFO" "Executing SQL script $1"
  timeout ${_timeout_verbose} --preserve-status ${TIMEOUT_SECOND} "${SIODB_BIN}/siocli" ${SIOCLI_DEBUG} \
  --nologo --admin ${SIODB_INSTANCE} -u root \
    -i "${ROOT_DIR}/tests/share/private_key" < $1
}

function _RunSqlThroughUserUnixSocket {
  # $1: Path to SQL script
  # $2: Siodb user
  # $3: Siodb user key
  # $4: Optional timeout
  if [[ ! -z "${4}" ]]; then TIMEOUT_SECOND="${4}"; else TIMEOUT_SECOND=${defaultClientTimeoutSecond}; fi
  _log "INFO" "Executing SQL (user: ${1}, pkey: ${2}): ${3}"
  timeout ${_timeout_verbose} --preserve-status ${TIMEOUT_SECOND} "${SIODB_BIN}/siocli" ${SIOCLI_DEBUG} \
  --admin ${SIODB_INSTANCE} --nologo -u ${1} -i ${2} <<< ''"${3}"''
}

function _RunSqlThroughUserTCPSocket {
  # $1: Path to SQL script
  # $2: Siodb user
  # $3: Siodb user key
  # $4: Optional timeout
  if [[ ! -z "${4}" ]]; then TIMEOUT_SECOND="${4}"; else TIMEOUT_SECOND=${defaultClientTimeoutSecond}; fi
  _log "INFO" "Executing SQL (user: ${1}, pkey: ${2}): ${3}"
  timeout ${_timeout_verbose} --preserve-status ${TIMEOUT_SECOND} "${SIODB_BIN}/siocli" ${SIOCLI_DEBUG} \
  --nologo -u ${1} -i ${2} <<< ''"${3}"''
}

function _RunSql {
  # $1: SQL to run
  # $2: Optional timeout
  if [[ ! -z "${2}" ]]; then TIMEOUT_SECOND="${2}"; else TIMEOUT_SECOND=${defaultClientTimeoutSecond}; fi
  _log "INFO" "Executing SQL: >$1<"
  timeout ${_timeout_verbose} --preserve-status ${TIMEOUT_SECOND} "${SIODB_BIN}/siocli" ${SIOCLI_DEBUG} \
  --nologo --admin ${SIODB_INSTANCE} -u root \
  -i "${ROOT_DIR}/tests/share/private_key" <<< ''"$1"''
}

function _RunSqlAndValidateOutput {
  # $1: the SQL to execute
  # $2: The expected output
  # $3: Optional timeout
  if [[ ! -z "${3}" ]]; then TIMEOUT_SECOND="${3}"; else TIMEOUT_SECOND=${defaultClientTimeoutSecond}; fi
  _log "INFO" "Executing SQL: >$1<"
  SIOCLI_OUTPUT=$(timeout ${_timeout_verbose} --preserve-status ${TIMEOUT_SECOND} ${SIODB_BIN}/siocli \
    ${SIOCLI_DEBUG} --nologo --admin ${SIODB_INSTANCE} -u root --keep-going \
    -i "${ROOT_DIR}/tests/share/private_key" <<< ''"${1}"'')
  EXPECTED_RESULT_COUNT=$(echo "${SIOCLI_OUTPUT}" | sed 's/^ *//;s/ *$//' | egrep "${2}" | wc -l | bc)
  if [[ ${EXPECTED_RESULT_COUNT} -eq 0 ]]; then
    _log "ERROR" "Siocli output does not match expected output. Output is: ${SIOCLI_OUTPUT}"
    _failExit "${2}"
  else
    _log "INFO" "Siocli output matched expected output."
  fi
}

function _RunRestRequest1 {
  _log "INFO" "Executing REST request: $1 $2"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -u $3 -T $4
}

function _RunRestRequest2 {
  _log "INFO" "Executing REST request: $1 $2 $3"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -u $4 -T $5
}

function _RunRestRequest3 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -u $5 -T $6
}

function _RunRestRequest4 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -P ''"$4"'' \
    -u $5 -T $6
}

function _RunRestRequest5 {
  _log "INFO" "Executing REST request: $1 $2 $3 @$4"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -f "$4" -u $5 -T $6
}

function _RunRestRequest6 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 $5"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -P ''"$5"'' \
    -u $6 -T $7
}

function _RunRestRequest6d {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 $5"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -P ''"$5"'' \
    -u $6 -T $7 --drop
}

function _RunRestRequest7 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 @$5"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -f "$5" -u $6 \
    -T $7
}

function _RunRestRequest7d {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 @$5"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -f "$5" -u $6 \
    -T $7 --drop
}

function _RunCurlGetDatabasesRequest {
  _log "INFO" "Executing CurlGetDatabasesRequest"
  auth=$(echo "$1:$2" | base64 -w0)
  _log "DEBUG" "auth=${auth}"
  curl -v -H "Authorization: Basic ${auth}" http://localhost:50080/databases
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

_log "INFO" "Common parts applied"
