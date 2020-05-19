// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UInt16UniqueLinearIndex.h"

// Project headers
#include "../ikt/UInt16IndexKeyTraits.h"

namespace siodb::iomgr::dbengine {

UInt16UniqueLinearIndex::UInt16UniqueLinearIndex(Table& table, const std::string& name,
        std::size_t valueSize, const IndexColumnSpecification& columnSpec,
        std::uint32_t dataFileSize)
    : UniqueLinearIndex(table, IndexType::kLinearIndexU16, name, UInt16IndexKeyTraits(), valueSize,
            &UInt16IndexKeyTraits::compareKeys, columnSpec, dataFileSize)
{
}

UInt16UniqueLinearIndex::UInt16UniqueLinearIndex(
        Table& table, const IndexRecord& indexRecord, std::size_t valueSize)
    : UniqueLinearIndex(table, indexRecord, UInt16IndexKeyTraits(), valueSize,
            &UInt16IndexKeyTraits::compareKeys)
{
}

}  // namespace siodb::iomgr::dbengine
