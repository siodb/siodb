// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#define SIODB_COMMON_UTILS_EMPTYSTRING_CPP__

#include "EmptyString.h"

namespace siodb::utils {

// Declare non-const to avoid elimination
std::string g_emptyString;

const std::string& getEmptyString() noexcept
{
    return g_emptyString;
}

}  // namespace siodb::utils
