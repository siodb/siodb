#!/bin/bash

SCRIPT_DIR=$(dirname "$0")

for d in $(ls -d "${SCRIPT_DIR}/gh-"*/); do
    echo "Running $d"
    ${d}/run.sh $*
done
