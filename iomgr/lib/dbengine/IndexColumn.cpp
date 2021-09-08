// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IndexColumn.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

// --- internals ---

Index& IndexColumn::validateIndex(Index& index, const IndexColumnRecord& indexColumnRecord)
{
    if (indexColumnRecord.m_indexId == index.getId()) return index;
    throwDatabaseError(IOManagerMessageId::kErrorInvalidIndexColumnIndex, indexColumnRecord.m_id,
            indexColumnRecord.m_indexId, index.getTable().getDatabaseName(), index.getTableName(),
            index.getName(), index.getTable().getDatabaseUuid(), index.getTableId(), index.getId());
}

}  // namespace siodb::iomgr::dbengine
