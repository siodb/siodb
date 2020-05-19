// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "EmptyContext.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine::requests {

const siodb::iomgr::dbengine::Variant& EmptyContext::getColumnValue(
        [[maybe_unused]] std::size_t tableIndex, [[maybe_unused]] std::size_t columnIndex)
{
    throwDatabaseError(IOManagerMessageId::kErrorOnlyConstantExpressionsAreAllowed);
}

siodb::ColumnDataType EmptyContext::getColumnDataType(
        [[maybe_unused]] std::size_t tableIndex, [[maybe_unused]] std::size_t columnIndex) const
{
    throwDatabaseError(IOManagerMessageId::kErrorOnlyConstantExpressionsAreAllowed);
}

}  // namespace siodb::iomgr::dbengine::requests
