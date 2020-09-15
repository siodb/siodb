// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "RegistryRecordUuidChecker.h"

// Project headers
#include "CipherKeyRecord.h"
#include "ColumnDefinitionRecord.h"
#include "ColumnRecord.h"
#include "ColumnSetRecord.h"
#include "ConstraintDefinitionRecord.h"
#include "ConstraintRecord.h"
#include "DatabaseRecord.h"
#include "IndexRecord.h"
#include "TableRecord.h"
#include "UserPermissionRecord.h"
#include "UserRecord.h"

// STL headers
#include <unordered_set>

namespace siodb::iomgr::dbengine {

#define CHECK_RECORD(ClassName)                                                        \
    {                                                                                  \
        if (!uuids.insert(ClassName::s_classUuid).second)                              \
            throw std::runtime_error(std::string(#ClassName) + " UUID is not unique"); \
    }

void checkRegistryRecordUuids()
{
    std::unordered_set<Uuid> uuids;
    CHECK_RECORD(ColumnDefinitionConstraintRecord);
    CHECK_RECORD(ColumnDefinitionRecord);
    CHECK_RECORD(ColumnRecord);
    CHECK_RECORD(ColumnSetColumnRecord);
    CHECK_RECORD(ColumnSetRecord);
    CHECK_RECORD(ConstraintDefinitionRecord);
    CHECK_RECORD(ConstraintRecord);
    CHECK_RECORD(DatabaseRecord);
    CHECK_RECORD(IndexColumnRecord);
    CHECK_RECORD(IndexRecord);
    CHECK_RECORD(TableRecord);
    CHECK_RECORD(UserAccessKeyRecord);
    CHECK_RECORD(UserPermissionRecord);
    CHECK_RECORD(UserRecord);
    CHECK_RECORD(UserTokenRecord);
    CHECK_RECORD(CipherKeyRecord);
}

}  // namespace siodb::iomgr::dbengine
