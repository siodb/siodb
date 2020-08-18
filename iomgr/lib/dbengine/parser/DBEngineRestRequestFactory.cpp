// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DBEngineRestRequestFactory.h"

// Project headers
#include "DBEngineRestRequest.h"

// Common project headers
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>

// Boost headers
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace siodb::iomgr::dbengine::parser {

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createRequest(
        const iomgr_protocol::DatabaseEngineRestRequest& msg)
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
                default: throw std::invalid_argument("REST request: Invalid object type");
            }
        }
        case iomgr_protocol::POST: {
            switch (msg.object_type()) {
                //case iomgr_protocol::DATABASE: return createPOstDatabaseRequest(msg);
                //case iomgr_protocol::TABLE: return createPOstTableRequest(msg);
                case iomgr_protocol::ROW: return createPostRowsRequest(msg);
                default:
                    throw std::invalid_argument("REST request: Invalid or unsupported object type");
            }
            throw std::invalid_argument("Not supported yet");
        }
        case iomgr_protocol::PATCH: {
            // TODO: implement PATCH requests
            throw std::invalid_argument("Not supported yet");
        }
        case iomgr_protocol::DELETE: {
            // TODO: implement DELETE requests
            throw std::invalid_argument("Not supported yet");
        }
        default: throw std::invalid_argument("REST request: Invalid verb");
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
        throw std::invalid_argument("GET TABLES: Invalid database name");
    return std::make_shared<requests::GetTablesRestRequest>(std::string(msg.object_name()));
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createGetAllRowsRequest(
        [[maybe_unused]] const iomgr_protocol::DatabaseEngineRestRequest& msg)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name(), boost::is_any_of("."));
    if (components.size() != 2) throw std::invalid_argument("GET DATABASES: Invalid object name");

    boost::trim(components[0]);
    if (!isValidDatabaseObjectName(components[0]))
        throw std::invalid_argument("GET ALL ROWS: Invalid database name");

    boost::trim(components[1]);
    if (!isValidDatabaseObjectName(components[1]))
        throw std::invalid_argument("GET ALL ROWS: Invalid table name");

    return std::make_shared<requests::GetAllRowsRestRequest>(
            std::move(components[0]), std::move(components[1]));
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createGetSingleRowRequest(
        [[maybe_unused]] const iomgr_protocol::DatabaseEngineRestRequest& msg)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name(), boost::is_any_of("."));
    if (components.size() != 2) throw std::invalid_argument("GET DATABASES: Invalid object name");

    boost::trim(components[0]);
    if (!isValidDatabaseObjectName(components[0]))
        throw std::invalid_argument("GET SINGLE ROW: Invalid database name");

    boost::trim(components[1]);
    if (!isValidDatabaseObjectName(components[1]))
        throw std::invalid_argument("GET SINGLE ROW: Invalid table name");

    if (msg.object_id() == 0) throw std::invalid_argument("GET SINGLE ROW: Invalid object ID");

    return std::make_shared<requests::GetSingleRowRestRequest>(
            std::move(components[0]), std::move(components[1]), msg.object_id());
}

requests::DBEngineRequestPtr DBEngineRestRequestFactory::createPostRowsRequest(
        [[maybe_unused]] const iomgr_protocol::DatabaseEngineRestRequest& msg)
{
    std::vector<std::string> components;
    boost::split(components, msg.object_name(), boost::is_any_of("."));
    if (components.size() != 2) throw std::invalid_argument("GET DATABASES: Invalid object name");

    boost::trim(components[0]);
    if (!isValidDatabaseObjectName(components[0]))
        throw std::invalid_argument("GET SINGLE ROW: Invalid database name");

    boost::trim(components[1]);
    if (!isValidDatabaseObjectName(components[1]))
        throw std::invalid_argument("GET SINGLE ROW: Invalid table name");

    std::vector<std::vector<requests::ConstExpressionPtr>> values;

    // TODO parse values

    return std::make_shared<requests::PostRowsRestRequest>(
            std::move(components[0]), std::move(components[1]), std::move(values));
}

}  // namespace siodb::iomgr::dbengine::parser
