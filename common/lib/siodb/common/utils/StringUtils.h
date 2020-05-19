// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// STL headers
#include <string>

namespace siodb::utils {

/**
 * Creates new string of a given length filled with a given character.
 * @param length String length.
 * @param fill Fill character.
 * @return A string.
 */
std::string createString(std::size_t length, char fill = 'x');

}  // namespace siodb::utils
