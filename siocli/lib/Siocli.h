// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>
#include <string>

namespace siodb::sql_client {

/** SQL client parameters. */
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

    /** Command to execute */
    std::unique_ptr<std::string> m_command;

    /** Output file */
    std::string m_outputFile;

    /** Database name to export. Empty string means export all databases */
    std::string m_exportObjectName;

    /** Indication whether client should use encryption or not */
    bool m_encryption = false;

    /** Indicates that STDIN is attached to a terminal */
    bool m_stdinIsTerminal = true;

    /** Echo commands if not on a terminal */
    bool m_echoCommandsWhenNotOnATerminal = true;

    /** Indicates that siocli should verify Siodb certificates */
    bool m_verifyCertificates = false;

    /** Indicates that siocli should not print logo */
    bool m_noLogo = false;

    /** Indicates the debug messages should be printed */
    bool m_printDebugMessages = false;

    /** Indicates that readline should be used for reading commands */
    bool m_useReadline = false;
};

/** Prints logo. */
void printLogo();

/**
 * Runs command line prompt.
 * @param params Client parameters.
 * @return Exit code.
 */
int commandPrompt(const ClientParameters& params);

/**
 * Exporting SQL dump of current instance.
 * @param params Client parameters.
 * @return Exit code.
 */
int exportSqlDump(const ClientParameters& params);

/**
 * Loads user's private key from a file.
 * @param path File path.
 * @return User's private key.
 */
std::string loadUserIdentityKey(const char* path);

/** Single word command types */
enum class SingleWordCommandType {
    kUnknownCommand,
    kExit,
    kHelp,
};

/**
 * Attempts to decode a single-word command.
 * @param command Command text.
 * @param length Command length.
 * @return Decoded command code or SingleWordCommandType::kUnknownCommand if input string
 *         doesn't represent a single word command.
 */
SingleWordCommandType decodeSingleWordCommand(const std::string& command) noexcept;

}  // namespace siodb::sql_client
