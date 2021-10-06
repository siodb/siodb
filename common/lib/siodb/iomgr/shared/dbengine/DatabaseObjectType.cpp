// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DatabaseObjectType.h"

// Common project headers
#include <siodb/common/crt_ext/compiler_defs.h>

// STL headers
#include <stdexcept>

namespace siodb::iomgr::dbengine {

namespace {

const char* g_databaseObjectTypeNames[] = {
        "Instance",
        "Database",
        "Table",
        "Column",
        "Index",
        "Constraint",
        "Trigger",
        "Procedure",
        "Function",
        "User",
        "UserAccessKey",
        "UserToken",
};

}  // anonymous namespace

const char* getDatabaseObjectTypeName(int objectType)
{
    if (SIODB_LIKELY(objectType >= 0 || objectType < static_cast<int>(DatabaseObjectType::kMax))) {
        return g_databaseObjectTypeNames[objectType];
    }
    throw std::invalid_argument("Invalid database object type");
}

}  // namespace siodb::iomgr::dbengine
