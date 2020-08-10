// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "TransactionParameters.h"

// Common project headers
#include <siodb/common/utils/ByteOrder.h>

namespace siodb::iomgr::dbengine {

void TransactionParameters::flipByteOrder() noexcept
{
    utils::reverseByteOrder(m_transactionId);
    utils::reverseByteOrder(m_timestamp);
    utils::reverseByteOrder(m_userId);
}

}  // namespace siodb::iomgr::dbengine
