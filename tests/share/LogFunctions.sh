# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

function _log {
    if [[ -z ${TEST_NAME} ]]; then TEST_NAME="UNDEFINED"; fi
    echo "## `date "+%Y-%m-%dT%H:%M:%S"` | ${TEST_NAME} | $1 | $2"
}
