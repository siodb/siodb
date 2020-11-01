#!/bin/bash

# Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
# Use of this source code is governed by a license that can be found
# in the LICENSE file.

./rest/run.sh "$*"
./sql/run.sh "$*"
./login_with_keys/run.sh "$*"
./users/run.sh "$*"
./export/run.sh "$*"
./issues/run.sh "$*"
