// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "Database.h"

namespace siodb::iomgr::dbengine {

/** User database */
class UserDatabase : public Database {
public:
    /**
     * Initializes object of class UserDatabase for new database.
     * @param instance Instance object.
     * @param uuid Database UUID.
     * @param name Database name.
     * @param cipherId Cipher ID used for encryption of this database.
     * @param cipherKey Key used for encryption of this database.
     * @param description Database description.
     * @param maxTableCount Maximum number of tables.
     * @param dataDirectoryMustExist Indicates that data directory must exist.
     */
    UserDatabase(Instance& instance, const Uuid& uuid, std::string&& name,
            const std::string& cipherId, BinaryValue&& cipherKey,
            std::optional<std::string>&& description, std::uint32_t maxTableCount,
            bool dataDirectoryMustExist);

    /**
     * Initializes object of class UserDatabase for existing database.
     * @param instance Instance object.
     * @param dbRecord Database record.
     */
    UserDatabase(Instance& instance, const DatabaseRecord& dbRecord);
};

}  // namespace siodb::iomgr::dbengine
