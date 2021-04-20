// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "MasterColumnRecord.h"
#include "MasterColumnRecordPtr.h"

namespace siodb::iomgr::dbengine {

/**
 * Update row result data.
 */
struct UpdateRowResult {
    /**
     * Initializes object of class UpdateRowResult.
     */
    UpdateRowResult() noexcept
        : m_updated(false)
    {
    }

    /**
     * Initializes object of class UpdateRowResult.
     * @param updated Update result, true is successful, false if row not found.
     * @param mcr New master column record.
     * @param nextBlockIds List of block IDs available for a next data modification operation.
     */
    UpdateRowResult(bool updated, MasterColumnRecordPtr&& mcr,
            std::vector<std::uint64_t>&& nextBlockIds) noexcept
        : m_updated(updated)
        , m_mcr(std::move(mcr))
        , m_nextBlockIds(std::move(nextBlockIds))
    {
    }

    /** Update result, true is successful, false if row not found. */
    bool m_updated;

    /** New master column record */
    MasterColumnRecordPtr m_mcr;

    /** List of block IDs available for a next data modification operation */
    std::vector<std::uint64_t> m_nextBlockIds;
};

}  // namespace siodb::iomgr::dbengine
