// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/** Database object types used in the permission control. */
enum class DatabaseObjectType {
    kInstance = 0,
    kDatabase = 1,
    kTable = 2,
    kColumn = 3,
    kIndex = 4,
    kConstraint = 5,
    kTrigger = 6,
    kProcedure = 7,
    kFunction = 8,
    kUser = 9,
    kUserAccessKey = 10,
    kUserToken = 11,
    kMax
};

/**
 * Returns database object type name.
 * @param objectType Database object type (as integer number).
 * @return Database object type name.
 */
const char* getDatabaseObjectTypeName(int objectType);

/**
 * Returns database object type name.
 * @param objectType Database object type.
 * @return Database object type name.
 */
inline const char* getDatabaseObjectTypeName(DatabaseObjectType objectType)
{
    return getDatabaseObjectTypeName(static_cast<int>(objectType));
}

}  // namespace siodb::iomgr::dbengine
