// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine::requests {

/** Database engine request types */
enum class DBEngineRequestType {
    // Stub request type
    kNone,

    // SQL requests
    kSelect,
    kInsert,
    kUpdate,
    kDelete,
    kBeginTransaction,
    kCommitTransaction,
    kRollbackTransaction,
    kSavepoint,
    kRelease,
    kAttachDatabase,
    kDetachDatabase,
    kCreateDatabase,
    kDropDatabase,
    kRenameDatabase,
    kSetDatabaseAttributes,
    kUseDatabase,
    kCreateTable,
    kDropTable,
    kRenameTable,
    kSetTableAttributes,
    kAddColumn,
    kDropColumn,
    kRenameColumn,
    kRedefineColumn,
    kCreateIndex,
    kDropIndex,
    kCreateUser,
    kSetUserAttributes,
    kDropUser,
    kAddUserAccessKey,
    kDropUserAccessKey,
    kSetUserAccessKeyAttributes,
    kRenameUserAccessKey,
    kAddUserToken,
    kCreateUserToken,
    kDropUserToken,
    kSetUserTokenAttributes,
    kRenameUserToken,
    kCheckUserToken,
    kShowDatabases,

    // REST requests
    kRestGetDatabases,
    kRestGetTables,
    kRestGetAllRows,
    kRestGetSingleRow,
    kRestPostRows,
    kRestDeleteRow,
    kRestPatchRow,
};

}  // namespace siodb::iomgr::dbengine::requests
