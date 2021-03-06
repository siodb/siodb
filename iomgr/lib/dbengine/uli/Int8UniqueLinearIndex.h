// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "UniqueLinearIndex.h"

namespace siodb::iomgr::dbengine {

/** Unique linear index with key of type uint8_t. */
class Int8UniqueLinearIndex : public UniqueLinearIndex {
public:
    /**
     * Initializes object of class UniqueLinearIndex.
     * @param table A table to which this index belongs.
     * @param name Index name.
     * @param valueSize Value size.
     * @param columnSpec Indexed column specification.
     * @param dataFileSize Data file size.
     * @param description Index description.
     */
    Int8UniqueLinearIndex(Table& table, std::string&& name, std::size_t valueSize,
            const IndexColumnSpecification& columnSpec, std::uint32_t dataFileSize,
            std::optional<std::string>&& description);

    /**
     * Initializes object of class Index for an existing index.
     * @param table A table to which this index belongs.
     * @param indexRecord Index record.
     * @param valueSize Value size.
     */
    Int8UniqueLinearIndex(Table& table, const IndexRecord& indexRecord, std::size_t valueSize);
};

}  // namespace siodb::iomgr::dbengine
