// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
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

TEST(Post, PostSingleRow)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::POST);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name("Abcd.efGh");

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
    ASSERT_EQ(request->m_requestType, req_ns::DBEngineRequestType::kRestPostRows);
    const auto r = dynamic_cast<const req_ns::PostRowsRestRequest*>(request.get());
    ASSERT_NE(r, nullptr);
    ASSERT_EQ(r->m_database, "ABCD");
    ASSERT_EQ(r->m_table, "EFGH");

    // Check row
    ASSERT_EQ(r->m_values.size(), 1U);

    const auto& row = r->m_values.at(0);
    ASSERT_EQ(row.size(), 5U);

    const std::array<dbengine::Variant, 5U> expectedValues {
            dbengine::Variant(-2),
            dbengine::Variant(3U),
            dbengine::Variant("hello world!!!"),
            dbengine::Variant(18.0),
            dbengine::Variant(),
    };

    for (std::size_t i = 0; i < expectedValues.size(); ++i) {
        const auto& e = row.at(i);
        ASSERT_EQ(e.first, i + 1);
        const auto& expected = expectedValues[i];
        if (expected.isNull()) {
            ASSERT_TRUE(e.second.isNull());
        } else {
            ASSERT_TRUE(e.second.compatibleEqual(expected));
        }
    }
}

TEST(Post, PostMultipleRows)
{
    // Create source protobuf message
    siodb::iomgr_protocol::DatabaseEngineRestRequest requestMsg;
    requestMsg.set_request_id(1);
    requestMsg.set_verb(siodb::iomgr_protocol::POST);
    requestMsg.set_object_type(siodb::iomgr_protocol::ROW);
    requestMsg.set_object_name("AbcD.efGh");

    // Create JSON payload
    stdext::buffer<std::uint8_t> payloadBuffer(4096);
    siodb::io::MemoryOutputStream out(payloadBuffer.data(), payloadBuffer.size());
    {
        siodb::io::BufferedChunkedOutputStream chunkedOutput(17, out);
        constexpr const char* kMultipleRowsJson = R"json(
            [
                {
                    "int_field": -2,
                    "uint_field": 3,
                    "string_field": "hello world!!!",
                    "float_field": 18.0,
                    "null_field": null
                },
                {
                    "int_field": -5,
                    "uint_field": 2,
                    "zero_field": 0,
                    "float_field": 18.0,
                    "string_field": "hello world again!!!"
                },
                {
                    "string_field": "hello world one more time!!!",
                    "null_field": null,
                    "yet_another_field": "something new"
                }
            ]
        )json";
        //DBG_LOG_DEBUG("\n===== JSON =====\n" << kMultipleRowsJson << "\n=====\n");
        chunkedOutput.write(kMultipleRowsJson, ::ct_strlen(kMultipleRowsJson));
    }

    // Create request object
    siodb::io::MemoryInputStream in(
            payloadBuffer.data(), payloadBuffer.size() - out.getRemaining());
    parser_ns::DBEngineRestRequestFactory requestFactory(1024 * 1024);
    const auto request = requestFactory.createRestRequest(requestMsg, &in);

    // Check request object
    ASSERT_EQ(request->m_requestType, req_ns::DBEngineRequestType::kRestPostRows);
    const auto r = dynamic_cast<const req_ns::PostRowsRestRequest*>(request.get());
    ASSERT_NE(r, nullptr);
    ASSERT_EQ(r->m_database, "ABCD");
    ASSERT_EQ(r->m_table, "EFGH");

    // Check rows
    const std::array<std::vector<std::pair<unsigned, dbengine::Variant>>, 3U> expectedValues {
            std::vector<std::pair<unsigned, dbengine::Variant>> {
                    {1U, dbengine::Variant(-2)},
                    {2U, dbengine::Variant(3U)},
                    {3U, dbengine::Variant("hello world!!!")},
                    {4U, dbengine::Variant(18.0)},
                    {5U, dbengine::Variant()},
            },
            std::vector<std::pair<unsigned, dbengine::Variant>> {
                    {1U, dbengine::Variant(-5)},
                    {2U, dbengine::Variant(2)},
                    {6U, dbengine::Variant(0)},
                    {4U, dbengine::Variant(18.0)},
                    {3U, dbengine::Variant("hello world again!!!")},
            },
            std::vector<std::pair<unsigned, dbengine::Variant>> {
                    {3U, dbengine::Variant("hello world one more time!!!")},
                    {5U, dbengine::Variant()},
                    {7U, dbengine::Variant("something new")},
            },
    };

    ASSERT_EQ(r->m_values.size(), expectedValues.size());

    for (std::size_t j = 0; j < expectedValues.size(); ++j) {
        const auto& row = r->m_values.at(j);
        const auto& expectedRow = expectedValues[j];
        const auto n = expectedRow.size();
        ASSERT_EQ(row.size(), n);
        for (std::size_t i = 0; i < n; ++i) {
            //DBG_LOG_DEBUG("Row #" << j << ", Column #" << i);
            const auto& e = row.at(i);
            const auto& expected = expectedRow[i];
            ASSERT_EQ(e.first, expected.first);
            if (expected.second.isNull()) {
                ASSERT_TRUE(e.second.isNull());
            } else {
                ASSERT_TRUE(e.second.compatibleEqual(expected.second));
            }
        }
    }
}
