// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DatabaseObjectName.h"

// CRT headers
#include <cctype>

namespace siodb::iomgr::dbengine {

namespace {

/** Allowed characters in database object name. */
const std::string g_allowedCharactersInObjectNames(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$");

}  // namespace

bool isValidDatabaseObjectName(const std::string& objectName) noexcept
{
    return !objectName.empty() && objectName.size() <= kMaxDatabaseObjectNameLength
           && !std::isdigit(objectName[0])
           && objectName.find_first_not_of(g_allowedCharactersInObjectNames) == std::string::npos;
}

}  // namespace siodb::iomgr::dbengine
