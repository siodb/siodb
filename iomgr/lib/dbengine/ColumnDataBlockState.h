// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/** Column data block state */
enum class ColumnDataBlockState {
    kCreating = 1,
    kClosing = 2,
    kClosed = 3,
    kCurrent = 4,
    kAvailable = 5
};

}  // namespace siodb::iomgr::dbengine
