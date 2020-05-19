// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

#ifdef _DEBUG

// Project headers
#include "../sys/Syscalls.h"

// STL headers
#include <iostream>
#include <sstream>

// System headers
#include <unistd.h>

#define DEBUG_SYSCALLS_LIBRARY_GUARD SyscallsLibraryGuard debugSyscallsLibraryGuard

#define DEBUG_TRACE(x) std::cerr << ::getpid() << ' ' << ::gettid() << " >>> " << x << std::endl

#else  // _DEBUG

#define DEBUG_SYSCALLS_LIBRARY_GUARD \
    do {                             \
    } while (false)

#define DEBUG_TRACE(x) \
    do {               \
    } while (false)

#endif  // _DEBUG

#define VOID_PTR(p) ((void*) (p))
