// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SignalHandlers.h"

// Project headers
#include "HelperMacros.h"
#include "internal/SignalHandlersInternal.h"

// CRT headers
#include <cstring>

namespace siodb::utils {

namespace {

sighandler_t g_chainedHandler;
int g_exitSignal;
WaitableEvent g_exitEvent;

}  // namespace

void setupSignalHandlers(sighandler_t chainedHandler)
{
    g_chainedHandler = chainedHandler;

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));

    // We need this one to stop connection listener
    // and connection handler threads. The main effect is that blocking
    // I/O system calls are interrupted.
    sa.sa_handler = &noActionSignalHandler;
    sigaction(SIGUSR1, &sa, nullptr);

    const int signals[] = {SIGHUP, SIGINT, SIGTERM};
    for (int signal : signals) {
        sigaddset(&sa.sa_mask, signal);
    }
    sa.sa_handler = &terminationSignalHandler;
    for (int signal : signals) {
        sigaction(signal, &sa, nullptr);
    }

    signal(SIGPIPE, SIG_IGN);
}

void waitForExitEvent()
{
    g_exitEvent.wait();
}

bool isExitEventSignaled()
{
    return g_exitEvent.isSignaled();
}

int getExitSignal() noexcept
{
    return g_exitSignal;
}

namespace {

void terminationSignalHandler(int signal)
{
    g_exitSignal = signal;
    g_exitEvent.signal();
    if (g_chainedHandler) {
        g_chainedHandler(signal);
    }
}

void noActionSignalHandler([[maybe_unused]] int signal)
{
    // Intentionally do not do anything here
}

}  // anonymous namespace

}  // namespace siodb::utils
