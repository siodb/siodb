// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

namespace siodb::utils {

#ifndef SIODB_COMMON_UTILS_EMPTYSTRING_CPP__

/** Empty string constant object. */
extern const std::string g_emptyString;

#endif  // SIODB_COMMON_UTILS_EMPTYSTRING_CPP__

/**
 * Returns empty string.
 * @return Empty string object.
 */
const std::string& getEmptyString() noexcept;

}  // namespace siodb::utils
