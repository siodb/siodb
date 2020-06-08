// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Internal headers
#include "detail/string_ext_detail.h"

// CRT Headers
#include <errno.h>
#include <string.h>

char* _i128toa(const int128_t* value, char* str, int base)
{
    if (!value || !str || base < 2 || base > 36) {
        errno = EINVAL;
        return 0;
    }
    char buffer[129];
    return strcpy(str, _i128toa_impl(value, buffer, sizeof(buffer), base, false, false));
}

char* _u128toa(const uint128_t* value, char* str, int base)
{
    if (!value || !str || base < 2 || base > 36) {
        errno = EINVAL;
        return 0;
    }
    char buffer[129];
    return strcpy(str, _u128toa_impl(value, buffer, sizeof(buffer), base, false, false));
}

static const char g_lowercaseDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char g_uppercaseDigits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

char* _i128toa_impl(const int128_t* value, char* buffer, size_t bufferSize, int base,
        _Bool uppercase, _Bool sign)
{
    const _Bool base10 = base == 10;
    const _Bool neg = base10 && *value < 0;
    const char* const digits = uppercase ? g_uppercaseDigits : g_lowercaseDigits;
    int128_t v = (neg) ? *value : -*value;
    char* p = buffer + bufferSize - 1;
    *p = '\0';
    do {
        *--p = digits[v % base];
        v /= base;
    } while (v != 0);
    if (neg || (base10 && sign)) *--p = neg ? '-' : '+';
    return p;
}

char* _u128toa_impl(const uint128_t* value, char* buffer, size_t bufferSize, int base,
        _Bool uppercase, _Bool sign)
{
    const char* const digits = uppercase ? g_uppercaseDigits : g_lowercaseDigits;
    uint128_t v = *value;
    char* p = buffer + bufferSize - 1;
    *p = '\0';
    do {
        *--p = digits[v % base];
        v /= base;
    } while (v != 0);
    if (base == 10 && sign) *p-- = '+';
    return p;
}
