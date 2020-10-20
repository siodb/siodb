// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/** Column data block state */
enum class ColumnDataBlockState {
    /** Specical value that marks block is not present in the block registry file */
    kNotPresent = 0,

    /** The block is in process of creation */
    kCreating = 1,

    /** The block is in process of closing */
    kClosing = 2,

    /** The block is closed, i.e. no longer writeable */
    kClosed = 3,

    /** The block is the one to which data must be written */
    kCurrent = 4,

    /** The block is candidate to be next "current" block */
    kAvailable = 5
};

}  // namespace siodb::iomgr::dbengine
