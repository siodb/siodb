// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "MasterColumnRecord.h"
#include "MasterColumnRecordPtr.h"

namespace siodb::iomgr::dbengine {

/**
 * Insert row result data.
 */
struct InsertRowResult {
    /**
     * Initializes object of class InsertRowResult.
     */
    InsertRowResult() noexcept = default;

    /**
     * Initializes object of class InsertRowResult.
     * @param mcr New master column record.
     * @param nextBlockIds List of block IDs available for a next data modification operation.
     */
    InsertRowResult(MasterColumnRecordPtr&& mcr, std::vector<std::uint64_t>&& nextBlockIds) noexcept
        : m_mcr(std::move(mcr))
        , m_nextBlockIds(std::move(nextBlockIds))
    {
    }

    /** New master column record */
    MasterColumnRecordPtr m_mcr;

    /** List of block IDs available for a next data modification operation */
    std::vector<std::uint64_t> m_nextBlockIds;
};

}  // namespace siodb::iomgr::dbengine
