// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DBEngineRestRequestFactory.h"

// Project headers
#include "DBEngineRequestFactoryError.h"
#include "DBEngineRestRequest.h"
#include "DBEngineSqlRequestFactory.h"
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
                case iomgr_protocol::SQL: return createSqlQueryRequest(msg);
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

        case iomgr_protocol::DELETE: {
            switch (msg.object_type()) {
                case iomgr_protocol::ROW: return createDeleteRowRequest(msg);
                default: {
                    throw DBEngineRequestFactoryError(
                            "REST request: Invalid or unsupported object type for the DELETE "
                            "request");
                }
            }
        }

        case iomgr_protocol::PUT: {
            if (input == nullptr) {
                throw std::runtime_error(
                        "DBEngineRestRequestFactory::createRestRequest(): Missing input stream, "
                        "it is required to create PUT request");
            }
            switch (msg.object_type()) {
                case iomgr_protocol::ROW: return createPatchRowRequest(msg, *input);
                default: {
                    throw DBEngineRequestFactoryError(
                            "REST request: Invalid or unsupported object type for the PUT "
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
            switch (msg.object_type()) {
                case iomgr_protocol::ROW: return createPatchRowRequest(msg, *input);
                default: {
                    throw DBEngineRequestFactoryError(
                            "REST request: Invalid or unsupported object type for the PATCH "
                            "request");
                }
            }
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
    if (!isValidDatabaseObjectName(msg.object_name_or_query()))
        throw DBEngineRequestFactoryError("GET TABLES: Invalid database name");
    return std::make_shared<requests::GetTablesRestRequest>(
            boost::to_upper_copy(msg.object_name_or_query()));
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createGetAllRowsRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name_or_query(), boost::is_any_of("."));
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
    boost::split(components, msg.object_name_or_query(), boost::is_any_of("."));
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

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createSqlQueryRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg)
{
    SqlParser parser(msg.object_name_or_query());
    parser.parse();

    const auto statementCount = parser.getStatementCount();
    if (statementCount != 1) {
        if (statementCount == 0) {
            throw DBEngineRequestFactoryError("SQL QUERY: No query");
        } else {
            std::ostringstream err;
            err << "SQL QUERY: Too many statements (" << statementCount << ')';
            throw DBEngineRequestFactoryError(err.str());
        }
    }

    DBEngineSqlRequestFactory factory(parser);
    auto request = factory.createSqlRequest();

    if (request->m_requestType == requests::DBEngineRequestType::kSelect) {
        auto query = std::shared_ptr<requests::SelectRequest>(
                request, static_cast<requests::SelectRequest*>(request.get()));
        return std::make_shared<requests::GetSqlQueryRowsRestRequest>(query);
    } else {
        throw DBEngineRequestFactoryError("SQL QUERY: Not a SELECT statement");
    }
}

namespace {

void parseJsonPayload(siodb::io::InputStream& input, std::size_t maxRowCount,
        std::size_t maxJsonPayloadSize, std::size_t jsonBufferGrowStep,
        std::unordered_map<unsigned, std::string>& columnNames,
        std::vector<std::vector<std::pair<unsigned, Variant>>>& values)
{
#ifdef SIODB_READ_JSON_PAYLOAD_IN_CHUNKS
    LOG_DEBUG << "DBEngineRestRequestFactory: Reading and parsing JSON payload";
#else
    LOG_DEBUG << "DBEngineRestRequestFactory: Reading JSON payload";
#endif
    RowDataJsonSaxParser jsonParser(maxRowCount, columnNames, values);
    io::ChunkedInputStream chunkedInput(input);
    try {
#ifdef SIODB_READ_JSON_PAYLOAD_IN_CHUNKS
        io::InputStreamStdStreamBuffer payloadInputStreamBuffer(chunkedInput, jsonBufferGrowStep);
#else
        stdext::buffer<char> payloadBuffer(jsonBufferGrowStep);
        std::size_t storedPayloadSize = 0;
        std::uint64_t totalPayloadSize = 0;
        while (!chunkedInput.isEof()) {
            if (storedPayloadSize == payloadBuffer.size() - 1)
                payloadBuffer.resize(payloadBuffer.size() + jsonBufferGrowStep);
            auto readLocation = payloadBuffer.data() + storedPayloadSize;
            const auto n =
                    chunkedInput.read(readLocation, payloadBuffer.size() - storedPayloadSize - 1);
            if (n < 1) break;
            readLocation[n] = 0;
            if (totalPayloadSize <= maxJsonPayloadSize) storedPayloadSize += n;
            totalPayloadSize += n;
        }
        LOG_DEBUG << "DBEngineRestRequestFactory: JSON payload read, length " << totalPayloadSize;
        if (totalPayloadSize > maxJsonPayloadSize) {
            std::ostringstream err;
            err << "JSON payload is too long: " << totalPayloadSize << " bytes, while max. "
                << maxJsonPayloadSize << " bytes is allowed";
            throw DBEngineRequestFactoryError(err.str());
        }
        io::MemoryStdStreamBuffer payloadInputStreamBuffer(payloadBuffer.data(), storedPayloadSize);
        LOG_DEBUG << "DBEngineRestRequestFactory: Parsing JSON payload";
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
        LOG_ERROR << "parseJsonPayload: " << ex.what();
        throw DBEngineRequestFactoryError("JSON payload parse error: other error");
    }
}

}  // namespace

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createPostRowsRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg, siodb::io::InputStream& input)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name_or_query(), boost::is_any_of("."));
    if (components.size() != 2) throw DBEngineRequestFactoryError("POST ROWS: Invalid object name");

    boost::trim(components[0]);
    if (!isValidDatabaseObjectName(components[0]))
        throw DBEngineRequestFactoryError("POST ROWS: Invalid database name");

    boost::trim(components[1]);
    if (!isValidDatabaseObjectName(components[1]))
        throw DBEngineRequestFactoryError("POST ROWS: Invalid table name");

    std::unordered_map<unsigned, std::string> columnNames;
    std::vector<std::vector<std::pair<unsigned, Variant>>> values;
    parseJsonPayload(input, std::numeric_limits<std::size_t>::max(), m_maxJsonPayloadSize,
            kJsonBufferGrowStep, columnNames, values);

    boost::to_upper(components[0]);
    boost::to_upper(components[1]);

    return std::make_shared<requests::PostRowsRestRequest>(std::move(components[0]),
            std::move(components[1]), std::move(columnNames), std::move(values));
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createDeleteRowRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name_or_query(), boost::is_any_of("."));
    if (components.size() != 2)
        throw DBEngineRequestFactoryError("DELETE ROW: Invalid object name");

    boost::trim(components[0]);
    if (!isValidDatabaseObjectName(components[0]))
        throw DBEngineRequestFactoryError("DELETE ROW: Invalid database name");

    boost::trim(components[1]);
    if (!isValidDatabaseObjectName(components[1]))
        throw DBEngineRequestFactoryError("DELETE ROW: Invalid table name");

    if (msg.object_id() == 0) throw DBEngineRequestFactoryError("DELETE ROW: Invalid object ID");

    boost::to_upper(components[0]);
    boost::to_upper(components[1]);

    return std::make_shared<requests::DeleteRowRestRequest>(
            std::move(components[0]), std::move(components[1]), msg.object_id());
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createPatchRowRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg, siodb::io::InputStream& input)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name_or_query(), boost::is_any_of("."));
    if (components.size() != 2) throw DBEngineRequestFactoryError("PATCH ROW: Invalid object name");

    boost::trim(components[0]);
    if (!isValidDatabaseObjectName(components[0]))
        throw DBEngineRequestFactoryError("PATCH ROW: Invalid database name");

    boost::trim(components[1]);
    if (!isValidDatabaseObjectName(components[1]))
        throw DBEngineRequestFactoryError("PATCH ROW: Invalid table name");

    std::unordered_map<unsigned, std::string> columnNames0;
    std::vector<std::vector<std::pair<unsigned, Variant>>> values0;
    parseJsonPayload(input, 1U, m_maxJsonPayloadSize, kJsonBufferGrowStep, columnNames0, values0);
    if (values0.empty()) throw DBEngineRequestFactoryError("PATCH ROW: Missing row data");

    boost::to_upper(components[0]);
    boost::to_upper(components[1]);

    std::vector<std::string> columnNames;
    std::vector<Variant> values;
    columnNames.reserve(values0.size());
    values.reserve(values0.size());
    for (auto& e : values0.front()) {
        columnNames.push_back(std::move(columnNames0[e.first]));
        values.push_back(std::move(e.second));
    }

    return std::make_shared<requests::PatchRowRestRequest>(std::move(components[0]),
            std::move(components[1]), msg.object_id(), std::move(columnNames), std::move(values));
}

}  // namespace siodb::iomgr::dbengine::parser
