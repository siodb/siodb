#!/bin/bash

# Copyright (C) 2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions
function build_insert_all_statement () {
    trid=$((trid+1))
    all_columns="$(printf ', %s' "${columns[@]}")"
    all_values="$(printf ', %s' "${values[@]}")"
    sql_insert_all_cols="insert into tdb.ttable ( ${all_columns:1} ) values ( ${all_values:1} )"
    echo ${sql_insert_all_cols}
    _RunSqlAndValidateOutput "${sql_insert_all_cols}" "^1 rows affected"
}

function build_insert_some_statement () {
    trid=$((trid+1))
    save_some_state=()
    some_columns=""
    some_values=""
    for ((idx = 0; idx < $((${#columns[@]}/2)); ++idx)); do
        random_col_idx=$(($RANDOM % ${#columns[@]}))
        if [[ ! ${save_some_state[@]} =~ ${random_col_idx} ]]; then
            save_some_state+=${random_col_idx}
            some_columns="${some_columns}, ${columns[$random_col_idx]}"
            some_values="${some_values}, ${values[$random_col_idx]}"
        fi
    done
    sql_insert_some_cols="insert into tdb.ttable ( ${some_columns:1} ) values ( ${some_values:1} )"
    echo ${sql_insert_some_cols}
    _RunSqlAndValidateOutput "${sql_insert_some_cols}" "^1 rows affected"
}

function build_update_all_columns_statement () {
    trid_to_update=${1}
    update_columns=""
    for ((idx = 0; idx < $((${#columns[@]})); ++idx)); do
        update_columns="${update_columns}, ${columns[$idx]} = ${values[$idx]}"
    done
    sql_update_all_cols="update tdb.ttable set ${update_columns:1} where trid = ${trid_to_update}"
    echo ${sql_update_all_cols}
    _RunSqlAndValidateOutput "${sql_update_all_cols}" "^1 rows affected"
}

function build_update_some_columns_statement () {
    trid_to_update=${1}
    save_some_state=()
    update_columns=""
    for ((idx = 0; idx < $((${#columns[@]}/2)); ++idx)); do
        random_col_idx=$(($RANDOM % ${#columns[@]}))
        if [[ ! ${save_some_state[@]} =~ ${random_col_idx} ]]; then
            save_some_state+=${random_col_idx}
            update_columns="${update_columns}, ${columns[$random_col_idx]} = ${values[$random_col_idx]}"
        fi
    done
    sql_update_some_cols="update tdb.ttable set ${update_columns:1} where trid = ${trid_to_update}"
    echo ${sql_update_some_cols}
    _RunSqlAndValidateOutput "${sql_update_some_cols}" "^1 rows affected"
}

function build_delete_random_statement () {
    trid_max=${1}
    delete_executed=0
    while [[ ${delete_executed} -eq 0 ]]; do
        trid_to_delete=$((($RANDOM % ((${trid_max}-1)))+1))
        if [[ ! ${trid_deleted[@]} =~ ${trid_to_delete} ]]; then
            trid_deleted+=${trid_to_delete}
            sql_delete_random="delete from tdb.ttable where trid = ${trid_to_delete}"
            echo ${sql_delete_random}
            _RunSqlAndValidateOutput "${sql_delete_random}" "^1 rows affected"
            delete_executed=1
        fi
    done
}

## Specific test parameters
columns[0]="col_integer"
columns[1]="col_int"
columns[2]="col_uint"
columns[3]="col_tinyint"
columns[4]="col_tinyuint"
columns[5]="col_smallint"
columns[6]="col_smalluint"
columns[7]="col_bigint"
columns[8]="col_biguint"
columns[9]="col_smallreal"
columns[10]="col_real"
columns[11]="col_float"
columns[12]="col_double"
columns[13]="col_text"
columns[14]="col_char"
columns[15]="col_varchar"
columns[16]="col_blob"
columns[17]="col_timestamp"

values[0]="382 "
values[1]="-182"
values[2]="4294967295"
values[3]="-127"
values[4]="127"
values[5]="-32767"
values[6]="65535"
values[7]="-4611686018427387904"
values[8]="9223372036854775808"
values[9]="45.3112"
values[10]="45.3112"
values[11]="45.3112"
values[12]="4125.3112"
values[13]="'a)&☻题为❤♫'"
values[14]="'a'"
values[15]="'a)&❤♫☻题为'"
values[16]="'0x73696f62'"
values[17]="'2016-02-29 12:10:29.123456789'"


## =============================================
## TEST HEADER
## =============================================
_TestBegin
_Prepare
_StartSiodb

## =============================================
## TEST
## =============================================


_RunSqlScript "${SCRIPT_DIR}/setup.sql" 120
_CheckLogFiles


trid_deleted=()
trid=0
for i in {1..100}; do
    build_insert_all_statement
    build_update_some_columns_statement ${trid}

    build_insert_all_statement
    build_update_all_columns_statement ${trid}

    build_insert_all_statement
    build_update_some_columns_statement ${trid}
    build_update_all_columns_statement ${trid}
    build_update_some_columns_statement ${trid}
    build_update_all_columns_statement ${trid}

    build_delete_random_statement ${trid}

    build_insert_some_statement
    build_update_some_columns_statement ${trid}

    build_insert_some_statement
    build_update_all_columns_statement ${trid}

    build_insert_some_statement
    build_update_some_columns_statement ${trid}
    build_update_all_columns_statement ${trid}
    build_update_some_columns_statement ${trid}
    build_update_all_columns_statement ${trid}

    build_delete_random_statement ${trid}
done
_CheckLogFiles


## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
