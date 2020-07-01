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
 * @param os Output stream for SQL data.
 * @throw runtime_error In case of connection error.
 */
void dumpAllDatabases(siodb::io::IoBase& connectionIo, std::ostream& os);

/**
 * Dumps table definitions and data from the specified database.
 * @param connectionIo Connection.
 * @param os Output stream for SQL data.
 * @param databaseName Specified database to dump.
 * @throw runtime_error In case of connection error.
 */
void dumpDatabase(
        siodb::io::IoBase& connectionIo, std::ostream& os, const std::string& databaseName);

}  // namespace siodb::siocli
