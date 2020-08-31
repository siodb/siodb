// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <sstream>

// Boost headers
#include <boost/system/error_code.hpp>

namespace siodb::utils {

/**
 * Constructs path from given components.
 * @param dir Directory.
 * @param extra Extra components.
 * @return Constructed path
 */
template<typename... Args>
std::string constructPath(const std::string& dir, Args... extra)
{
    std::ostringstream str;
    str << dir << '/';
    (str << ... << extra);
    return str.str();
}

/**
 * Removes content of the directory but doesn't delete directory itself.
 * @param dir Directory.
 * @throws boost::system_error if error happened.
 */
void clearDir(const std::string& path);

/**
 * Removes content of the directory but doesn't delete directory itself.
 * @param dir Directory.
 * @param[out] errorCode Error code.
 * @return true if operation succeeded, false otherwise.
 */
bool clearDir(const std::string& path, boost::system::error_code& errorCode);

}  // namespace siodb::utils
