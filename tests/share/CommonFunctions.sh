set -e
trap _testfails ERR

function _testfails {
  _killSiodb
}

function _killSiodb {
  if [[ $(pkill -c siodb) -gt 0 ]]; then
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
    echo "Contents of the ${LOG_DIR}  after cleanup"
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
    _log "INFO" "No error detected in the log files"
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
