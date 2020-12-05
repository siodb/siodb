// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

#if __cplusplus <= 201703L  // C++17 and below
#include <boost/system/system_error.hpp>
using system_error_code = boost::system::error_code;
#else  // C++20 and above
#include <system_error>
using system_error_code = std::error_code;
namespace fs = std::filesystem;
#endif  // __cplusplus <= 201703L
