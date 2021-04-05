// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// System headers
#include <signal.h>

namespace siodb::utils {
namespace {

/**
 * No action signal handler. Just does nothing on signal.
 * @param signal Signal number.
 */
void noActionSignalHandler(int signal);

/**
 * Termination signal handler. Must initiate correct database shutdown.
 * @param signal Signal number.
 * @param info Detailed information about the signal.
 * @param uncontext Signal context information saved by kernel (ucontext_t).
 */
void terminationSignalHandler(int signal, siginfo_t* info, void* ucontext);

}  // anonymous namespace
}  // namespace siodb::utils
