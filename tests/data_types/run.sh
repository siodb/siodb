#!/bin/bash

# Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/../share/LogFunctions.sh"

for d in $(ls -d "${SCRIPT_DIR}/"*/); do
    _log "INFO" "Running test $d"
    ${d}/run.sh "$@"
    if [[ $? -ne  0 ]]; then
        _log "TEST:ERROR" "Tests failed for $d"
        if [[ "${SIODB_TEST_ALL}" != "yes" ]]; then
            exit 1
        fi
    else
        _log "TEST:SUCCESS" "Tests succeed in $d"
    fi
done
