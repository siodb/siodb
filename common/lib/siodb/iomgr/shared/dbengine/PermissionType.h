// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>

namespace siodb::iomgr::dbengine {

/** Permission types and respective bitmask bit numbers. */
enum class PermissionType {
    /**
     * SELECT permission.
     * Applicable to: Table, View
     */
    kSelect = 0,

    /**
     * SELECT FROM SYSTEM OBJECT permission.
     * Applicable to: Table, View
     */
    kSelectSystem = 1,

    /**
     * INSERT permission.
     * Applicable to: Table
     */
    kInsert = 2,

    /**
     * DELETE permission.
     * Applicable to: Table
     */
    kDelete = 3,

    /**
     * UPDATE permission.
     * Applicable to: Table
     */
    kUpdate = 4,

    /**
     * SHOW permission.
     * Applicable to: Database, Table, View, Index, Trigger, Procedure, Function, User
     */
    kShow = 5,

    /**
     * SHOW system object permission.
     * Applicable to: Database, Table, View, Index
     */
    kShowSystem = 6,

    /**
     * CREATE permission.
     * Applicable to: Database, Table, View, Index, Trigger, Procedure, Function, User
     */
    kCreate = 7,

    /**
     * DROP permission.
     * Applicable to: Database, Table, View, Index, Trigger, Procedure, Function, User
     */
    kDrop = 8,

    /**
     * ALTER permission.
     * Applicable to: Database, Table, View, Index, Trigger, Procedure, Function, User
     */
    kAlter = 9,

    /**
     * ATTACH permission.
     * Applicable to: Database
     */
    kAttach = 10,

    /**
     * DETACH permission.
     * Applicable to: Database
     */
    kDetach = 11,

    /**
     * DETACH permission.
     * Applicable to: Instance
     */
    kShutdown = 12,

    /**
     * SHOW PERMISSIONS permission.
     * Applicable to: User
     */
    kShowPermissions = 13,

    kMax
};

/**
 * Returns permission type name.
 * @param permissionType Permission type (as integer number).
 * @return Permission type name.
 */
const char* getPermissionTypeName(int permissionType);

/**
 * Returns permission type name.
 * @param permissionType Permission type.
 * @return Permission type name.
 */
inline const char* getPermissionTypeName(PermissionType permissionType)
{
    return getPermissionTypeName(static_cast<int>(permissionType));
}

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

/** SHUTDOWN permission bitmask */
constexpr auto kShutdownPermissionMask = getSinglePermissionMask(PermissionType::kShutdown);

/** SHOW PERMISSIONS permission bitmask */
constexpr auto kShowPermissionsPermissionMask =
        getSinglePermissionMask(PermissionType::kShowPermissions);

}  // namespace siodb::iomgr::dbengine
