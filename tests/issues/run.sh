#!/bin/bash

SCRIPT_DIR=$(dirname "$0")

for d in $(ls -d "${SCRIPT_DIR}/"*/); do
    ${d}/run.sh "$*"
done
