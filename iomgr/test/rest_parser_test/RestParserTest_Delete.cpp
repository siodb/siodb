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

TEST(Delete, DeleteRow)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::DELETE);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name("abcd.efgh");
    requestMsg.set_object_id(1);

    // Create request object
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg);

    // Check request object
    ASSERT_EQ(request->m_requestType, req_ns::DBEngineRequestType::kRestDeleteRow);
    const auto r = dynamic_cast<const req_ns::DeleteRowRestRequest*>(request.get());
    ASSERT_NE(r, nullptr);
    ASSERT_EQ(r->m_database, "ABCD");
    ASSERT_EQ(r->m_table, "EFGH");
    ASSERT_EQ(r->m_trid, 1U);
}
