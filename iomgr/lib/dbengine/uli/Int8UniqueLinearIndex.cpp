// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Int8UniqueLinearIndex.h"

// Project headers
#include "../ikt/Int8IndexKeyTraits.h"

namespace siodb::iomgr::dbengine {

Int8UniqueLinearIndex::Int8UniqueLinearIndex(Table& table, const std::string& name,
        std::size_t valueSize, const IndexColumnSpecification& columnSpec,
        std::uint32_t dataFileSize)
    : UniqueLinearIndex(table, IndexType::kLinearIndexI8, name, Int8IndexKeyTraits(), valueSize,
            &Int8IndexKeyTraits::compareKeys, columnSpec, dataFileSize)
{
}

Int8UniqueLinearIndex::Int8UniqueLinearIndex(
        Table& table, const IndexRecord& indexRecord, std::size_t valueSize)
    : UniqueLinearIndex(
            table, indexRecord, Int8IndexKeyTraits(), valueSize, &Int8IndexKeyTraits::compareKeys)
{
}

}  // namespace siodb::iomgr::dbengine
