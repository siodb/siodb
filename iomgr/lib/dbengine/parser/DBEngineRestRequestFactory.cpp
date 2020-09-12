// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DBEngineRestRequestFactory.h"

// Project headers
#include "DBEngineRequestFactoryError.h"
#include "DBEngineRestRequest.h"
#include "JsonParserError.h"
#include "RowDataJsonSaxParser.h"

// Common project headers
#include <siodb/common/io/ChunkedInputStream.h>
#include <siodb/common/io/InputStreamStdStreamBuffer.h>
#include <siodb/common/io/MemoryStdStreamBuffer.h>
#include <siodb/common/log/Log.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

// STL headers
#include <sstream>

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace siodb::iomgr::dbengine::parser {

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createRestRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg, siodb::io::InputStream* input)
{
    switch (msg.verb()) {
        case iomgr_protocol::GET: {
            switch (msg.object_type()) {
                case iomgr_protocol::DATABASE: return createGetDatabasesRequest();
                case iomgr_protocol::TABLE: return createGetTablesRequest(msg);
                case iomgr_protocol::ROW: {
                    return msg.object_id() == 0 ? createGetAllRowsRequest(msg)
                                                : createGetSingleRowRequest(msg);
                }
                default: {
                    throw DBEngineRequestFactoryError(
                            "REST request: Invalid object type for the GET request");
                }
            }
        }

        case iomgr_protocol::POST: {
            if (input == nullptr) {
                throw std::runtime_error(
                        "DBEngineRestRequestFactory::createRestRequest(): Missing input stream, "
                        "it is required to create POST request");
            }
            switch (msg.object_type()) {
                case iomgr_protocol::ROW: return createPostRowsRequest(msg, *input);
                default: {
                    throw DBEngineRequestFactoryError(
                            "REST request: Invalid or unsupported object type for the POST "
                            "request");
                }
            }
        }

        case iomgr_protocol::PATCH: {
            if (input == nullptr) {
                throw std::runtime_error(
                        "DBEngineRestRequestFactory::createRestRequest(): Missing input stream, "
                        "it is required to create PATCH request");
            }
            // TODO: implement PATCH requests
            throw DBEngineRequestFactoryError("PATCH is not supported yet");
        }

        case iomgr_protocol::DELETE: {
            // TODO: implement DELETE requests
            throw DBEngineRequestFactoryError("DELETE is not supported yet");
        }

        default: throw DBEngineRequestFactoryError("REST request: Invalid verb");
    }
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createGetDatabasesRequest()
{
    return std::make_shared<requests::GetDatabasesRestRequest>();
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createGetTablesRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg)
{
    if (!isValidDatabaseObjectName(msg.object_name()))
        throw DBEngineRequestFactoryError("GET TABLES: Invalid database name");
    return std::make_shared<requests::GetTablesRestRequest>(
            boost::to_upper_copy(msg.object_name()));
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createGetAllRowsRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name(), boost::is_any_of("."));
    if (components.size() != 2)
        throw DBEngineRequestFactoryError("GET ALL ROWS: Invalid object name");

    boost::trim(components[0]);
    if (!isValidDatabaseObjectName(components[0]))
        throw DBEngineRequestFactoryError("GET ALL ROWS: Invalid database name");

    boost::trim(components[1]);
    if (!isValidDatabaseObjectName(components[1]))
        throw DBEngineRequestFactoryError("GET ALL ROWS: Invalid table name");

    boost::to_upper(components[0]);
    boost::to_upper(components[1]);

    return std::make_shared<requests::GetAllRowsRestRequest>(
            std::move(components[0]), std::move(components[1]));
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createGetSingleRowRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name(), boost::is_any_of("."));
    if (components.size() != 2)
        throw DBEngineRequestFactoryError("GET SINGLE ROW: Invalid object name");

    boost::trim(components[0]);
    if (!isValidDatabaseObjectName(components[0]))
        throw DBEngineRequestFactoryError("GET SINGLE ROW: Invalid database name");

    boost::trim(components[1]);
    if (!isValidDatabaseObjectName(components[1]))
        throw DBEngineRequestFactoryError("GET SINGLE ROW: Invalid table name");

    if (msg.object_id() == 0)
        throw DBEngineRequestFactoryError("GET SINGLE ROW: Invalid object ID");

    boost::to_upper(components[0]);
    boost::to_upper(components[1]);

    return std::make_shared<requests::GetSingleRowRestRequest>(
            std::move(components[0]), std::move(components[1]), msg.object_id());
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createPostRowsRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg, siodb::io::InputStream& input)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name(), boost::is_any_of("."));
    if (components.size() != 2) throw DBEngineRequestFactoryError("POST ROWS: Invalid object name");

    boost::trim(components[0]);
    if (!isValidDatabaseObjectName(components[0]))
        throw DBEngineRequestFactoryError("POST ROWS: Invalid database name");

    boost::trim(components[1]);
    if (!isValidDatabaseObjectName(components[1]))
        throw DBEngineRequestFactoryError("POST ROWS: Invalid table name");

    LOG_DEBUG << "DBEngineRestRequestFactory::createPostRowsRequest: Reading and parsing JSON "
                 "payload";
    std::unordered_map<unsigned, std::string> columnNames;
    std::vector<std::vector<std::pair<unsigned, Variant>>> values;

    RowDataJsonSaxParser jsonParser(std::numeric_limits<std::size_t>::max(), columnNames, values);
    io::ChunkedInputStream chunkedInput(input);
    try {
#ifdef SIODB_READ_JSON_PAYLOAD_IN_CHUNKS
        io::InputStreamStdStreamBuffer payloadInputStreamBuffer(chunkedInput, kReadJsonBufferSize);
#else
        stdext::buffer<char> payloadBuffer(kJsonBufferGrowStep);
        std::size_t storedPayloadSize = 0;
        std::uint64_t totalPayloadSize = 0;
        while (!chunkedInput.isEof()) {
            if (storedPayloadSize == payloadBuffer.size() - 1)
                payloadBuffer.resize(payloadBuffer.size() + kJsonBufferGrowStep);
            auto readLocation = payloadBuffer.data() + storedPayloadSize;
            const auto n =
                    chunkedInput.read(readLocation, payloadBuffer.size() - storedPayloadSize - 1);
            //DBG_LOG_DEBUG("DBEngineRestRequestFactory::createPostRowsRequest: Read "
            //              << n << " bytes of payload");
            if (n < 1) break;
            readLocation[n] = 0;
            if (totalPayloadSize <= m_maxJsonPayloadSize) storedPayloadSize += n;
            totalPayloadSize += n;
        }
        if (totalPayloadSize > m_maxJsonPayloadSize)
            throw DBEngineRequestFactoryError("JSON payload is too long");
        //DBG_LOG_DEBUG("DBEngineRestRequestFactory::createPostRowsRequest: Payload: "
        //              << payloadBuffer.data());
        io::MemoryStdStreamBuffer payloadInputStreamBuffer(payloadBuffer.data(), storedPayloadSize);
#endif
        nlohmann::json::sax_parse(std::istream(&payloadInputStreamBuffer),
                static_cast<nlohmann::json_sax<nlohmann::json>*>(&jsonParser));
    } catch (JsonParserError& ex) {
#ifdef SIODB_READ_JSON_PAYLOAD_IN_CHUNKS
        chunkedInput.setStopReadingAfterCurrentChunkFinished();
        chunkedInput.skip(chunkedInput.getRemainingBytesInChunk());
#endif
        std::ostringstream err;
        err << "JSON payload parse error: " << ex.what();
        throw DBEngineRequestFactoryError(err.str());
    } catch (std::exception& ex) {
        chunkedInput.setStopReadingAfterCurrentChunkFinished();
        chunkedInput.skip(chunkedInput.getRemainingBytesInChunk());
        LOG_ERROR << "DBEngineRestRequestFactory::createPostRowsRequest: " << ex.what();
        throw DBEngineRequestFactoryError("JSON payload parse error: other error");
    }

    boost::to_upper(components[0]);
    boost::to_upper(components[1]);

    return std::make_shared<requests::PostRowsRestRequest>(std::move(components[0]),
            std::move(components[1]), std::move(columnNames), std::move(values));
}

}  // namespace siodb::iomgr::dbengine::parser
