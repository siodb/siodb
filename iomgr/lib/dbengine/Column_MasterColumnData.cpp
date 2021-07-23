// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Column.h"

namespace siodb::iomgr::dbengine {

////// class Column //////

// --- internals ---

std::unique_ptr<Column::MasterColumnData> Column::maybeCreateMasterColumnData(
        bool create, std::uint64_t firstUserTrid)
{
    if (!isMasterColumnName()) return nullptr;
    return std::make_unique<MasterColumnData>(*this, create, firstUserTrid);
}

////// class Column::MasterColumnData //////

Column::MasterColumnData::MasterColumnData(
        Column& parent, bool createCounters, std::uint64_t firstUserTrid)
    : m_firstUserTrid(firstUserTrid)
    , m_file(createCounters ? parent.createTridCountersFile(firstUserTrid)
                            : parent.openTridCountersFile(),
              true, PROT_READ | PROT_WRITE, MAP_POPULATE, 0, sizeof(DatabaseMetadata))
    , m_tridCounters(reinterpret_cast<TridCounters*>(m_file.getMappingAddress()))
{
}

}  // namespace siodb::iomgr::dbengine
