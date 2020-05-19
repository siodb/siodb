// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

#if defined(_MSC_VER)

#define SIODB_ALWAYS_INLINE __forceinline
#define SIODB_LIKELY(cond) cond
#define SIODB_UNLIKELY(cond) cond

#elif defined(__GNUC__)

#define SIODB_ALWAYS_INLINE __attribute__((always_inline))
#define SIODB_LIKELY(cond) __builtin_expect((cond), 1)
#define SIODB_UNLIKELY(cond) __builtin_expect((cond), 0)

#elif defined(__clang__)

#define SIODB_ALWAYS_INLINE __attribute__((always_inline))
#define SIODB_LIKELY(cond) __builtin_expect((cond), 1)
#define SIODB_UNLIKELY(cond) __builtin_expect((cond), 0)

#else  // Other compilers

#define SIODB_ALWAYS_INLINE
#define SIODB_LIKELY(cond) cond
#define SIODB_UNLIKELY(cond) cond

#endif  // Compiler type
