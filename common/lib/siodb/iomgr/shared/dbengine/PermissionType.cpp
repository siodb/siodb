// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "PermissionType.h"

// Common project headers
#include <siodb/common/crt_ext/compiler_defs.h>

// STL headers
#include <stdexcept>

namespace siodb::iomgr::dbengine {

namespace {

const char* g_permissionTypeNames[] = {
        "Select",
        "SelectSystem",
        "Insert",
        "Delete",
        "Update",
        "Show",
        "ShowSystem",
        "Create",
        "Drop",
        "Alter",
        "Attach",
        "Detach",
        "Enable",
        "Disable",
        "Shutdown",
        "ShowPermissions",
};

}  // anonymous namespace

const char* getPermissionTypeName(int permissionType)
{
    if (SIODB_LIKELY(
                permissionType >= 0 || permissionType < static_cast<int>(PermissionType::kMax))) {
        return g_permissionTypeNames[permissionType];
    }
    throw std::invalid_argument("Invalid permission type");
}

}  // namespace siodb::iomgr::dbengine
