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
#include <siodb/common/io/InputStreamStdStreamBuffer.h>
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
                case iomgr_protocol::DATABASE: return createGetDatabasesRequest(msg);
                case iomgr_protocol::TABLE: return createGetTablesRequest(msg);
                case iomgr_protocol::ROW: {
                    return msg.object_id() == 0 ? createGetAllRowsRequest(msg)
                                                : createGetSingleRowRequest(msg);
                }
                default: throw DBEngineRequestFactoryError("REST request: Invalid object type");
            }
        }
        case iomgr_protocol::POST: {
            if (input == nullptr) {
                throw std::runtime_error(
                        "DBEngineRestRequestFactory::createRestRequest(): Missing input stream, "
                        "it is required to create POST request");
            }
            switch (msg.object_type()) {
                //case iomgr_protocol::DATABASE: return createPostDatabaseRequest(msg);
                //case iomgr_protocol::TABLE: return createPostTableRequest(msg);
                case iomgr_protocol::ROW: return createPostRowsRequest(msg, *input);
                default:
                    throw DBEngineRequestFactoryError(
                            "REST request: Invalid or unsupported object type");
            }
            throw DBEngineRequestFactoryError("POST is not supported yet");
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
    return nullptr;
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createGetDatabasesRequest(
        [[maybe_unused]] const iomgr_protocol::DatabaseEngineRestRequest& msg)
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
        [[maybe_unused]] const iomgr_protocol::DatabaseEngineRestRequest& msg)
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
        [[maybe_unused]] const iomgr_protocol::DatabaseEngineRestRequest& msg)
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
        [[maybe_unused]] const iomgr_protocol::DatabaseEngineRestRequest& msg,
        siodb::io::InputStream& input)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name(), boost::is_any_of("."));
    if (components.size() != 2) throw DBEngineRequestFactoryError("POST ROWS: Invalid object name");

    boost::trim(components[0]);
    if (!isValidDatabaseObjectName(components[0]))
        throw DBEngineRequestFactoryError("GET SINGLE ROW: Invalid database name");

    boost::trim(components[1]);
    if (!isValidDatabaseObjectName(components[1]))
        throw DBEngineRequestFactoryError("GET SINGLE ROW: Invalid table name");

    std::unordered_map<unsigned, std::string> columnNames;
    std::vector<std::vector<std::pair<unsigned, requests::ConstExpressionPtr>>> values;
    try {
        RowDataJsonSaxParser jsonParser(kMaxPostRowCount, columnNames, values);
        io::InputStreamStdStreamBuffer streamBuffer(input, kJsonChunkSize);
        std::istream is(&streamBuffer);
        nlohmann::json::sax_parse(
                std::move(is), static_cast<nlohmann::json_sax<nlohmann::json>*>(&jsonParser));
    } catch (JsonParserError& ex) {
        std::ostringstream err;
        err << "JSON parsing error: " << ex.what();
        throw DBEngineRequestFactoryError(err.str());
    }

    boost::to_upper(components[0]);
    boost::to_upper(components[1]);

    return std::make_shared<requests::PostRowsRestRequest>(std::move(components[0]),
            std::move(components[1]), std::move(columnNames), std::move(values));
}

}  // namespace siodb::iomgr::dbengine::parser
