// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/** Database object types used in the permission control. */
enum class DatabaseObjectType {
    kInstance,
    kDatabase,
    kTable,
    kColumn,
    kIndex,
    kConstraint,
    kTrigger,
    kProcedure,
    kFunction,
    kMax
};

}  // namespace siodb::iomgr::dbengine
