// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstddef>

/**
 * Calculates length of a constexpr null-terminated string at compile time.
 * Best suitable for use with short constrexpr strings.
 * @param s null-terminated string
 * @return string length
 * @see https://stackoverflow.com/a/36390498/1540501
 */
template<typename CharT>
constexpr std::size_t ct_strlen(const CharT* s) noexcept
{
    return *s ? (ct_strlen(s + 1) + 1) : 0;
}
