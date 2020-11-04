#!/bin/bash

SCRIPT_DIR=$(dirname "$0")

for d in $(ls -d "${SCRIPT_DIR}/gh-"*/); do
    echo "Running test for the issue $d"
    ${d}/run.sh "$*"
    if [[ $? -ne  0 ]]; then
        echo "Test for the issue ${d} failed" >&2
        exit 1
    fi
done
