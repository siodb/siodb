#!/bin/bash

# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
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

  # Test query result set
  _RunSqlScript "${SCRIPT_DIR}/query_test_data_model.sql" 120
  _RunSqlAndValidateOutput "select *
                            from db1.t1 tab1, db1.t2 tab2, db1.t3 tab3
                            where tab1.trid = tab2.trid and tab2.trid = tab3.trid" \
                            '^.*1.*1.*a1.*1.*1.*a2.*1.*1.*a2.*$'
  _RunSqlAndValidateOutput "select *
                            from db1.t1 tab1, db1.t2 tab2, db1.t3 tab3
                            where tab1.trid = tab2.trid and tab2.trid = tab3.trid" \
                            '^.*2.*2.*b1.*2.*2.*b2.*2.*2.*b2.*$'
  _RunSqlAndValidateOutput "select *
                            from db1.t1 tab1, db1.t2 tab2, db1.t3 tab3
                            where tab1.trid = tab2.trid and tab2.trid = tab3.trid" \
                            '^.*3.*3.*c1.*3.*3.*c2.*3.*3.*c2.*$'
  _RunSqlAndValidateOutput "select *
                            from db1.t1 tab1, db1.t2 tab2, db1.t3 tab3
                            where tab1.trid = tab2.trid and tab2.trid = tab3.trid" \
                            '^.*4.*4.*d1.*4.*4.*d2.*4.*4.*d2.*$'
  _RunSqlAndValidateOutput "select tab1.*
                            from db1.t1 tab1, db1.t2 tab2, db1.t3 tab3
                            where tab1.trid = tab2.trid and tab2.trid = tab3.trid" \
                            '^.*4.*4.*d1.*$'
  _RunSqlAndValidateOutput "select tab1.*, tab2.*
                            from db1.t1 tab1, db1.t2 tab2, db1.t3 tab3
                            where tab1.trid = tab2.trid and tab2.trid = tab3.trid" \
                            '^.*4.*4.*d1.*4.*4.*d2.*$'
  _RunSqlAndValidateOutput "select tab1.*, tab2.*, tab3.*
                            from db1.t1 tab1, db1.t2 tab2, db1.t3 tab3
                            where tab1.trid = tab2.trid and tab2.trid = tab3.trid" \
                            '^.*4.*4.*d1.*4.*4.*d2.*4.*4.*d2.*$'
  _RunSqlAndValidateOutput "select tab1.*, tab2.*, tab3.*
                            from db1.t1 tab1, db1.t2 tab2, db1.t3 tab3
                            where tab1.trid = tab2.trid and tab2.trid = tab3.trid and tab3.trid = 4" \
                            '^.*4.*4.*d1.*4.*4.*d2.*4.*4.*d2.*$'
fi

_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
