// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "MasterColumnRecord.h"
#include "MasterColumnRecordPtr.h"

namespace siodb::iomgr::dbengine {

/**
 * Delete row result data.
 */
struct DeleteRowResult {
    /**
     * Initializes object of class DeleteRowResult.
     */
    DeleteRowResult() noexcept
        : m_deleted(false)
    {
    }

    /**
     * Initializes object of class DeleteRowResult.
     * @param deleted Delete result, true is successful, false if row not found.
     * @param mcr New master column record.
     * @param mcrAddress New master column record address.
     * @param nextAddress Next available address.
     */
    DeleteRowResult(bool deleted, MasterColumnRecordPtr&& mcr, const ColumnDataAddress& mcrAddress,
            const ColumnDataAddress& nextAddress) noexcept
        : m_deleted(deleted)
        , m_mcr(std::move(mcr))
        , m_mcrAddress(mcrAddress)
        , m_nextAddress(nextAddress)
    {
    }

    /** Delete result - true if successful, false if row not found */
    bool m_deleted;

    /** New master column record */
    MasterColumnRecordPtr m_mcr;

    /** Master column record address */
    ColumnDataAddress m_mcrAddress;

    /** Next available address */
    ColumnDataAddress m_nextAddress;
};

}  // namespace siodb::iomgr::dbengine
