// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

/**
 * Helper macro to suppress compiler warning on unused parameters
 * where they cannot be avoided in the C code.
 */
#define UNUSED(x) (void) (x)

/**
 * Helper macro to suppress compiler warning on unused parameters
 * when they are supposed to be temporarily unused.
 */
#define TEMPORARY_UNUSED(x) UNUSED(x)

/** Helper macro to count C array elements */
#ifdef _MSC_VER
#define COUNTOF(x) _countof(x)
#else
#define COUNTOF(x) (sizeof(x) / sizeof((x)[0]))
#endif

/** Helper macro to define non-copyable classes */
#define DECLARE_NONCOPYABLE(ClassName)    \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete
