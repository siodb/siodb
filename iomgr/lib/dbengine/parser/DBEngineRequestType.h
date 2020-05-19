// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr::dbengine::requests {

/** Database engine request types */
enum class DBEngineRequestType {
    kNone,
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
    kUseDatabase,
    kCreateTable,
    kDropTable,
    kRenameTable,
    kAddColumn,
    kDropColumn,
    kAlterColumn,
    kCreateIndex,
    kDropIndex,
    kCreateUser,
    kAlterUser,
    kDropUser,
    kAddUserAccessKey,
    kDropUserAccessKey,
    kAlterUserAccessKey,
    kShowDatabases,
};

}  // namespace siodb::iomgr::dbengine::requests
