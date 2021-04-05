// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace {

/**
 * Termination signal handler. Must initiate correct database shutdown.
 * @param signal Signal number.
 */
void connWorkerTerminationSignalChainHandler(int signal);

}  // anonymous namespace
