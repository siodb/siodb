// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserDatabase.h"

namespace siodb::iomgr::dbengine {

UserDatabase::UserDatabase(Instance& instance, std::string&& name, const std::string& cipherId,
        BinaryValue&& cipherKey, std::size_t tableCacheCapacity,
        std::optional<std::string>&& description)
    : Database(instance, std::move(name), cipherId, std::move(cipherKey), tableCacheCapacity,
            std::move(description))
{
    // Indicate that database is initialized
    createInitializationFlagFile();
}

UserDatabase::UserDatabase(
        Instance& instance, const DatabaseRecord& dbRecord, std::size_t tableCacheCapacity)
    : Database(instance, dbRecord, tableCacheCapacity)
{
}

}  // namespace siodb::iomgr::dbengine
