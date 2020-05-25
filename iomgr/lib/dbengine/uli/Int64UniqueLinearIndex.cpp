// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Int64UniqueLinearIndex.h"

// Project headers
#include "../ikt/Int64IndexKeyTraits.h"

namespace siodb::iomgr::dbengine {

Int64UniqueLinearIndex::Int64UniqueLinearIndex(Table& table, std::string&& name,
        std::size_t valueSize, const IndexColumnSpecification& columnSpec,
        std::uint32_t dataFileSize, std::optional<std::string>&& description)
    : UniqueLinearIndex(table, IndexType::kLinearIndexI64, std::move(name), Int64IndexKeyTraits(),
            valueSize, &Int64IndexKeyTraits::compareKeys, columnSpec, dataFileSize,
            std::move(description))
{
}

Int64UniqueLinearIndex::Int64UniqueLinearIndex(
        Table& table, const IndexRecord& indexRecord, std::size_t valueSize)
    : UniqueLinearIndex(
            table, indexRecord, Int64IndexKeyTraits(), valueSize, &Int64IndexKeyTraits::compareKeys)
{
}

}  // namespace siodb::iomgr::dbengine
