// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "RowsetWriter.h"

// Common project headers
#include <siodb/common/io/OutputStream.h>

// STL headers
#include <memory>

namespace siodb::iomgr::dbengine {

/**
 * An abstract factory for creating rowset writer objects.
 */
class RowsetWriterFactory {
public:
    /** De-initializes object of class RowsetWriterFactory. */
    virtual ~RowsetWriterFactory() = default;

    /**
     * Creates new rowset writer object.
     * @param connection Client connection stream.
     * @return New rowset writer object.
     */
    virtual std::unique_ptr<RowsetWriter> createRowsetWriter(
            siodb::io::OutputStream& connection) = 0;
};

}  // namespace siodb::iomgr::dbengine
