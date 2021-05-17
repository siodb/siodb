// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "dbengine/parser/DBEngineRestRequest.h"
#include "dbengine/parser/DBEngineRestRequestFactory.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/io/BufferedChunkedOutputStream.h>
#include <siodb/common/io/MemoryInputStream.h>
#include <siodb/common/io/MemoryOutputStream.h>
#include <siodb/common/log/Log.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;
namespace parser_ns = dbengine::parser;
namespace req_ns = dbengine::requests;

TEST(Patch, PatchSingleRow)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::PATCH);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name_or_query("Abcd.efGh");
    requestMsg.set_object_id(1);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        constexpr const char* kSingleRowJson = R"json(
            [
                {
                    "int_field": -2,
                    "uint_field": 3,
                    "string_field": "hello world!!!",
                    "float_field": 18.0,
                    "null_field": null
                }
            ]
        )json";
        //DBG_LOG_DEBUG("\n===== JSON =====\n" << kSingleRowJson << "\n=====\n");
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg, &in);

    // Check request object
    ASSERT_EQ(request->m_requestType, req_ns::DBEngineRequestType::kRestPatchRow);
    const auto r = dynamic_cast<const req_ns::PatchRowRestRequest*>(request.get());
    ASSERT_NE(r, nullptr);
    ASSERT_EQ(r->m_database, "ABCD");
    ASSERT_EQ(r->m_table, "EFGH");

    // Check row
    ASSERT_EQ(r->m_columnNames.size(), 5U);
    ASSERT_EQ(r->m_values.size(), 5U);

    const std::array<const char*, 5U> expectedFieldNames = {
            "INT_FIELD",
            "UINT_FIELD",
            "STRING_FIELD",
            "FLOAT_FIELD",
            "NULL_FIELD",
    };

    const std::array<dbengine::Variant, 5U> expectedValues {
            dbengine::Variant(-2),
            dbengine::Variant(3U),
            dbengine::Variant("hello world!!!"),
            dbengine::Variant(18.0),
            dbengine::Variant(),
    };

    for (std::size_t i = 0; i < expectedValues.size(); ++i) {
        ASSERT_EQ(r->m_columnNames.at(i), expectedFieldNames[i]);
        const auto& v = r->m_values.at(i);
        const auto& expected = expectedValues[i];
        if (expected.isNull()) {
            ASSERT_TRUE(v.isNull());
        } else {
            ASSERT_TRUE(v.compatibleEqual(expected));
        }
    }
}

TEST(Patch, TryPatchMultipleRows)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::PATCH);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name_or_query("Abcd.efGh");
    requestMsg.set_object_id(1);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        constexpr const char* kSingleRowJson = R"json(
            [
                {
                    "int_field": -2,
                    "uint_field": 3,
                    "string_field": "hello world!!!",
                    "float_field": 18.0,
                    "null_field": null
                },
                {
                    "int_field": -2,
                    "uint_field": 3,
                    "string_field": "hello world!!!",
                    "float_field": 18.0,
                    "null_field": null
                }
            ]
        )json";
        //DBG_LOG_DEBUG("\n===== JSON =====\n" << kSingleRowJson << "\n=====\n");
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    try {
        requestFactory.createRestRequest(requestMsg, &in);
        FAIL() << "Patching multiple rows at once is not allowed";
    } catch (...) {
        // expected
    }
}

TEST(Patch, TryPatchNoRows)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::PATCH);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name_or_query("Abcd.efGh");
    requestMsg.set_object_id(1);

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        constexpr const char* kSingleRowJson = R"json(
            [
            ]
        )json";
        //DBG_LOG_DEBUG("\n===== JSON =====\n" << kSingleRowJson << "\n=====\n");
        chunkedOutput.write(kSingleRowJson, ::ct_strlen(kSingleRowJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    try {
        requestFactory.createRestRequest(requestMsg, &in);
        FAIL() << "Patching no rows is not allowed";
    } catch (...) {
        // expected
    }
}
