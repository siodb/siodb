#!/bin/bash

# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

## Global
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME=$(basename "${SCRIPT_DIR}")
source "${SCRIPT_DIR}/../../share/CommonFunctions.sh"

## Specific test functions

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

_RunSql "       -- QUERY 1
   -- just a comment --
        select
         -- just a comment
     name  -- just a comment --
                           --
                  from /* just a comment line 1
             just a comment line 2
             just a comment line 3 */
                     sys_databases
           where --
         trid = 1
         --
         /*
             just a comment line 1
             */   ;
   -- just a comment --
        select
         -- just a comment
     name  -- just a comment --
                           --
                  from /* just a comment line 1
             just a comment line 2
             just a comment line 3 */
                     sys_databases
           where --
         trid = 1
         --
         /*
             just a comment line 1
             */   ;       select name from sys_databases where trid = 1;    "


_RunSqlAndValidateOutput "       -- QUERY 2
       select name from sys_databases where trid = 1;    " "^SYS$"

_RunSqlAndValidateOutput "       -- QUERY 3
  select       --
'

---
---
' from sys_databases where trid = 1;    " "^[\]n[\]n---[\]n---[\]n$"

_RunSqlAndValidateOutput "       -- QUERY 4
       select     '
--
--
' from
  --
sys_databases
  /* commment */ where trid = 1;    " "^[\]n--[\]n--[\]n$"


line_of_spaces='   '
_RunSqlAndValidateOutput "${line_of_spaces}
       -- QUERY 5
${line_of_spaces}
       select     '
${line_of_spaces}
--
${line_of_spaces}
--
${line_of_spaces}
' from
${line_of_spaces}
  --
${line_of_spaces}
sys_databases
${line_of_spaces}
  /* commment */ where trid = 1;    " "^[\]n   [\]n--[\]n   [\]n--[\]n   [\]n$"


_RunSqlAndValidateOutput "/*  -- QUERY 5
    this is a multiline comment
  -- this is attempt to use comment in comment, but this is in fact part of previously started
                       -- multiline comment which ends right here*/
      select
                 name
              /* I start multline comment /* multiple /* times
${line_of_spaces}
                 but that /* should /* not /*
${line_of_spaces}
                 matter
              */
${line_of_spaces}
from
-- this is single line comments  /* but NOT a multiline comment despite /*
sys_databases
${line_of_spaces}
-- this is single line comments  /* but NOT a multiline comment despite /*
${line_of_spaces}
/*
    this is a multiline comment
  -- this is attempt to use comment in comment, but this is in fact part of previously started
-- multiline comment which ends right here*/ where trid = 1
;${line_of_spaces}
" "^SYS$"


_RunSql "        -- MULTI LINE COMMENT ONLY 1
         /*${line_of_spaces}
 aze

azeaze
${line_of_spaces}
${line_of_spaces}
                          */${line_of_spaces}
"

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
