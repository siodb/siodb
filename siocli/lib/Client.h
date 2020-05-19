// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Common project headers
#include <siodb/common/io/IoBase.h>

// CRT headers
#include <cstdint>

// STL headers
#include <ostream>
#include <string>
#include <system_error>

/**
 * Executes given command on the server and prints out results.
 * @param requestId Unique request identifier.
 * @param commandText A text of the command. This parameter will be moved.
 * @param connectionIo Connection IO.
 * @param os Output stream.
 * @param stopOnError Indicates that execution should stop on SQL error.
 * @throw std::system_error if I/O errors happened.
 * @throw std::runtime_error if protocol error happened.
 * @throw std::runtime_error if @ref stopOnError is true and SQL error happened.
 */
void executeCommandOnServer(std::uint64_t requestId, std::string&& commandText,
        siodb::io::IoBase& connectionIo, std::ostream& os, bool stopOnError);

/**
 * Authenticates user.
 * @param identityKey Indentity key of a user.
 * @param userName Name of a user.
 * @param connectionIo Connection IO.
 */
void authenticate(const std::string& identityKey, const std::string& userName,
        siodb::io::IoBase& connectionFd);
