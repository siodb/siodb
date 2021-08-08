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
    kSelectSystem = 1,
    kInsert = 2,
    kDelete = 3,
    kUpdate = 4,
    kShow = 5,
    kShowSystem = 6,
    kCreate = 7,
    kDrop = 8,
    kAlter = 9,
    kAttach = 10,
    kDetach = 11,
    kEnable = 12,
    kDisable = 13,
    kShutdown = 14
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

/** SELECT FROM SYATEM TABLE permission bitmask */
constexpr auto kSelectSystemPermissionMask = getSinglePermissionMask(PermissionType::kSelectSystem);

/** INSERT permission bitmask */
constexpr auto kInsertPermissionMask = getSinglePermissionMask(PermissionType::kInsert);

/** DELETE permission bitmask */
constexpr auto kDeletePermissionMask = getSinglePermissionMask(PermissionType::kDelete);

/** UPDATE permission bitmask */
constexpr auto kUpdatePermissionMask = getSinglePermissionMask(PermissionType::kUpdate);

/** SHOW permission bitmask */
constexpr auto kShowPermissionMask = getSinglePermissionMask(PermissionType::kShow);

/** SHOW SYSTEM permission bitmask */
constexpr auto kShowSystemPermissionMask = getSinglePermissionMask(PermissionType::kShowSystem);

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
