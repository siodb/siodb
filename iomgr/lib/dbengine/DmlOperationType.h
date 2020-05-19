// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/** DML operation type. Used in master column records. */
enum class DmlOperationType {
    kInsert = 0,
    kUpdate = 1,
    kDelete = 2,
};

}  // namespace siodb::iomgr::dbengine
