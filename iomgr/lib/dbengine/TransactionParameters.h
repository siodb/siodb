// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// CRT headers
#include <cstdint>
#include <ctime>

namespace siodb::iomgr::dbengine {

/** Transaction parameters */
struct TransactionParameters {
    /** Initializes object of class TransactionParameters */
    TransactionParameters() noexcept
        : m_transactionId(0)
        , m_timestamp(0)
        , m_userId(0)
    {
    }

    /**
     * Initializes object of class TransactionParameters.
     * @param userId User ID.
     * @param transactionId Transaction ID.
     * @param timestamp Transaction timestamp.
     */
    TransactionParameters(std::uint32_t userId, std::uint64_t transactionId,
            std::time_t timestamp = std::time(nullptr)) noexcept
        : m_transactionId(transactionId)
        , m_timestamp(timestamp)
        , m_userId(userId)
    {
    }

    /** Changes byte order of the all contained data. */
    void flipByteOrder() noexcept;

    /** Transaction ID */
    std::uint64_t m_transactionId;

    /** Transaction timestamp */
    std::time_t m_timestamp;

    /** User ID */
    std::uint32_t m_userId;
};

}  // namespace siodb::iomgr::dbengine
