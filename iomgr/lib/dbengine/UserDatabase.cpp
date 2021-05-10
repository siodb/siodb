// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "UserDatabase.h"

namespace siodb::iomgr::dbengine {

UserDatabase::UserDatabase(Instance& instance, const Uuid& uuid, std::string&& name,
        const std::string& cipherId, BinaryValue&& cipherKey,
        std::optional<std::string>&& description, std::uint32_t maxTableCount,
        bool dataDirectoryMustExist)
    : Database(instance, uuid, std::move(name), cipherId, std::move(cipherKey),
            std::move(description), maxTableCount, dataDirectoryMustExist)
{
    // Indicate that database is initialized
    createInitializationFlagFile();
}

UserDatabase::UserDatabase(Instance& instance, const DatabaseRecord& dbRecord)
    : Database(instance, dbRecord)
{
}

}  // namespace siodb::iomgr::dbengine
