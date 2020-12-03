// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "system_error_wrapper.h"

#if __cplusplus <= 201703L  // C++17 and below
#include <boost/filesystem.hpp>
using system_error_code = boost::system::error_code;
namespace fs = boost::filesystem;
#define SIODB_USES_BOOST_FILESYSTEM
#else  // C++20 and above
#include <filesystem>
using system_error_code = std::error_code;
namespace fs = std::filesystem;
#endif  // __cplusplus <= 201703L
