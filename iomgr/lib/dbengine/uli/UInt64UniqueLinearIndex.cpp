// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UInt64UniqueLinearIndex.h"

// Project headers
#include "../ikt/UInt64IndexKeyTraits.h"

namespace siodb::iomgr::dbengine {

UInt64UniqueLinearIndex::UInt64UniqueLinearIndex(Table& table, const std::string& name,
        std::size_t valueSize, const IndexColumnSpecification& columnSpec,
        std::uint32_t dataFileSize)
    : UniqueLinearIndex(table, IndexType::kLinearIndexU64, name, UInt64IndexKeyTraits(), valueSize,
            &UInt64IndexKeyTraits::compareKeys, columnSpec, dataFileSize)
{
}

UInt64UniqueLinearIndex::UInt64UniqueLinearIndex(
        Table& table, const IndexRecord& indexRecord, std::size_t valueSize)
    : UniqueLinearIndex(table, indexRecord, UInt64IndexKeyTraits(), valueSize,
            &UInt64IndexKeyTraits::compareKeys)
{
}

}  // namespace siodb::iomgr::dbengine
