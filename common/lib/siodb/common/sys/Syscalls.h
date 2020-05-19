// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// System headers
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/** Initializes syscalls library */
void initSyscalls(void);

/** Finalizes syscalls library */
void finalizeSyscalls(void);

/** Returns current kernel thread ID */
pid_t gettid(void);

#ifdef __cplusplus
}  // extern "C"

/** Syscalls library initialization guard for C++ */
struct SyscallsLibraryGuard {
    SyscallsLibraryGuard() noexcept
    {
        initSyscalls();
    }

    ~SyscallsLibraryGuard()
    {
        finalizeSyscalls();
    }
};

#endif  // __cplusplus
