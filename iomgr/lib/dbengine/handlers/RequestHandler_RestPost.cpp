// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "../ThrowDatabaseError.h"

// Common project headers
#include <siodb/common/protobuf/ProtobufMessageIO.h>

namespace siodb::iomgr::dbengine {

void RequestHandler::executePostRowsRestRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::PostRowsRestRequest& request)
{
    sendNotImplementedYet(response);
}

}  // namespace siodb::iomgr::dbengine
