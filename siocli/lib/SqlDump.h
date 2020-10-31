// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/io/InputOutputStream.h>

// STL headers
#include <ostream>

namespace siodb::siocli {

/**
 * Dumps table definitions and data from all databases.
 * @param connection Connection.
 * @param os Output stream.
 * @throw std::runtime_error if I/O error occurs.
 */
void dumpAllDatabases(siodb::io::InputOutputStream& connection, std::ostream& os);

/**
 * Dumps table definitions and data from the specified database.
 * @param connection Connection.
 * @param databaseName Database name.
 * @param os Output stream.
 * @throw std::runtime_error if I/O error occurs.
 */
void dumpSingleDatabase(siodb::io::InputOutputStream& connection, const std::string& databaseName,
        std::ostream& os);

/**
 * Dumps table definitions and data from the specified table.
 * @param connection Connection.
 * @param databaseName Database name.
 * @param tableName Table name.
 * @param os Output stream.
 * @throw std::runtime_error if I/O error occurs.
 */
void dumpSingleTable(siodb::io::InputOutputStream& connection, const std::string& databaseName,
        const std::string& tableName, std::ostream& os);

}  // namespace siodb::siocli
