// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/parser/DBEngineRestRequest.h"
#include "dbengine/parser/DBEngineRestRequestFactory.h"

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace parser_ns = dbengine::parser;
namespace req_ns = dbengine::requests;

TEST(Patch, PatchRow)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::PATCH);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_id(1);

#if 0
    // Create request object
    const auto request = parser_ns::DBEngineRestRequestFactory::createRestRequest(requestMsg);

    // Check request object
    ASSERT_EQ(request->m_requestType, req_ns::DBEngineRequestType::kRestPatchRow);
    const auto r = dynamic_cast<const req_ns::PatchRowRestRequest*>(request.get());
    ASSERT_NE(r, nullptr);
#endif
}
