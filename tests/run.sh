#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

# --------------------------------------------------------------
# External Parameters
# --------------------------------------------------------------
if [[ -z "${SIODB_BIN}" ]]; then
    SIODB_BIN="../build/debug/bin"
fi

# --------------------------------------------------------------
# Scenarios
# --------------------------------------------------------------
./rest/run.sh
./sql/run.sh
./users/run.sh
./data/run.sh
