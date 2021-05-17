// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SqlClientProtocolRowsetWriterFactory.h"

// Project headers
#include "SqlClientProtocolRowsetWriter.h"

namespace siodb::iomgr::dbengine {

std::unique_ptr<RowsetWriter> SqlClientProtocolRowsetWriterFactory::createRowsetWriter(
        siodb::io::OutputStream& connection)
{
    auto p = std::make_unique<SqlClientProtocolRowsetWriter>(connection);
    return std::unique_ptr<RowsetWriter>(p.release());
}

}  // namespace siodb::iomgr::dbengine
