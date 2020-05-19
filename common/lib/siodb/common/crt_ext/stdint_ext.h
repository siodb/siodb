// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)  // GCC or Clang

/** 128-bit signed integer type */
typedef __int128_t int128_t;

/** 128-bit unsigned integer type */
typedef __uint128_t uint128_t;

#else  // Other compilers

#ifndef SIODB_COMMON_STL_EXT_CSTDINT_EXT_H__
#error 128-bit integers in a C code are not natively supported by this compiler.
#endif  // SIODB_COMMON_STL_EXT_CSTDINT_EXT_H__

#endif  // Compiler type
