// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

namespace {

/** Client parameters. */
struct ClientParameters {
    /** Instance name */
    std::string m_instance;

    /** Host address or name */
    std::string m_host;

    /** Host port */
    int m_port = 0;

    /** Should Siocli exit on error? */
    bool m_exitOnError = false;

    /** User name */
    std::string m_user;

    /** Path to identity key */
    std::string m_identityKey;

    /** Indication whether client should use encryption or not */
    bool m_encryption = false;

    /** Indicates that STDIN is attached to a terminal */
    bool m_stdinIsTerminal = true;

    /** Echo commands if not on a terminal */
    bool m_echoCommandsWhenNotOnATerminal = true;

    /** Indicates that siocli should verify siodb certificates */
    bool m_verifyCertificates = false;

    /** Database name to export. Empty string means export all databases */
    std::string m_exportDatabaseName;
};

/**
 * Loads user's private key from a file.
 * @param path File path.
 * @return User's private key.
 */
std::string loadUserIdentityKey(const char* path);

/**
 * Exporting SQL dump of current instance.
 * @param params Client parameters.
 * @return Exit code.
 */
int exportSqlDump(const ClientParameters& params);

/**
 * Runs command line prompt.
 * @param params Client parameters.
 * @return Exit code.
 */
int commandPrompt(const ClientParameters& params);

/** Single word command types */
enum class SingleWordCommandType {
    kUnknownCommand,
    kExit,
    kHelp,
};

/**
 * Attempts to decode a single-word command.
 * @param command Command text.
 * @return Decoded command code or SingleWordCommandType::kUnknownCommand if input string
 *         doesn't represent a single word command.
 */
SingleWordCommandType decodeSingleWordCommand(const std::string& command) noexcept;

}  // anonymous namespace
