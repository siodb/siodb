// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/**
 * Permission types.
 * See ISO/IEC 9075-1:2016(E) 4.6.11 Privileges.
 * There may be few our own additional permission constants.
 */
enum class PermissionType : std::uint64_t {
    kSelect = 0x1,
    kInsert = 0x2,
    kUpdate = 0x4,
    kDelete = 0x8,
    kReferences = 0x10,
    kUsage = 0x20,
    kUnder = 0x40,
    kTrigger = 0x80,
    kExecute = 0x100,
    kCreate = 0x200,
    kAlter = 0x400,
    kDrop = 0x800,
    kShow = 0x1000,
};

/**
 * Tests if permission is included into a set of permissions.
 * @param permissions Set of permissions.
 * @param permission Permission to check.
 * @return true if permission set includes permission, false otherwise.
 */
inline bool hasPermission(const std::uint64_t permissions, const PermissionType permission) noexcept
{
    return (permissions & static_cast<std::uint64_t>(permission)) != 0;
}

/**
 * Includes permission into a set of permissions.
 * @param permissions Set of permissions.
 * @param permission Permission to check.
 */
inline void enablePermission(std::uint64_t& permissions, const PermissionType permission) noexcept
{
    permissions |= static_cast<std::uint64_t>(permission);
}

/**
 * Excludes permission into a set of permissions.
 * @param permissions Set of permissions.
 * @param permission Permission to check.
 */
inline void disablePermission(std::uint64_t& permissions, const PermissionType permission) noexcept
{
    permissions &= ~static_cast<std::uint64_t>(permission);
}

}  // namespace siodb::iomgr::dbengine
