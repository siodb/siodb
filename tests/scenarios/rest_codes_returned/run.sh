#!/bin/bash

# Copyright (C) 2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions
urlencode (){
   echo "${1}" | curl -Gso /dev/null -w %{url_effective} --data-urlencode @- "" | cut -c 3-
}

executeRestRequest_GET (){
    curl_options="-k -s -o /dev/null -w %{http_code}"
    _log "INFO" "curl ${curl_options} ${1}"
    STATUS=$(curl ${curl_options} ${1})
    if [[ $STATUS -ne ${2} ]]; then
        _log "ERROR" "curl returned ${STATUS}. Expected: ${2}"
        _failExit
    fi
}

executeRestRequest_POST (){
    curl_options="-k -s -o /dev/null -w %{http_code}"
    _log "INFO" "curl ${curl_options} -X POST -d '${3}' ${1}"
    STATUS=$(curl ${curl_options} -X POST -d "${3}" ${1})
    if [[ $STATUS -ne ${2} ]]; then
        _log "ERROR" "curl returned ${STATUS}. Expected: ${2}"
        _failExit
    fi
}

executeRestRequest_PUT (){
    curl_options="-k -s -o /dev/null -w %{http_code}"
    _log "INFO" "curl ${curl_options} -X PUT -d '${3}' ${1}"
    STATUS=$(curl ${curl_options} -X PUT -d "${3}" ${1})
    if [[ $STATUS -ne ${2} ]]; then
        _log "ERROR" "curl returned ${STATUS}. Expected: ${2}"
        _failExit
    fi
}

executeRestRequest_DELETE (){
    curl_options="-k -s -o /dev/null -w %{http_code}"
    _log "INFO" "curl ${curl_options} -X DELETE ${1}"
    STATUS=$(curl ${curl_options} -X DELETE ${1})
    if [[ $STATUS -ne ${2} ]]; then
        _log "ERROR" "curl returned ${STATUS}. Expected: ${2}"
        _failExit
    fi
}

## Specific test parameters

## =============================================
## TEST HEADER
## =============================================
_TestBegin
_Prepare
_StartSiodb

## =============================================
## TEST
## =============================================

### Vars
database_name=db1
table_name=table1
TOKEN="$(openssl rand -hex 64)"
WRONG_TOKEN="$(openssl rand -hex 64)"
QUERY_1="select t.name tname, c.name cname from sys_tables t, sys_columns c where t.trid = c.table_id"
QUERY_2="select * from ${database_name}.${table_name}"
QUERY_3="select * from db1.t1 tab1, db1.t2 tab2, db1.t3 tab3 where tab1.trid = tab2.trid and tab2.trid=tab3.trid"
QUERY_WITH_ERROR="select * from not_exists"

## Data model
_RunSql "create database ${database_name}"
_RunSql "create table ${database_name}.${table_name} ( col1 text )"
_RunSql "create user user1"
_RunSql "alter user user1 add token test_token x'${TOKEN}'"
_RunSql "alter user root add token test_token x'${TOKEN}'"
_RunSqlScript "${SCRIPT_DIR}/create_objects.sql" 90

## AUTH
executeRestRequest_GET "https://root:faketoken@localhost:50443/databases" 401
executeRestRequest_GET "https://root:${WRONG_TOKEN}@localhost:50443/databases" 401
executeRestRequest_GET "https://fakeuser:${WRONG_TOKEN}@localhost:50443/databases" 401

## GET
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/databases//tables" 400
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables//rows" 400
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables" 200
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/databases/fakedatabase/tables" 404
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/sys_tables/rows" 200
executeRestRequest_GET \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/sys_tables/rows" 404
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows" 200
executeRestRequest_GET \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows" 200
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/1" 404
executeRestRequest_GET \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/1" 404
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/faketable/rows" 404
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/databases/fakedatabase/tables/faketable/rows" 404
executeRestRequest_GET \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/faketable/rows" 404
executeRestRequest_GET \
    "https://user1:${TOKEN}@localhost:50443/databases/fakedatabase/tables/faketable/rows" 404

## GET Query
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/query?q=$(urlencode "${QUERY_1}")" 200
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/query?q=$(urlencode "${QUERY_2}")" 200
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/query?q=$(urlencode "${QUERY_3}")" 200
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/query?q=$(urlencode "${QUERY_WITH_ERROR}")" 400
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/query?q1=$(urlencode "${QUERY_1}")\
&q2=$(urlencode "${QUERY_2}")\
&q3=$(urlencode "${QUERY_3}")\
&q4=$(urlencode "${QUERY_WITH_ERROR}")" 200
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/query?q1=$(urlencode "${QUERY_WITH_ERROR}")\
&q2=$(urlencode "${QUERY_3}")\
&q3=$(urlencode "${QUERY_2}")\
&q4=$(urlencode "${QUERY_1}")" 200

///////////////////////////
///////////////////////////  Add in README a snippet with SQL / REST
///////////////////////////


## POST
executeRestRequest_POST \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows" \
    201 '[{"col1":"value"}]'
executeRestRequest_POST \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows" \
    201 '[{"col1":"value"}]'
executeRestRequest_POST \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/sys_tables/rows" \
    403 '[{"col1":"value"}]'
executeRestRequest_POST \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/sys_tables/rows" \
    404 '[{"col1":"value"}]'
executeRestRequest_POST \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/faketable/rows" \
    404 '[{"col1":"value"}]'
executeRestRequest_POST \
    "https://root:${TOKEN}@localhost:50443/databases/fakedatabase/tables/faketable/rows" \
    404 '[{"col1":"value"}]'
executeRestRequest_POST \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/faketable/rows" \
    404 '[{"col1":"value"}]'
executeRestRequest_POST \
    "https://user1:${TOKEN}@localhost:50443/databases/fakedatabase/tables/faketable/rows" \
    404 '[{"col1":"value"}]'

## PUT
executeRestRequest_PUT \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/1" \
    200 '[{"col1":"value_updated"}]'
executeRestRequest_PUT \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/1" \
    200 '[{"col1":"value_updated"}]'
executeRestRequest_PUT \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/sys_tables/rows/1" \
    403 '[{"col1":"value"}]'
executeRestRequest_PUT \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/sys_tables/rows/1" \
    404 '[{"col1":"value"}]'
executeRestRequest_PUT \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/sys_tables/rows/999" \
    403 '[{"col1":"value"}]'
executeRestRequest_PUT \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/sys_tables/rows/999" \
    404 '[{"col1":"value"}]'
executeRestRequest_PUT \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/faketable/rows/1" \
    404 '[{"col1":"value"}]'
executeRestRequest_PUT \
    "https://root:${TOKEN}@localhost:50443/databases/fakedatabase/tables/faketable/rows/1" \
    404 '[{"col1":"value"}]'
executeRestRequest_PUT \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/faketable/rows/1" \
    404 '[{"col1":"value"}]'
executeRestRequest_PUT \
    "https://user1:${TOKEN}@localhost:50443/databases/fakedatabase/tables/faketable/rows/1" \
    404 '[{"col1":"value"}]'

## GET Rows
executeRestRequest_GET \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/1" 200
executeRestRequest_GET \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/1" 200

## DELETE
executeRestRequest_DELETE \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/1" 200
executeRestRequest_DELETE \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/2" 200
executeRestRequest_DELETE \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/sys_tables/rows/1" 403
executeRestRequest_DELETE \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/sys_tables/rows/1" 404
executeRestRequest_DELETE \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/999" 404
executeRestRequest_DELETE \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/${table_name}/rows/999" 404
executeRestRequest_DELETE \
    "https://root:${TOKEN}@localhost:50443/databases/${database_name}/tables/faketable/rows/1" 404
executeRestRequest_DELETE \
    "https://root:${TOKEN}@localhost:50443/databases/fakedatabase/tables/faketable/rows/1" 404
executeRestRequest_DELETE \
    "https://user1:${TOKEN}@localhost:50443/databases/${database_name}/tables/faketable/rows/1" 404
executeRestRequest_DELETE \
    "https://user1:${TOKEN}@localhost:50443/databases/fakedatabase/tables/faketable/rows/1" 404


## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_TestEnd
exit 0
