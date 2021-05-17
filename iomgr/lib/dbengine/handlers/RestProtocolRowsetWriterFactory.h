// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "RowsetWriterFactory.h"

namespace siodb::iomgr::dbengine {

/**
 * An abstract factory implemntation for creating rowset writer objects
 * of class RestProtocolRowsetWriter.
 */
class RestProtocolRowsetWriterFactory : public RowsetWriterFactory {
public:
    /**
     * Creates new rowset writer object.
     * @param connection Client connection stream.
     * @return New rowset writer object.
     */
    std::unique_ptr<RowsetWriter> createRowsetWriter(siodb::io::OutputStream& connection) override;
};

}  // namespace siodb::iomgr::dbengine
