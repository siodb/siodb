// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RequestHandler.h"

// Project headers
#include <siodb-generated/iomgr/lib/messages/IOManagerMessageId.h>
#include "RequestHandlerSharedConstants.h"
#include "VariantOutput.h"
#include "../ThrowDatabaseError.h"
#include "../UserAccessKey.h"
#include "../UserToken.h"

// Common project headers
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_wrap/filesystem_wrapper.h>
#include <siodb/iomgr/shared/dbengine/DatabaseObjectName.h>
#include <siodb/iomgr/shared/dbengine/parser/CommonConstants.h>

// STL headers
#include <numeric>

namespace siodb::iomgr::dbengine {

void RequestHandler::executeGrantPermissionsForTableRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::GrantPermissionsForTableRequest& request)
{
    response.set_has_affected_row_count(false);

    const std::string& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (databaseName != requests::kAllObjectsName && !isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    if (request.m_table != requests::kAllObjectsName && !isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_user))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidUserName, request.m_user);

    m_instance.grantTablePermissionsToUser(request.m_user, databaseName, request.m_table,
            request.m_permissions, request.m_withGrantOption, m_currentUserId);

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

void RequestHandler::executeRevokePermissionsForTableRequest(
        iomgr_protocol::DatabaseEngineResponse& response,
        const requests::RevokePermissionsForTableRequest& request)
{
    response.set_has_affected_row_count(false);

    const std::string& databaseName =
            request.m_database.empty() ? m_currentDatabaseName : request.m_database;
    if (databaseName != requests::kAllObjectsName && !isValidDatabaseObjectName(databaseName))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidDatabaseName, request.m_database);

    if (request.m_table != requests::kAllObjectsName && !isValidDatabaseObjectName(request.m_table))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidTableName, request.m_table);

    if (!isValidDatabaseObjectName(request.m_user))
        throwDatabaseError(IOManagerMessageId::kErrorInvalidUserName, request.m_user);

    m_instance.revokeTablePermissionsFromUser(
            request.m_user, databaseName, request.m_table, request.m_permissions, m_currentUserId);

    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, m_connection);
}

void RequestHandler::executeShowPermissionsRequest(iomgr_protocol::DatabaseEngineResponse& response,
        const requests::ShowPermissionsRequest& request)
{
    response.set_has_affected_row_count(false);
    const auto currentUser = m_instance.findUserChecked(m_currentUserId);
    UserPtr inspectedUser;
    if (request.m_user) {
        if (*request.m_user != requests::kAllObjectsName
                && !isValidDatabaseObjectName(*request.m_user)) {
            throwDatabaseError(IOManagerMessageId::kErrorInvalidUserName, *request.m_user);
        }
        inspectedUser = m_instance.findUserChecked(*request.m_user);
        if (inspectedUser->getId() != m_currentUserId) {
            const UserPermissionKey permissionKey(
                    0, DatabaseObjectType::kUser, inspectedUser->getId());
            if (!currentUser->hasPermissions(
                        permissionKey, kShowPermissionsPermissionMask, false)) {
                throwDatabaseError(IOManagerMessageId::kErrorPermissionDenied);
            }
        }
    } else {
        inspectedUser = currentUser;
    }

    struct SortableUserPermissionKey {
        std::string m_database;
        DatabaseObjectType m_objectType;
        std::string m_object;
        std::string m_objectTypeName;

        bool operator<(const SortableUserPermissionKey& other) const noexcept
        {
            return m_database < other.m_database
                           ? true
                           : (m_objectType < other.m_objectType ? true : m_object < other.m_object);
        }
    };

    std::map<SortableUserPermissionKey, UserPermissionDataEx> sortedPermissions;
    if (!inspectedUser->isSuperUser()) {
        for (const auto& [key, value] : inspectedUser->getGrantedPermissions()) {
            SortableUserPermissionKey sortableKey;

            const bool allDatabases = key.getDatabaseId() == 0;
            DatabasePtr database;
            if (allDatabases) {
                sortableKey.m_database = '*';
            } else {
                try {
                    database = m_instance.findDatabaseChecked(key.getDatabaseId());
                    sortableKey.m_database = database->getName();
                } catch (DatabaseError& ex) {
                    sortableKey.m_database =
                            stdext::concat("<UNAVAILABLE DATABASE #", key.getDatabaseId(), '>');
                }
            }

            sortableKey.m_objectType = key.getObjectType();
            switch (sortableKey.m_objectType) {
                case DatabaseObjectType::kInstance: {
                    sortableKey.m_object = boost::uuids::to_string(m_instance.getUuid());
                    break;
                }

                case DatabaseObjectType::kDatabase: {
                    if (key.getObjectId() == 0) {
                        sortableKey.m_object = '*';
                    } else if (key.getObjectId() > std::numeric_limits<std::uint32_t>::max()) {
                        sortableKey.m_object =
                                stdext::concat("<UNKNOWN DATABASE #", key.getObjectId(), '>');
                    } else {
                        try {
                            sortableKey.m_object =
                                    m_instance
                                            .findDatabaseChecked(
                                                    static_cast<std::uint32_t>(key.getObjectId()))
                                            ->getName();
                        } catch (DatabaseError& ex) {
                            sortableKey.m_object = stdext::concat(
                                    "<UNAVAILABLE DATABASE #", key.getObjectId(), '>');
                        }
                    }
                    break;
                }

                case DatabaseObjectType::kTable: {
                    if (key.getObjectId() == 0) {
                        sortableKey.m_object = '*';
                    } else if (allDatabases
                               || key.getObjectId() > std::numeric_limits<std::uint32_t>::max()) {
                        sortableKey.m_object =
                                stdext::concat("<UNKNOWN TABLE #", key.getObjectId(), '>');
                    } else {
                        try {
                            sortableKey.m_object =
                                    database->findTableChecked(key.getObjectId())->getName();
                        } catch (DatabaseError& ex) {
                            sortableKey.m_object =
                                    stdext::concat("<UNAVAILABLE TABLE #", key.getObjectId(), '>');
                        }
                    }
                    break;
                }

                case DatabaseObjectType::kColumn: {
                    if (key.getObjectId() == 0) {
                        sortableKey.m_object = '*';
                    } else {
                        sortableKey.m_object = stdext::concat("<COLUMN #", key.getObjectId(), '>');
                    }
                    break;
                }

                case DatabaseObjectType::kIndex: {
                    if (key.getObjectId() == 0) {
                        sortableKey.m_object = '*';
                    } else {
                        sortableKey.m_object = stdext::concat("<INDEX #", key.getObjectId(), '>');
                    }
                    break;
                }

                case DatabaseObjectType::kConstraint: {
                    if (key.getObjectId() == 0) {
                        sortableKey.m_object = '*';
                    } else {
                        sortableKey.m_object =
                                stdext::concat("<CONSTRAINT #", key.getObjectId(), '>');
                    }
                    break;
                }

                case DatabaseObjectType::kTrigger: {
                    if (key.getObjectId() == 0) {
                        sortableKey.m_object = '*';
                    } else {
                        sortableKey.m_object = stdext::concat("<TRIGGER #", key.getObjectId(), '>');
                    }
                    break;
                }

                case DatabaseObjectType::kProcedure: {
                    if (key.getObjectId() == 0) {
                        sortableKey.m_object = '*';
                    } else {
                        sortableKey.m_object =
                                stdext::concat("<PROCEDURE #", key.getObjectId(), '>');
                    }
                    break;
                }

                case DatabaseObjectType::kFunction: {
                    if (key.getObjectId() == 0) {
                        sortableKey.m_object = '*';
                    } else {
                        sortableKey.m_object =
                                stdext::concat("<FUNCTION #", key.getObjectId(), '>');
                    }
                    break;
                }

                case DatabaseObjectType::kUser: {
                    if (allDatabases) {
                        if (key.getObjectId() == 0) {
                            sortableKey.m_object = '*';
                        } else if (key.getObjectId() > std::numeric_limits<std::uint32_t>::max()) {
                            sortableKey.m_object =
                                    stdext::concat("<UNKNOWN USER #", key.getObjectId(), '>');
                        } else {
                            try {
                                sortableKey.m_object =
                                        m_instance
                                                .findUserChecked(static_cast<std::uint32_t>(
                                                        key.getObjectId()))
                                                ->getName();
                            } catch (DatabaseError& ex) {
                                sortableKey.m_object = stdext::concat(
                                        "<UNAVAILABLE USER #", key.getObjectId(), '>');
                            }
                        }
                    } else {
                        sortableKey.m_object =
                                stdext::concat("<UNKNOWN USER #", key.getObjectId(), '>');
                    }
                    break;
                }

                case DatabaseObjectType::kUserAccessKey: {
                    if (allDatabases) {
                        if (key.getObjectId() == 0) {
                            sortableKey.m_object = '*';
                        } else {
                            try {
                                const auto userAccessKey =
                                        m_instance.findUserAccessKeyChecked(key.getObjectId());
                                sortableKey.m_object =
                                        stdext::concat(userAccessKey.first->getName(), '.',
                                                userAccessKey.second->getName());
                            } catch (DatabaseError& ex) {
                                sortableKey.m_object = stdext::concat(
                                        "<UNAVAILABLE USER ACCESS KEY #", key.getObjectId(), '>');
                            }
                        }
                    } else {
                        sortableKey.m_object = stdext::concat(
                                "<UNKNOWN USER ACCESS KEY #", key.getObjectId(), '>');
                    }
                    break;
                }

                case DatabaseObjectType::kUserToken: {
                    if (allDatabases) {
                        if (key.getObjectId() == 0) {
                            sortableKey.m_object = '*';
                        } else {
                            try {
                                const auto userToken =
                                        m_instance.findUserTokenChecked(key.getObjectId());
                                sortableKey.m_object = stdext::concat(userToken.first->getName(),
                                        ',', userToken.second->getName());
                            } catch (DatabaseError& ex) {
                                sortableKey.m_object = stdext::concat(
                                        "<UNAVAILABLE USER TOKEN KEY #", key.getObjectId(), '>');
                            }
                        }
                    } else {
                        sortableKey.m_object =
                                stdext::concat("<UNKNOWN USER TOKEN KEY #", key.getObjectId(), '>');
                    }
                    break;
                }

                case DatabaseObjectType::kMax: {
                    sortableKey.m_object = stdext::concat("<MAX_OBJECT_", key.getObjectId(), '>');
                    break;
                }
            }

            try {
                sortableKey.m_objectTypeName = getDatabaseObjectTypeName(sortableKey.m_objectType);
            } catch (std::invalid_argument&) {
                sortableKey.m_objectTypeName = stdext::concat(
                        "<UNKNOWN_TYPE_", static_cast<int>(sortableKey.m_objectType), '>');
            }

            sortedPermissions.emplace(std::move(sortableKey), value);
        }
    }

    addColumnToResponse(response, "USER", COLUMN_DATA_TYPE_TEXT);
    addColumnToResponse(response, "DATABASE", COLUMN_DATA_TYPE_TEXT);
    addColumnToResponse(response, "OBJECT_TYPE", COLUMN_DATA_TYPE_TEXT);
    addColumnToResponse(response, "OBJECT_NAME", COLUMN_DATA_TYPE_TEXT);
    addColumnToResponse(response, "PERMISSION", COLUMN_DATA_TYPE_TEXT);
    addColumnToResponse(response, "GRANT_OPTION", COLUMN_DATA_TYPE_BOOL);

    utils::DefaultErrorCodeChecker errorChecker;
    protobuf::StreamOutputStream rawOutput(m_connection, errorChecker);
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, rawOutput);

    protobuf::ExtendedCodedOutputStream codedOutput(&rawOutput);

    std::vector<Variant> values;
    values.reserve(6);
    if (inspectedUser->isSuperUser()) {
        // Special stuff comes for super user
        values.push_back(inspectedUser->getName());  // USER
        values.push_back("*");  // DATABASE
        values.push_back("*");  // OBJECT_TYPE
        values.push_back("*");  // OBJECT_NAME
        values.push_back("*");  // PERMISSION
        values.push_back(true);  // GRANT_OPTION
        const auto rowSize = std::accumulate(values.cbegin(), values.cend(), std::uint64_t(0),
                [](std::uint64_t a, const Variant& b) noexcept {
                    return a + getVariantSerializedSize(b);
                });
        codedOutput.Write(rowSize);
        for (std::size_t i = 0; i < values.size(); ++i) {
            writeVariant(values[i], codedOutput);
            rawOutput.CheckNoError();
        }
    } else {
        // Normal user, list all found permissions
        for (const auto& e : sortedPermissions) {
            const auto permissions = e.second.getPermissions();
            const auto effectiveGrantOptions = e.second.getEffectiveGrantOptions();
            for (int i = 0; i < static_cast<int>(PermissionType::kMax); ++i) {
                const auto bitMask = std::uint64_t(1) << i;
                if (permissions & bitMask) {
                    values.clear();
                    values.push_back(inspectedUser->getName());  // USER
                    values.push_back(e.first.m_database);  // DATABASE
                    values.push_back(e.first.m_objectTypeName);  // OBJECT_TYPE
                    values.push_back(e.first.m_object);  // OBJECT_NAME
                    values.push_back(getPermissionTypeName(i));  // PERMISSION
                    values.push_back((effectiveGrantOptions & bitMask) != 0);  // GRANT_OPTION
                    const auto rowSize = std::accumulate(values.cbegin(), values.cend(),
                            std::uint64_t(0), [](std::uint64_t a, const Variant& b) noexcept {
                                return a + getVariantSerializedSize(b);
                            });
                    codedOutput.Write(rowSize);
                    for (std::size_t i = 0; i < values.size(); ++i) {
                        writeVariant(values[i], codedOutput);
                        rawOutput.CheckNoError();
                    }
                }
            }
        }
    }
    codedOutput.WriteVarint64(kNoMoreRows);
    rawOutput.CheckNoError();
}

}  // namespace siodb::iomgr::dbengine
