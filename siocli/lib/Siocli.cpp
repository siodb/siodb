// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Siocli.h"

// Project headers
#include "SqlClient.h"
#include "SqlDump.h"
#include "SqlQueryException.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/config/SiodbVersion.h>
#include <siodb/common/crt_ext/ct_string.h>
#include <siodb/common/crypto/TlsClient.h>
#include <siodb/common/io/FDStream.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/net/NetConstants.h>
#include <siodb/common/net/TcpConnection.h>
#include <siodb/common/net/UnixConnection.h>
#include <siodb/common/options/SiodbInstance.h>
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/deleter.h>
#include <siodb/common/stl_ext/sstream_ext.h>
#include <siodb/common/stl_ext/system_error_ext.h>
#include <siodb/common/sys/Syscalls.h>
#include <siodb/common/utils/CheckOSUser.h>
#include <siodb/common/utils/DebugMacros.h>
#include <siodb/common/utils/FDGuard.h>
#include <siodb/common/utils/StartupActions.h>

// Protobuf message headers
#include <siodb/common/proto/ClientProtocol.pb.h>

// CRT headers
#include <cerrno>
#include <cstring>

// STL headers
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

// System headers
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

// Readline
#include <readline/history.h>
#include <readline/readline.h>

namespace {
const std::string kDefaultIdentityFile = siodb::utils::getHomeDir() + "/.ssh/id_rsa";
constexpr const char* kFirstLinePrompt = "\033[1msiocli> \033[0m";
constexpr const char* kSubsequentLinePrompt = "\033[1m      > \033[0m";
constexpr const char kSqlDelimiter = ';';
constexpr const char* kCommentStart = "--";
constexpr const char* kMultilineCommentStart = "/*";
constexpr const char* kMultilineCommentEnd = "*/";
constexpr const char* kVariablePrefix = "var:";
constexpr auto kVariablePrefixLen = ::ct_strlen(kVariablePrefix);
}  // namespace

extern "C" int siocliMain(int argc, char** argv)
{
    // Must be called very first!
    siodb::utils::performCommonStartupActions();

    DEBUG_SYSCALLS_LIBRARY_GUARD;

    const bool stdinIsTerminal = ::isatty(::fileno(stdin)) == 1;

    try {
        const auto uid = ::geteuid();
        const auto osUserName = siodb::utils::getOsUserName(uid);

        // Declare options
        boost::program_options::options_description desc("Options");
        desc.add_options()("admin,a",
                boost::program_options::value<std::string>()->default_value(std::string()),
                "Connect to given instance in the admin mode");
        desc.add_options()("host,H",
                boost::program_options::value<std::string>()->default_value(siodb::net::kLocalhost),
                "Server host name or IP address");
        desc.add_options()("port,p",
                boost::program_options::value<int>()->default_value(
                        siodb::config::kDefaultIpv4PortNumber),
                "Server port");
        desc.add_options()(
                "keep-going,k", "Keep going if stdin is pipe or file and error occurred");
        desc.add_options()("identity-file,i",
                boost::program_options::value<std::string>()->default_value(kDefaultIdentityFile),
                "Identity file (client private key)");
        desc.add_options()("user,u",
                boost::program_options::value<std::string>()->default_value(osUserName),
                "User name");
        desc.add_options()("verify-certificates,V", "Verify certificates");
        desc.add_options()("plaintext,P", "Use plaintext connection");
        desc.add_options()("no-echo,N", "Do not commands if not on the terminal");
        desc.add_options()(
                "command,c", boost::program_options::value<std::string>(), "Command to execute");
        desc.add_options()("export,e", boost::program_options::value<std::string>(),
                "Export single database or table");
        desc.add_options()("export-all,E", "Export all databases");
        desc.add_options()("output-file,o", boost::program_options::value<std::string>(),
                "Output file for the exported data");
        desc.add_options()("use-readline,r", "Use readline library for console input");
        desc.add_options()("help,h", "Print help message");
        desc.add_options()("nologo", "Do not print logo");
        desc.add_options()("debug,d", "Print debug messages");

        // Parse options
        boost::program_options::variables_map vm;
        boost::program_options::store(
                boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        // Handle help options
        if (vm.count("help") > 0) {
            siodb::sql_client::printLogo();
            std::cout << '\n' << desc << std::endl;
            return 0;
        }

        const auto exportDatabase = vm.count("export") > 0;
        const auto exportAllDatabases = vm.count("export-all") > 0;
        if (exportDatabase && exportAllDatabases) {
            std::cerr << "Only one of '--export' and '--export-all' can be specified." << std::endl;
            return 1;
        }
        const auto exportSomething = exportDatabase || exportAllDatabases;

        // Handle options
        siodb::sql_client::ClientParameters params;
        params.m_instance = vm["admin"].as<std::string>();
        params.m_host = vm["host"].as<std::string>();
        params.m_port = vm["port"].as<int>();
        params.m_exitOnError = !stdinIsTerminal && vm.count("keep-going") == 0;
        params.m_user = vm["user"].as<std::string>();
        auto identityFile = vm["identity-file"].as<std::string>();
        if (identityFile.size() > kVariablePrefixLen
                && identityFile.substr(0, 4) == kVariablePrefix) {
            const auto varName = identityFile.substr(kVariablePrefixLen);
            const char* varValue = std::getenv(varName.c_str());
            if (varValue == nullptr) {
                std::ostringstream err;
                err << "Can't get identity file name from the variable '" << varName
                    << "': variable is undefined";
                throw std::runtime_error(err.str());
            }
            identityFile = varValue;
        }
        if (vm.count("command") > 0) {
            auto command = vm["command"].as<std::string>();
            boost::trim(command);
            params.m_command = std::make_unique<std::string>(std::move(command));
        }
        params.m_stdinIsTerminal = stdinIsTerminal;
        params.m_echoCommandsWhenNotOnATerminal = vm.count("no-echo") == 0;
        params.m_verifyCertificates = vm.count("verify-certificates") > 0;
        if (vm.count("output-file") > 0) params.m_outputFile = vm["output-file"].as<std::string>();
        params.m_useReadline = vm.count("use-readline") > 0;
        params.m_noLogo = vm.count("nologo") > 0;
        params.m_printDebugMessages = vm.count("debug") > 0;

        if (exportDatabase) {
            params.m_exportObjectName = vm["export"].as<std::string>();
            boost::to_upper(params.m_exportObjectName);
        }

        if (vm.count("plaintext") > 0)
            params.m_encryption = false;
        else {
            // Default simple client connection - secure
            // Default admin connection - non secure
            params.m_encryption = params.m_instance.empty();
        }

        params.m_identityKey = siodb::sql_client::loadUserIdentityKey(identityFile.c_str());

        if (exportSomething) params.m_noLogo = true;

        // Ignore SIGPIPE
        signal(SIGPIPE, SIG_IGN);

        // Print logo
        if (!params.m_noLogo) siodb::sql_client::printLogo();

        if (exportSomething) return exportSqlDump(params);

        // Execute command prompt
        return siodb::sql_client::commandPrompt(params);
    } catch (std::exception& ex) {
        std::cerr << "\nError: " << ex.what() << '.' << std::endl;
        return 2;
    }
}

namespace siodb::sql_client {

void printLogo()
{
    std::cout << "Siodb SQL Client v." << SIODB_VERSION_MAJOR << '.' << SIODB_VERSION_MINOR << '.'
              << SIODB_VERSION_PATCH
#ifdef _DEBUG
              << " (debug build)"
#endif
              << "\nCompiled on " << __DATE__ << ' ' << __TIME__ << "\nCopyright (C) "
              << SIODB_COPYRIGHT_YEARS << " Siodb GmbH. All rights reserved." << std::endl;
}

int commandPrompt(const ClientParameters& params)
{
    static bool readlineHistoryInitialized = false;
    if (params.m_stdinIsTerminal && params.m_useReadline && !readlineHistoryInitialized) {
        using_history();
        readlineHistoryInitialized = true;
    }

    std::uint64_t requestId = 1;
    bool hasMoreInput = true;
    std::unique_ptr<siodb::io::InputOutputStream> connection;
    std::unique_ptr<siodb::crypto::TlsClient> tlsClient;
    const bool singleCommand = static_cast<bool>(params.m_command);
    ServerConnectionInfo serverConnectionInfo;

    do {
        std::string commandHolder;
        std::string* command = &commandHolder;

        try {
            auto singleWordCommand = SingleWordCommandType::kUnknownCommand;
            if (singleCommand) {
                command = params.m_command.get();
            } else {
                std::ostringstream text;
                char textLastChar = '\0';

                // Read command text, possibly multiline.
                std::size_t lineNo = 0;
                const char* prompt = kFirstLinePrompt;
                bool lineStartsInStringValue = false;
                bool lineEndsInStringValue = false;
                bool isInStringValue = false;
                bool isIsolatedMultilineComment = false;
                do {
                    // Prompt style
                    if (params.m_stdinIsTerminal && lineNo == 0) std::cout << std::endl;
                    if (params.m_stdinIsTerminal && lineNo > 0) prompt = kSubsequentLinePrompt;

                    // Read line
                    std::string line;
                    if (params.m_stdinIsTerminal && params.m_useReadline) {
                        std::unique_ptr<char, stdext::free_deleter<char>> rltext;
                        while (!rltext)
                            rltext.reset(::readline(prompt));
                        line = rltext.get();
                        if (!line.empty()) {
                            add_history(rltext.get());
                        }
                    } else {
                        if (params.m_stdinIsTerminal) std::cout << prompt << std::flush;
                        if (!std::getline(std::cin, line)) {
                            hasMoreInput = false;
                            break;
                        }
                    }

                    // Detect string value state
                    lineStartsInStringValue = lineEndsInStringValue;
                    bool isEscaped = false;
                    for (char c : line) {
                        if (c == '\'' && !isEscaped) {
                            isInStringValue = !isInStringValue;
                        }
                        isEscaped = c == '\\';
                        lineEndsInStringValue = isInStringValue;
//#define DEBUG_COMMENT_REMOVAL
#ifdef DEBUG_COMMENT_REMOVAL
                        std::cout << "debug: char: " << c << " | isEscaped:" << isEscaped
                                  << " | isInStringValue:" << isInStringValue
                                  << " | lineStartsInStringValue:" << lineStartsInStringValue
                                  << " | lineEndsInStringValue:" << lineEndsInStringValue << '\n';
#endif
                    }

                    // Empty line considered as '\n'
                    if (line.empty() && lineNo == 0) break;

                    // Trim line when not in string value
                    if (!lineStartsInStringValue) {
                        boost::trim_left(line);
                    }
                    if (!lineEndsInStringValue) {
                        boost::trim_right(line);
                        if (!line.empty()) {
                            textLastChar = line.back();
                        }
                    }

#ifdef DEBUG_COMMENT_REMOVAL
                    std::cout << "debug: lineNo: " << lineNo << "value_for_iomgr_begin>" << line
                              << "<value_for_iomgr_end\n";
#endif

                    // Never send single line comment to the iomgr
                    if (boost::starts_with(line, kCommentStart) && lineNo == 0) {
                        break;
                    }
                    // Never send multilines comment to the iomgr
                    if (boost::starts_with(line, kMultilineCommentStart) && lineNo == 0)
                        isIsolatedMultilineComment = true;
                    if (isIsolatedMultilineComment
                            && boost::ends_with(line, kMultilineCommentEnd)) {
                        break;
                    }

                    if (!isIsolatedMultilineComment) {
                        if (lineNo > 0) text << '\n';
                        text << line;
                    }

                    ++lineNo;

                    if (lineNo == 1) {
                        auto command1 = boost::to_lower_copy(line);
                        // Remove whitespace before kSqlDelimiter and kSqlDelimiter itself
                        boost::trim_right_if(command1, boost::is_any_of("\t\v\f ;"));
                        singleWordCommand = decodeSingleWordCommand(command1);
                    }
                } while (singleWordCommand == SingleWordCommandType::kUnknownCommand
                         && textLastChar != kSqlDelimiter);
                commandHolder = text.str();
            }  // read command

            // Maybe echo command
            if (!params.m_stdinIsTerminal && params.m_echoCommandsWhenNotOnATerminal)
                std::cout << '\n' << *command << '\n' << std::endl;

            // Handle single-word commands
            switch (singleWordCommand) {
                case SingleWordCommandType::kExit: {
                    std::cout << '\n' << "Bye." << '\n' << std::endl;
                    return 0;
                }
                case SingleWordCommandType::kHelp: {
                    std::cout
                            << '\n'
                            << "Type SQL statements separated by '" << kSqlDelimiter << "':\n"
                            << '\n'
                            << "    Example 1: select * from sys_dummy;\n"
                            << "    Example 2: select * from sys_dummy; select * from sys_dummy;\n"
                            << '\n'
                            << "exit|quit: quits siocli." << '\n'
                            << std::flush;
                    continue;
                }
                case SingleWordCommandType::kUnknownCommand: break;
            }

            // Connect to server
            if (!connection || !connection->isValid()) {
                if (params.m_instance.empty()) {
                    auto connectionFd = siodb::net::openTcpConnection(params.m_host, params.m_port);
                    std::cout << '\n'
                              << "Connected to " << params.m_host << ':' << params.m_port
                              << std::endl;

                    if (params.m_encryption) {
                        tlsClient = std::make_unique<siodb::crypto::TlsClient>();

                        if (params.m_verifyCertificates) tlsClient->enableCertificateVerification();

                        auto tlsConnection = tlsClient->connectToServer(connectionFd);

                        auto x509Certificate = SSL_get_peer_certificate(tlsConnection->getSsl());
                        if (x509Certificate == nullptr)
                            throw siodb::crypto::OpenSslError("SSL_get_peer_certificate failed");

                        connection = std::move(tlsConnection);
                    } else
                        connection = std::make_unique<siodb::io::FDStream>(connectionFd, true);
                } else {
                    // Admin connection is always non-secure
                    const auto instanceSocketPath =
                            siodb::composeInstanceSocketPath(params.m_instance);
                    auto connectionFd = siodb::net::openUnixConnection(instanceSocketPath);
                    std::cout << "Connected to Siodb instance " << params.m_instance << " at "
                              << instanceSocketPath << " in the admin mode." << std::endl;
                    connection = std::make_unique<siodb::io::FDStream>(connectionFd, true);
                }
                authenticate(
                        params.m_identityKey, params.m_user, *connection, serverConnectionInfo);
                requestId = 1;
            }

            // Execute command
            if (!command->empty()) {
                executeCommandOnServer(requestId++, std::move(*command), *connection, std::cout,
                        singleCommand ? true : params.m_exitOnError, params.m_printDebugMessages);
            }
        } catch (std::exception& ex) {
            std::cerr << "\nError: " << ex.what() << '.' << std::endl;
            if (connection && connection->isValid()) {
                connection.reset();
                if (params.m_instance.empty()) {
                    std::cout << "Connection to " << params.m_host << ':' << params.m_port
                              << " closed." << std::endl;
                } else {
                    const auto instanceSocketPath =
                            siodb::composeInstanceSocketPath(params.m_instance);
                    std::cout << "Connection to " << instanceSocketPath << " closed." << std::endl;
                }
            }
            if (params.m_exitOnError) return 3;
            if (singleCommand) return 3;
        }
    } while (!singleCommand && hasMoreInput);
    return 0;
}

int exportSqlDump(const ClientParameters& params)
{
    std::unique_ptr<siodb::io::InputOutputStream> connection;
    std::unique_ptr<siodb::crypto::TlsClient> tlsClient;
    if (params.m_instance.empty()) {
        auto connectionFd = siodb::net::openTcpConnection(params.m_host, params.m_port);
        if (params.m_encryption) {
            tlsClient = std::make_unique<siodb::crypto::TlsClient>();
            if (params.m_verifyCertificates) tlsClient->enableCertificateVerification();
            auto tlsConnection = tlsClient->connectToServer(connectionFd);
            auto x509Certificate = SSL_get_peer_certificate(tlsConnection->getSsl());
            if (x509Certificate == nullptr)
                throw siodb::crypto::OpenSslError("SSL_get_peer_certificate failed");
            connection = std::move(tlsConnection);
        } else
            connection = std::make_unique<siodb::io::FDStream>(connectionFd, true);
    } else {
        // Admin connection is always non-secure
        const auto instanceSocketPath = siodb::composeInstanceSocketPath(params.m_instance);
        auto connectionFd = siodb::net::openUnixConnection(instanceSocketPath);
        connection = std::make_unique<siodb::io::FDStream>(connectionFd, true);
    }

    ServerConnectionInfo serverConnectionInfo;
    siodb::sql_client::authenticate(
            params.m_identityKey, params.m_user, *connection, serverConnectionInfo);

    std::ostream* out = &std::cout;
    std::unique_ptr<std::ofstream> ofs;
    if (!params.m_outputFile.empty()) {
        ofs = std::make_unique<std::ofstream>(params.m_outputFile);
        if (!ofs->is_open()) {
            std::cerr << "Can't open output file '" << params.m_outputFile << "'" << std::endl;
            return 2;
        }
        out = ofs.get();
    }

    try {
        const auto currentTime = std::time(nullptr);
        std::tm localTime, utcTime;
        ::localtime_r(&currentTime, &localTime);
        ::gmtime_r(&currentTime, &utcTime);
        *out << "-- Siodb SQL Dump\n"
             << "-- Hostname: " << params.m_host << '\n'
             << "-- Instance: "
             << (serverConnectionInfo.m_instanceName.empty() ? serverConnectionInfo.m_instanceName
                                                             : params.m_instance)
             << '\n'
             << "-- Timestamp: " << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << '\n'
             << "-- Timestamp (UTC): " << std::put_time(&utcTime, "%Y-%m-%d %H:%M:%S") << '\n';

        if (params.m_exportObjectName.empty()) {
            siodb::siocli::dumpAllDatabases(*connection, *out, params.m_printDebugMessages);
        } else {
            std::vector<std::string> names;
            boost::split(names, params.m_exportObjectName, boost::is_any_of("."));
            if (names.size() == 1) {
                siodb::siocli::dumpSingleDatabase(
                        *connection, names.front(), *out, params.m_printDebugMessages);
            } else if (names.size() == 2) {
                siodb::siocli::dumpSingleTable(
                        *connection, names[0], names[1], *out, params.m_printDebugMessages);
            } else {
                std::cerr << "Invalid database or table name: " << params.m_exportObjectName
                          << std::endl;
                return 2;
            }
        }
    } catch (const siodb::SqlQueryException& sqlQueryException) {
        std::cerr << sqlQueryException.what() << ":\n";
        for (const auto& errMsg : sqlQueryException.getErrors())
            std::cerr << "code: " << errMsg.status_code() << ", message: " << errMsg.text() << '\n';
        std::cerr << std::flush;
        return 2;
    }

    return 0;
}

std::string loadUserIdentityKey(const char* path)
{
    siodb::FDGuard fd(::open(path, O_RDONLY));
    if (!fd.isValidFd())
        stdext::throw_system_error("Can't open user identity key file " + std::string(path));

    struct stat st;
    if (::fstat(fd.getFD(), &st) < 0)
        stdext::throw_system_error("Can't stat user identity key file " + std::string(path));

    if (static_cast<std::size_t>(st.st_size) > siodb::kMaxUserAccessKeySize) {
        throw std::runtime_error(stdext::concat("User identity key file ", path, " of size ",
                st.st_size, " bytes is longer than allowed maximum size ",
                siodb::kMaxUserAccessKeySize, " bytes"));
    }

    std::string key;
    key.resize(st.st_size);
    if (::readExact(fd.getFD(), key.data(), key.size(), kIgnoreSignals) != key.size())
        stdext::throw_system_error("Can't read user identity key file " + std::string(path));

    return key;
}

SingleWordCommandType decodeSingleWordCommand(const std::string& command) noexcept
{
    if (command == "exit" || command == "quit")
        return SingleWordCommandType::kExit;
    else if (command == "help")
        return SingleWordCommandType::kHelp;
    else
        return SingleWordCommandType::kUnknownCommand;
}

}  // namespace siodb::sql_client
