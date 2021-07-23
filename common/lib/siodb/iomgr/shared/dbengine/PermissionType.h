// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/** Permission types and respective bitmask bit numbers. */
enum class PermissionType {
    kSelect = 0,
    kInsert = 1,
    kUpdate = 2,
    kDelete = 3,
    kShow = 4,
    kCreate = 6,
    kDrop = 7,
    kAlter = 8,
    kAttach = 9,
    kDetach = 10,
    kEnable = 11,
    kDisable = 12
};

/**
 * Returns permission mask from a single permission type.
 * @param permissions A permission type.
 * @return Corresponding permission bitmask.
 */
constexpr inline std::uint64_t getSinglePermissionMask(PermissionType permissionType)
{
    return static_cast<std::uint64_t>(1U) << static_cast<int>(permissionType);
}

/**
 * Builds permission mask for the multiple permission types.
 * @tparam permissionTypes List of permission types.
 * @return Corresponding permission bitmask.
 */
template<PermissionType... permissionTypes>
constexpr inline std::uint64_t buildMultiPermissionMask()
{
    return (... | getSinglePermissionMask(permissionTypes));
}

}  // namespace siodb::iomgr::dbengine
