// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/** Constraint state */
enum class ConstraintState {
    kCreating,
    kActive,
    kDeleted,
    kMax,
};

}  // namespace siodb::iomgr::dbengine
