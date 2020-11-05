#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

SCRIPT_DIR=$(dirname $(realpath "$0"))

for d in $(ls -d "${SCRIPT_DIR}/"*/); do
    echo "## `date "+%Y-%m-%dT%H:%M:%S"` | INFO | Running scenario $d"
    ${d}/run.sh "$@"
    if [[ $? -ne  0 ]]; then
        echo "## `date "+%Y-%m-%dT%H:%M:%S"` | TEST:ERROR | Tests in scenario $d failed"
        if [[ "${SIODB_TEST_ALL}" != "yes" ]]; then
            exit 1
        fi
    else
        echo "## `date "+%Y-%m-%dT%H:%M:%S"` | TEST:SUCCESS | Tests in scenario $d succeed"
    fi
done