// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/** Database object types used in permission control. */
enum class DatabaseObjectType {
    kInstance,
    kDatabase,
    kTable,
    kSingleColumnReference,
    kIndex,
    kConstraint,
    kTrigger,
    kProcedure,
    kFunction,
};

}  // namespace siodb::iomgr::dbengine
