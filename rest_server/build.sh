#!/bin/bash

set -e
set -x

if [ -z "${1}" ]; then
  MODE="DEBUG"
else
  MODE="${1}"
fi

if [ -z "${2}" ]; then
  OUTPUT_DIRECTORY_PATH="/tmp"
else
  OUTPUT_DIRECTORY_PATH="${2}"
fi

if [ -z "${3}" ]; then
  GO_BIN_DIR="/usr/local/go/bin"
else
  GO_BIN_DIR="${3}"
fi

case ${MODE} in
  "DEBUG")
    /usr/local/go/bin/go build -o ${OUTPUT_DIRECTORY_PATH}/siodb_rest_server -gcflags="all=-N -l -dwarf -dwarflocationlists"
    ;;
  "RELEASE")
    /usr/local/go/bin/go build -o ${OUTPUT_DIRECTORY_PATH}/siodb_rest_server -ldflags "-s -w"
    ;;
  *)
    echo "Unknown build mode. Please use 'DEBUG' or 'RELEASE'."
    ;;
esac


