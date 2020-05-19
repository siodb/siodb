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

void RequestHandler::executeBeginTransactionRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::BeginTransactionRequest& request)
{
    sendNotImplementedYet(response);
}

void RequestHandler::executeCommitTransactionRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::CommitTransactionRequest& request)
{
    sendNotImplementedYet(response);
}

void RequestHandler::executeRollbackTransactionRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::RollbackTransactionRequest& request)
{
    sendNotImplementedYet(response);
}

void RequestHandler::executeSavepointRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::SavepointRequest& request)
{
    sendNotImplementedYet(response);
}

void RequestHandler::executeReleaseRequest(iomgr_protocol::DatabaseEngineResponse& response,
        [[maybe_unused]] const requests::ReleaseRequest& request)
{
    sendNotImplementedYet(response);
}

}  // namespace siodb::iomgr::dbengine
