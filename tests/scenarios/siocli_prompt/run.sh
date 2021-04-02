#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
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

## =============================================
## TEST FOOTER
## =============================================
_FinalStopOfSiodb
_CheckLogFiles
_TestEnd
exit 0
