#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

if [ $# -lt 1 ]; then
    echo "Usage: format.sh TOP_LEVEL_DIR"
    exit 1
fi

CLANG_FORMAT=/usr/lib/llvm-8/bin/clang-format

find "$1" -iname *.h -o -iname *.cpp -o -iname *.c | xargs ${CLANG_FORMAT} -i
exit $?
