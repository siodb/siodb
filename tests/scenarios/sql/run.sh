#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Program
_TestBegin
_Prepare
_StartSiodb
_CheckLogFiles

##export SIOCLI_DEBUG=--debug

if [[ "${SHORT_TEST}" == "1" ]]; then

  _RunSql "SELECT * FROM SYS.SYS_TABLES"
  _CheckLogFiles

  _RunSql "SELECT * FROM SYS.SYS_DATABASES"
  _CheckLogFiles

else

  _RunSqlScript "${SCRIPT_DIR}/query_sys_tables.sql" 120
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/ddl_database.sql" 120
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/ddl_table.sql" 120
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/ddl_user.sql" 120
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/dml_general.sql" 120
  _CheckLogFiles

  _RunSqlScript "${SCRIPT_DIR}/dml_datetime.sql" 120
  _CheckLogFiles

fi

_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
