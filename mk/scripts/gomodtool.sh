#!/usr/bin/env bash

set -e

if [[ $# -lt 3 ]]; then
    echo "Missing required parameters" >&2
    echo "Usage: $0 MODULE_NAME MODULE_PATH GO_EXECUTABLE" >&2
    exit 1
fi

_go=$(realpath "$3")
_mod_path=$(realpath "$2")
_mod_dir=$(dirname "${_mod_path}")
if [[ ! -d "${_mod_dir}" ]]; then
    mkdir -p "${_mod_dir}"
fi

cd "${_mod_dir}"
if [[ ! -f ${_mod_path} ]]; then
    "${_go}" mod init "$1"
fi
"${_go}" mod tidy
touch "${_mod_path}"
