// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../string_ext.h"

// CRT headers
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Converts 128-bit signed integer to string.
 * @param value Value to be converted to a string.
 * @param buffer Buffer to store the resulting null-terminated string.
 * @param bufferSize Buffer size. Must be long enough to store expected result.
 * @param base Numerical base used to represent the value as a string, between 2 and 3.
 * @return Pointer to the first character of a resulting string.
 */
char* _i128toa_impl(const int128_t* value, char* buffer, size_t bufferSize, int base,
        _Bool uppercase, _Bool sign);

/**
 * Converts 128-bit unsigned integer to string.
 * @param value Value to be converted to a string.
 * @param buffer Buffer to store the resulting null-terminated string.
 * @param bufferSize Buffer size. Must be long enough to store expected result.
 * @param base Numerical base used to represent the value as a string, between 2 and 3.
 * @return Pointer to the first character of a resulting string.
 */
char* _u128toa_impl(const uint128_t* value, char* buffer, size_t bufferSize, int base,
        _Bool uppercase, _Bool sign);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
