#!/usr/bin/env bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

CLANG_FORMAT=clang-format

STYLE=$(git config --get hooks.clangformat.style)
if [[ -n "${STYLE}" ]]; then
    STYLEARG="-style=${STYLE}"
else
    STYLEARG=""
fi

format_file()
{
    local fpath="$1"
    local fext="${fpath##*\.}"
    if [[ -f "${fpath}" ]] && ( \
            [[ ${fext} == "c" ]] \
            || [[ ${fext} == "cpp" ]] \
            || [[ ${fext} == "h" ]] \
        );
    then
        echo "Formatting ${fpath}"
        ${CLANG_FORMAT} -i ${STYLEARG} "$1"
        git add "$1"
    fi
}

case "$1" in
    --about)
        echo "Runs clang-format on source files"
        ;;
    *)
        for file in `git diff-index --cached --name-only HEAD` ; do
            format_file "${file}"
        done
        ;;
esac
