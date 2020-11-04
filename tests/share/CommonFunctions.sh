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
SCRIPT=$(realpath "$0")
SCRIPT_DIR=$(dirname "${SCRIPT}")
SCRIPT_DIR=$(realpath "${SCRIPT_DIR}")
ROOT_DIR=${SCRIPT_DIR}
while [[ ! -f "${ROOT_DIR}/.rootdir" ]]; do
  ROOT_DIR=$(realpath "${ROOT_DIR}/..")
done

instanceStartupTimeout=45
previousTestStartedAtTimestamp=0
previousInstanceStartTimestamp=0
doInstanceConfiguration=1
defaultClientTimeoutSecond=30

# --------------------------------------------------------------
# Command line
# --------------------------------------------------------------
while [[ $# -gt 0 ]];
do
  if [[ "$1" == "--" ]]; then
    break
  fi
  if [[ "$1" == "-b" ]]; then
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
if [[ -z "${CLIENT_TIMEOUT_SECOND}" ]]; then
  CLIENT_TIMEOUT_SECOND=30
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

function _ConfigureInstance {
  if [[ "${doInstanceConfiguration}" != "1" ]]; then
    _log "INFO" "Instance configuration skipped."
    return
  fi

  _log "INFO" "Configuring instance ${SIODB_INSTANCE}..."

  # Copy default config file
  mkdir -p "/etc/siodb/instances/${SIODB_INSTANCE}"
  cp "${ROOT_DIR}/config/siodb.conf" "/etc/siodb/instances/${SIODB_INSTANCE}/config"
  chmod 660 "/etc/siodb/instances/${SIODB_INSTANCE}/config"

  # Overload default config file
  _SetInstanceParameter "data_dir" "/var/lib/siodb/${SIODB_INSTANCE}/data"
  SIODB_DATA_DIR="/var/lib/siodb/${SIODB_INSTANCE}/data"
  mkdir -p ${SIODB_DATA_DIR}
  chmod 770 ${SIODB_DATA_DIR}
  _SetInstanceParameter "log.file.destination" "/var/log/siodb/${SIODB_INSTANCE}"
  SIODB_LOG_DIR="/var/log/siodb/${SIODB_INSTANCE}"
  mkdir -p ${SIODB_LOG_DIR}
  chmod 770 ${SIODB_LOG_DIR}
  _SetInstanceParameter "enable_rest_server" "yes"
  _SetInstanceParameter "client.enable_encryption" "yes"
  _SetInstanceParameter "client.tls_certificate" "cert.pem"
  _SetInstanceParameter "client.tls_private_key" "key.pem"
  _SetInstanceParameter "rest_server.tls_certificate" "cert.pem"
  _SetInstanceParameter "rest_server.tls_private_key" "key.pem"
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
    echo "Contents of the ${SIODB_DATA_DIR}  after cleanup"
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
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
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
    _CheckLogFiles 1>/dev/null
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
  else
    _log "INFO" "Siodb process kept in running state"
  fi
}

function _CheckLogFiles {
  # $1: exclude these patterns because expected
  _log "INFO" "Checking for errors in the log files"
  foundErrors=0
  for logFile in $(ls "${SIODB_LOG_DIR}"); do
    LOG_ERROR=$(cat "${SIODB_LOG_DIR}/${logFile}" \
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
      }
    ')

    if [[ "${1}" == "" ]]; then
      ERROR_COUNT=$(echo "${LOG_ERROR}" | grep error | wc -l)
    else
      ERROR_COUNT=$(echo "${LOG_ERROR}" | grep error | egrep -v "${1}" | wc -l)
    fi

    if [[ "${ERROR_COUNT}" -ne "0" ]]; then
      foundErrors=1
      _log "ERROR" "Found an issue in the log file ${SIODB_LOG_DIR}/${logFile}"
      echo "## ================================================="
      echo "${LOG_ERROR}"
      echo "## ================================================="
    fi
  done

  echo "foundErrors=${foundErrors}"
  if [[ "${foundErrors}" == "1" ]]; then
    _failExit
  fi

  _log "INFO" "No error detected the in log files"
}

function _log {
    echo "## `date "+%Y-%m-%dT%H:%M:%S"` | $1 | $2"
}

function _failExit {
    echo "Test failed."
    if [[ "${SIOTEST_KEEP_INSTANCE_UP}" == "0" ]]; then
      _testfails
    fi
    exit 3
}

function _RunSqlScript {
  # $2: Optional timeout
  if [[ ! -z "${2}" ]]; then TIMEOUT_SECOND="${2}"; fi
  _log "INFO" "Executing SQL script $1"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  timeout -v --preserve-status ${TIMEOUT_SECOND} "${SIODB_BIN}/siocli" ${SIOCLI_DEBUG} \
  --nologo --admin ${SIODB_INSTANCE} -u root \
    -i "${ROOT_DIR}/tests/share/private_key" < $1
}

function _RunSqlThroughUser {
  # $2: Optional timeout
  if [[ ! -z "${2}" ]]; then TIMEOUT_SECOND="${2}"; fi
  _log "INFO" "Executing SQL (user: ${1}, pkey: ${2}): ${3}"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  timeout -v --preserve-status ${TIMEOUT_SECOND} "${SIODB_BIN}/siocli" ${SIOCLI_DEBUG} \
  --nologo -u ${1} -i ${2} <<< ''"${3}"''
}

function _RunSql {
  # $2: Optional timeout
  if [[ ! -z "${2}" ]]; then TIMEOUT_SECOND="${2}"; fi
  _log "INFO" "Executing SQL: $1"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  timeout -v --preserve-status ${TIMEOUT_SECOND} "${SIODB_BIN}/siocli" ${SIOCLI_DEBUG} \
  --nologo --admin ${SIODB_INSTANCE} -u root \
  -i "${ROOT_DIR}/tests/share/private_key" <<< ''"$1"''
}

function _RunSqlAndValidateOutput {
  # $1: the SQL to execute
  # $2: The expected output
  # $3: Optional timeout
  if [[ ! -z "${3}" ]]; then TIMEOUT_SECOND="${3}"; fi
  _log "INFO" "Executing SQL: $1"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  SIOCLI_OUTPUT=$(timeout -v --preserve-status ${TIMEOUT_SECOND} ${SIODB_BIN}/siocli \
    ${SIOCLI_DEBUG} --nologo --admin ${SIODB_INSTANCE} -u root --keep-going \
    -i "${ROOT_DIR}/tests/share/private_key" <<< ''"${1}"'')
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
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -u $3 -T $4
}

function _RunRestRequest2 {
  _log "INFO" "Executing REST request: $1 $2 $3"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -u $4 -T $5
}

function _RunRestRequest3 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -u $5 -T $6
}

function _RunRestRequest4 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -P ''"$4"'' \
    -u $5 -T $6
}

function _RunRestRequest5 {
  _log "INFO" "Executing REST request: $1 $2 $3 @$4"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -f "$4" -u $5 -T $6
}

function _RunRestRequest6 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 $5"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -P ''"$5"'' \
    -u $6 -T $7
}

function _RunRestRequest6d {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 $5"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -P ''"$5"'' \
    -u $6 -T $7 --drop
}

function _RunRestRequest7 {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 @$5"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -f "$5" -u $6 \
    -T $7
}

function _RunRestRequest7d {
  _log "INFO" "Executing REST request: $1 $2 $3 $4 @$5"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  "${SIODB_BIN}/restcli" ${RESTCLI_DEBUG} --nologo -m $1 -t $2 -n $3 -i $4 -f "$5" -u $6 \
    -T $7 --drop
}

function _RunCurlGetDatabasesRequest {
  _log "INFO" "Executing CurlGetDatabasesRequest"
  previousTestStartedAtTimestamp="$(date=$(date +'%Y%m%d%H%M%S%N'); echo ${date:0:-3})"
  auth=$(echo "$1:$2" | base64 -w0)
  _log "DEBUG" "auth=${auth}"
  curl -v -H "Authorization: Basic ${auth}" http://localhost:50080/databases
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
