// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include <siodb/common/io/IoBase.h>

// STL headers
#include <ostream>

namespace siodb::siocli {

/**
 * Dumps table definitions and data from all databases.
 * @param connectionIo Connection.
 * @param os Output stream.
 * @throw std::runtime_error if I/O error occurs.
 */
void dumpAllDatabases(siodb::io::IoBase& connectionIo, std::ostream& os);

/**
 * Dumps table definitions and data from the specified database.
 * @param connectionIo Connection.
 * @param os Output stream.
 * @param databaseName Database to dump.
 * @throw std::runtime_error if I/O error occurs.
 */
void dumpDatabase(
        siodb::io::IoBase& connectionIo, std::ostream& os, const std::string& databaseName);

}  // namespace siodb::siocli
