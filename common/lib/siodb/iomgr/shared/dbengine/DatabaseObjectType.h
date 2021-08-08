// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/** Database object types used in the permission control. */
enum class DatabaseObjectType {
    kNoObject = 0,
    kInstance = 1,
    kDatabase = 2,
    kTable = 3,
    kColumn = 4,
    kIndex = 5,
    kConstraint = 6,
    kTrigger = 7,
    kProcedure = 8,
    kFunction = 9,
    kUser = 10,
    kUserAccessKey = 11,
    kUserToken = 12,
    kMax
};

}  // namespace siodb::iomgr::dbengine
