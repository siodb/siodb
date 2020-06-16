// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

namespace siodb::iomgr::dbengine {

/** Maximum allowed database object name length */
constexpr std::size_t kMaxDatabaseObjectNameLength = 255;

/**
 * Validates database object name.
 * Database object name must be at most 255 characters long, and allows only
 * ASCII letters, digits and underscore. Digit cannot be first character.
 * @param objectName Object name to validate.
 * @return true if object name is valid, false otherwise.
 */
bool isValidDatabaseObjectName(const std::string& objectName) noexcept;

}  // namespace siodb::iomgr::dbengine
