// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "TransactionParameters.h"

// STL headers
#include <atomic>

namespace siodb::iomgr::dbengine {

/** Database metadata includes various persistent parameters. */
class DatabaseMetadata {
public:
    /**
     * Initializes object of class DatabaseMetadata.
     * @param userId Database initialization transaction user.
     */
    explicit DatabaseMetadata(std::uint32_t userId) noexcept
        : m_marker(kMarker)
        , m_version(kCurrentVersion)
        , m_lastTransactionId(0)
        , m_lastAtomicOperationId(0)
        , m_initTransactionParams(userId, ++m_lastTransactionId)
    {
    }

    /** Copy construction disabled. */
    DatabaseMetadata(const DatabaseMetadata&) = delete;

    /** Move construction disabled. */
    DatabaseMetadata(DatabaseMetadata&&) = delete;

    /**
     * Adjusts byte order of the all data if necessary.
     * @return true if byte order was adjusted, false if adjustement was not required.
     * @throw std::runtime_error if it wasn't possible to determine
     *        whether byte order adjustement is required or not.
     */
    bool adjustByteOrder();

    /**
     * Retuns database initialization transaction parameters.
     * @return Database initialization transaction parameters object.
     */
    const auto& getInitTransactionParams() const noexcept
    {
        return m_initTransactionParams;
    }

    /**
     * Generates next transaction ID.
     * @return New unqiue transaction ID.
     */
    std::uint64_t generateNextTransactionId() noexcept
    {
        return ++m_lastTransactionId;
    }

    /**
     * Generates next transaction ID.
     * @return New unqiue transaction ID.
     */
    std::uint64_t generateNextAtomicOperationId() noexcept
    {
        return ++m_lastAtomicOperationId;
    }

private:
    /** Changes byte order of the all contained data. */
    void flipByteOrder() noexcept;

private:
    /** Marker that is used to detect endianness. */
    std::uint64_t m_marker;

    /** Data version. Always little-endian value. Use ::pbeDecodeUInt64() to read it correctly. */
    std::uint64_t m_version;

    /** Last transaction ID */
    std::atomic<std::uint64_t> m_lastTransactionId;

    /** Last atomic operation ID */
    std::atomic<std::uint64_t> m_lastAtomicOperationId;

    /** Database initialization transaction parameters */
    const TransactionParameters m_initTransactionParams;

    /** Current medatata version */
    static constexpr std::uint64_t kCurrentVersion = 1;

    /** Marker value */
    static constexpr std::uint64_t kMarker = 0x0123456789ABCDEFULL;
};

}  // namespace siodb::iomgr::dbengine
