// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../stl_ext/event.h"

// System headers
#include <signal.h>

namespace siodb::utils {

/**
 * Sets up handlers for important signals.
 * @param chainedHandler Additional signal handler to chain with standard one.
 */
void setupSignalHandlers(sighandler_t chainedHandler = nullptr);

/** Waits for exit event */
void waitForExitEvent();

/**
 * Retuns exit event status.
 * @return exit event status.
 */
bool isExitEventSignaled();

/**
 * Returns exit signal number that have triggered exit event.
 * @return exit signal number.
 */
int getExitSignal() noexcept;

}  // namespace siodb::utils
