// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine {

/** Column index type */
enum class IndexType {
    kLinearIndexI8,
    kLinearIndexU8,
    kLinearIndexI16,
    kLinearIndexU16,
    kLinearIndexI32,
    kLinearIndexU32,
    kLinearIndexI64,
    kLinearIndexU64,
    kBPlusTreeIndex,  // not implemented yet
    kHashIndex  // not supported yet
};

}  // namespace siodb::iomgr::dbengine
