// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Required for stdint_ext.h
#define SIODB_COMMON_STL_EXT_CSTDINT_EXT_H__

// Project headers
#include "../crt_ext/stdint_ext.h"

namespace stdext {

#if defined(__GNUC__) || defined(__clang__)

typedef ::int128_t int128_t;
typedef ::uint128_t uint128_t;

#else
#error 128-bit integers in a C++ code are not supported for this compiler.
#endif  // Compiler type

}  // namespace stdext
