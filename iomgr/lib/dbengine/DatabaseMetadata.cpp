// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DatabaseMetadata.h"

// Common project headers
#include <siodb/common/stl_ext/utility_ext.h>
#include <siodb/common/utils/ByteOrder.h>
#include <siodb/common/utils/PlainBinaryEncoding.h>

// STL headers
#include <stdexcept>

namespace siodb::iomgr::dbengine {

DatabaseMetadata::DatabaseMetadata(std::uint32_t userId) noexcept
    : m_marker(kMarker)
    , m_lastTransactionId(0)
    , m_lastAtomicOperationId(0)
    , m_initTransactionParams(userId, ++m_lastTransactionId)
    , m_schemaVersion(kCurrentSchemaVersion)
{
    ::pbeEncodeUInt64(kCurrentVersion, m_version);
}

std::uint64_t DatabaseMetadata::getVersion() const noexcept
{
    std::uint64_t version = 0;
    return ::pbeDecodeUInt64(m_version, &version) ? version : 0xFFFFFFFFFFFFFFFFULL;
}

bool DatabaseMetadata::adjustByteOrder()
{
    if (m_marker == kMarker) return false;
    if (boost::endian::endian_reverse(m_marker) != kMarker)
        throw std::runtime_error("Database metadata corrupted");
    flipByteOrder();
    return true;
}

// ----- internals -----

void DatabaseMetadata::flipByteOrder() noexcept
{
    utils::reverseByteOrder(m_lastTransactionId);
    utils::reverseByteOrder(m_lastAtomicOperationId);
    stdext::as_mutable(m_initTransactionParams).flipByteOrder();
    utils::reverseByteOrder(m_schemaVersion);
    // must be changed last
    utils::reverseByteOrder(m_marker);
}

}  // namespace siodb::iomgr::dbengine
