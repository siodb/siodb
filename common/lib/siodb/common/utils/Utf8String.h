// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

namespace siodb::utils {

/**
 * Compares two UTF-8 strings.
 * @param s1 First string.
 * @param s1Len First string length.
 * @param s2 Second string.
 * @param s2Len Secon string length.
 * @return -1 if @ref s1 lexicographically less than @ref s2.
 *         0 if @ref s1 nd @ref s2 are equal.
 *         1 if @ref s1 lexicographically greater than @ref s2.
 */
int utf8_strcmp(const char* s1, std::size_t s1Len, const char* s2, std::size_t s2Len);

/**
 * Compares two UTF-8 strings.
 * @param s1 First string.
 * @param s2 Second string.
 * @return -1 if @ref s1 lexicographically less than @ref s2.
 *         0 if @ref s1 nd @ref s2 are equal.
 *         1 if @ref s1 lexicographically greater than @ref s2.
 */
inline int utf8_strcmp(const std::string& s1, const std::string& s2)
{
    return utf8_strcmp(s1.c_str(), s1.size(), s2.c_str(), s2.size());
}

}  // namespace siodb::utils
