// Copyright (C) 2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DBEngineRequestType.h"

// CRT headers
#include <cstddef>

namespace siodb::iomgr::dbengine::requests {

namespace {

const char* g_DBEngineRequestTypeNames[static_cast<std::size_t>(DBEngineRequestType::kMax) + 1] = {
        // Stub request type
        "None",

        // SQL requests
        "Select",
        "Insert",
        "Update",
        "Delete",
        "BeginTransaction",
        "kCommitTransaction",
        "RollbackTransaction",
        "Savepoint",
        "Release",
        "AttachDatabase",
        "DetachDatabase",
        "CreateDatabase",
        "DropDatabase",
        "RenameDatabase",
        "SetDatabaseAttributes",
        "UseDatabase",
        "CreateTable",
        "DropTable",
        "RenameTable",
        "SetTableAttributes",
        "AddColumn",
        "DropColumn",
        "RenameColumn",
        "RedefineColumn",
        "CreateIndex",
        "DropIndex",
        "CreateUser",
        "SetUserAttributes",
        "DropUser",
        "AddUserAccessKey",
        "DropUserAccessKey",
        "SetUserAccessKeyAttributes",
        "RenameUserAccessKey",
        "AddUserToken",
        "CreateUserToken",
        "DropUserToken",
        "SetUserTokenAttributes",
        "RenameUserToken",
        "CheckUserToken",
        "GrantPermissionsForInstance",
        "RevokePermissionsForInstance",
        "GrantPermissionsForDatabase",
        "RevokePermissionsForDatabase",
        "GrantPermissionsForTable",
        "RevokePermissionsForTable",
        "GrantPermissionsForView",
        "RevokePermissionsForView",
        "GrantPermissionsForIndex",
        "RevokePermissionsForIndex",
        "GrantPermissionsForTrigger",
        "RevokePermissionsForTrigger",
        "ShowDatabases",
        "ShowTables",
        "DescribeTable",

        // REST requests
        "RestGetDatabases",
        "RestGetTables",
        "RestGetAllRows",
        "RestGetSingleRow",
        "RestGetSqlQueryRows",
        "RestPostRows",
        "RestDeleteRow",
        "RestPatchRow",

        // MAX constant
        "Max",
};

}  // anonymous namespace

const char* getDBEngineRequestTypeName(DBEngineRequestType requestType) noexcept
{
    return g_DBEngineRequestTypeNames[static_cast<std::size_t>(requestType)];
}

}  // namespace siodb::iomgr::dbengine::requests
