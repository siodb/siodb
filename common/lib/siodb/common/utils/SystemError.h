// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers

// STL headers
#include <string>
#include <system_error>

namespace siodb::utils {

/**
 * Formats error string using given prefix and error code.
 * @param errorCode An error code.
 * @param prefix Prefix string.
 * @return Error string.
 */
std::string formatErrorMessage(int errorCode, const char* prefix);

/**
 * Formats error string using given prefix and error code.
 * @param errorCode An error code.
 * @param prefix Prefix string.
 * @return Error string.
 */
inline std::string formatErrorMessage(int errorCode, const std::string& prefix)
{
    return formatErrorMessage(errorCode, prefix.c_str());
}

/**
 * Throws std::system_error with given error code and action description.
 * @param errorCode An error code.
 * @param description Action description.
 */
[[noreturn]] inline void throwSystemError(int errorCode, const char* description)
{
    throw std::system_error(errorCode, std::generic_category(), description);
}

/**
 * Throws std::system_error with current errno and action description.
 * @param description Action description.
 */
[[noreturn]] inline void throwSystemError(const char* description)
{
    throwSystemError(errno, description);
}

/**
 * Throws std::system_error with given error code and action description.
 * @param errorCode An error code.
 * @param description Action description.
 */
[[noreturn]] inline void throwSystemError(int errorCode, const std::string& description)
{
    throwSystemError(errorCode, description.c_str());
}

/**
 * Throws std::system_error with current errno and action description.
 * @param description Action description.
 */
[[noreturn]] inline void throwSystemError(const std::string& description)
{
    throwSystemError(errno, description);
}

/**
 * Throws std::system_error with given error code and action description.
 * @param description Action description.
 * @param arg1 Action description argument.
 */
[[noreturn]] void throwSystemError(int errorCode, const char* description, const char* arg1);

/**
 * Throws std::system_error with current errno and action description.
 * @param description Action description.
 * @param arg1 Action description argument.
 */
[[noreturn]] inline void throwSystemError(const char* description, const char* arg1)
{
    throwSystemError(errno, description, arg1);
}

}  // namespace siodb::utils
