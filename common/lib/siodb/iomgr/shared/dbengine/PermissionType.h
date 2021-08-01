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
    kDelete = 2,
    kUpdate = 3,
    kShow = 4,
    kCreate = 6,
    kDrop = 7,
    kAlter = 8,
    kAttach = 9,
    kDetach = 10,
    kEnable = 11,
    kDisable = 12,
    kShutdown = 13
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

/**
 * Removes one permissions from the permission mask.
 * @param permissions Permission bitmask to modify.
 * @param permissions A permission type to remove.
 * @return Corresponding permission bitmask.
 */
constexpr inline std::uint64_t removeSinglePermissionFromMask(
        std::uint64_t permissions, PermissionType permissionType)
{
    return permissions & ~getSinglePermissionMask(permissionType);
}

/**
 * Removes one or more permissions from the permission mask.
 * @tparam permissionTypes List of permission types.
 * @param permissions Permission bitmask to modify.
 * @return Corresponding permission bitmask.
 */
template<PermissionType... permissionTypes>
constexpr inline std::uint64_t removeMultiplePermissionsFromMask(std::uint64_t permissions)
{
    return permissions & ~buildMultiPermissionMask<permissionTypes...>();
}

/** SELECT permission bitmask */
constexpr auto kSelectPermissionMask = getSinglePermissionMask(PermissionType::kSelect);

/** INSERT permission bitmask */
constexpr auto kInsertPermissionMask = getSinglePermissionMask(PermissionType::kInsert);

/** DELETE permission bitmask */
constexpr auto kDeletePermissionMask = getSinglePermissionMask(PermissionType::kDelete);

/** UPDATE permission bitmask */
constexpr auto kUpdatePermissionMask = getSinglePermissionMask(PermissionType::kUpdate);

/** SHOW permission bitmask */
constexpr auto kShowPermissionMask = getSinglePermissionMask(PermissionType::kShow);

/** CREATE permission bitmask */
constexpr auto kCreatePermissionMask = getSinglePermissionMask(PermissionType::kCreate);

/** DROP permission bitmask */
constexpr auto kDropPermissionMask = getSinglePermissionMask(PermissionType::kDrop);

/** ALTER permission bitmask */
constexpr auto kAlterPermissionMask = getSinglePermissionMask(PermissionType::kAlter);

/** ATTACH permission bitmask */
constexpr auto kAttachPermissionMask = getSinglePermissionMask(PermissionType::kAttach);

/** DETACH permission bitmask */
constexpr auto kDetachPermissionMask = getSinglePermissionMask(PermissionType::kDetach);

}  // namespace siodb::iomgr::dbengine
