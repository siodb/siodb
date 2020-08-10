// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include "../crt_ext/stddef_ext.h"

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

#define DEBUG_DECL(t, v) x

#define DEBUG_DECL_LOCAL(x) x

#else

#define DEBUG_SYSCALLS_LIBRARY_GUARD \
    do {                             \
    } while (false)

#define DEBUG_TRACE(x) \
    do {               \
    } while (false)

#define DEBUG_DECL_LOCAL(x) \
    do {                    \
    } while (false)

#endif  // _DEBUG
