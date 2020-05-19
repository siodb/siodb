// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Boost headers
#include <boost/format.hpp>

namespace siodb::utils {

/**
 * Formats string according given format template and parameters.
 * @tparam Args Parameter types.
 * @param formatStr Format string.
 * @param args Actual parameters.
 * @returns Formatted string.
 */
template<class... Args>
std::string format(const std::string& formatStr, Args&&... args)
{
    return ((boost::format(formatStr)) % ... % std::forward<Args>(args)).str();
}

/**
 * Formats string according given format template and parameters.
 * @tparam Args Parameter types.
 * @param formatStr Format string.
 * @param args Actual parameters.
 * @returns Formatted string.
 */
template<class... Args>
std::string format(const char* formatStr, Args&&... args)
{
    return ((boost::format(formatStr)) % ... % std::forward<Args>(args)).str();
}

}  // namespace siodb::utils
