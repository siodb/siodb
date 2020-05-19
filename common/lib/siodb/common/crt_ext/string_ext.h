// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "stdint_ext.h"

// CRT Headers
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Converts 128-bit signed integer to string.
 * @param value Value to be converted to a string.
 * @param str Array in memory where to store the resulting null-terminated string.
 * @param base Numerical base used to represent the value as a string, between 2 and 3.
 * @return str on success, NULL on error, and sets errno to an error code.
 */
char* _i128toa(const int128_t* value, char* str, int base);

/**
 * Converts 128-bit unsigned integer to string.
 * @param value Value to be converted to a string.
 * @param str Array in memory where to store the resulting null-terminated string.
 * @param base Numerical base used to represent the value as a string, between 2 and 3.
 * @return str on success, NULL on error, and sets errno to an error code.
 */
char* _u128toa(const uint128_t* value, char* str, int base);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
