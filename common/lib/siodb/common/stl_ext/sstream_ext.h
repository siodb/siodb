// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <sstream>

namespace stdext {

/**
 * Concatenates arguments into a string.
 * @tpatam Args Argument types.
 * @param args Argument values.
 * @return String formed as concatenation of arguments converted to string.
 */
template<class... Args>
std::string concat(Args&&... args)
{
    std::ostringstream oss;
    (oss << ... << args);
    return oss.str();
}

/**
 * Concatenates arguments into a wide string.
 * @tpatam Args Argument types.
 * @param args Argument values.
 * @return String formed as concatenation of arguments converted to wide string.
 */
template<class... Args>
std::wstring wconcat(Args&&... args)
{
    std::wostringstream oss;
    (oss << ... << args);
    return oss.str();
}

}  // namespace stdext
