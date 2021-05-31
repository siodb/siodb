// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "JsonOutput.h"
#include "RestProtocolRowsetWriterFactory.h"
#include "../ThrowDatabaseError.h"

namespace siodb::iomgr::dbengine {

void RequestHandler::executeGetSqlQueryRowsRestRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::GetSqlQueryRowsRestRequest& request)
{
    response.set_has_affected_row_count(false);
    response.set_affected_row_count(0);
    RestProtocolRowsetWriterFactory rowsetWriterFactory;
    executeSelectRequest(response, *request.m_query, rowsetWriterFactory);
}

}  // namespace siodb::iomgr::dbengine
