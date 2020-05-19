// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DatabaseMetadata.h"

// Common project headers
#include <siodb/common/utils/ByteOrder.h>

// STL headers
#include <stdexcept>

namespace siodb::iomgr::dbengine {

//////////////////////// class DatabaseMetadata ////////////////////////////////////////////////////

bool DatabaseMetadata::adjustByteOrder()
{
    if (m_marker == kMarker) return false;
    if (boost::endian::endian_reverse(m_marker) != kMarker)
        throw std::runtime_error("Database metadata corrupted");
    flipByteOrder();
    return true;
}

void DatabaseMetadata::flipByteOrder() noexcept
{
    utils::reverseByteOrder(m_lastTransactionId);
    utils::reverseByteOrder(m_lastAtomicOperationId);
    utils::reverseByteOrder(m_marker);
}

}  // namespace siodb::iomgr::dbengine
