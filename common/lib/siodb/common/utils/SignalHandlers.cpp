// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SignalHandlers.h"

// Internal headers
#include "detail/SignalHandlersDetail.h"

// Project headers
#include "HelperMacros.h"

// CRT headers
#include <cstring>

// System headers
#include <ucontext.h>

namespace siodb::utils {

namespace {

int g_exitSignalNumber;
sighandler_t g_chainedHandler;
siginfo_t g_exitSignalInfo;
ucontext_t g_exitSignalUContext;
stdext::event g_exitEvent;

}  // namespace

void setupSignalHandlers(sighandler_t chainedHandler)
{
    g_chainedHandler = chainedHandler;

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));

    // We need this one to stop connection listener
    // and connection handler threads. The main effect is that blocking I/O
    // system calls are interrupted.
    sa.sa_handler = &noActionSignalHandler;
    sigaction(SIGUSR1, &sa, nullptr);

    const int signals[] = {SIGHUP, SIGINT, SIGTERM};
    for (int signal : signals) {
        sigaddset(&sa.sa_mask, signal);
    }

    // https://serverfault.com/questions/94956/how-to-find-out-the-source-of-a-posix-signal
    sa.sa_handler = nullptr;
    sa.sa_sigaction = &terminationSignalHandler;
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
    return g_exitEvent.signaled();
}

int getExitSignalNumber() noexcept
{
    return g_exitSignalNumber;
}

siginfo_t getExitSignalInfo() noexcept
{
    return g_exitSignalInfo;
}

pid_t getExitSignalSenderPid() noexcept
{
    const auto sigCode = g_exitSignalInfo.si_code & 0xFF;
    return (sigCode == SI_USER || sigCode == SI_QUEUE) ? g_exitSignalInfo.si_pid : -1;
}

namespace {

void noActionSignalHandler([[maybe_unused]] int signal)
{
    // Intentionally don't do anything here
}

void terminationSignalHandler(int signal, siginfo_t* info, void* ucontext)
{
    g_exitSignalNumber = signal;
    g_exitSignalInfo = *info;
    g_exitSignalUContext = *reinterpret_cast<ucontext_t*>(ucontext);
    g_exitEvent.notify_one();
    if (g_chainedHandler) g_chainedHandler(signal);
}

}  // anonymous namespace

}  // namespace siodb::utils
