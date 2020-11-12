// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
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
#include <siodb/common/crypto/TlsClient.h>
#include <siodb/common/io/FDStream.h>
#include <siodb/common/io/FileIO.h>
#include <siodb/common/net/NetConstants.h>
#include <siodb/common/net/TcpConnection.h>
#include <siodb/common/net/UnixConnection.h>
#include <siodb/common/options/SiodbInstance.h>
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/string_builder.h>
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

namespace {
const std::string kDefaultIdentityFile = siodb::utils::getHomeDir() + "/.ssh/id_rsa";
const char* kFirstLinePrompt = "\033[1msiocli> \033[0m";
const char* kSubsequentLinePrompt = "\033[1m      > \033[0m";
const char* kCommentStart = "--";
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
        const auto identityFile = vm["identity-file"].as<std::string>();
        if (vm.count("command") > 0) {
            auto command = vm["command"].as<std::string>();
            boost::trim(command);
            params.m_command = std::make_unique<std::string>(std::move(command));
        }
        params.m_stdinIsTerminal = stdinIsTerminal;
        params.m_echoCommandsWhenNotOnATerminal = vm.count("no-echo") == 0;
        params.m_verifyCertificates = vm.count("verify-certificates") > 0;
        if (vm.count("output-file") > 0) params.m_outputFile = vm["output-file"].as<std::string>();
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
        std::cerr << "Error: " << ex.what() << '.' << std::endl;
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
    std::uint64_t requestId = 1;
    bool hasMoreInput = true;
    std::unique_ptr<siodb::io::InputOutputStream> connection;
    std::unique_ptr<siodb::crypto::TlsClient> tlsClient;
    const bool needPrompt = params.m_stdinIsTerminal;
    const bool singleCommand = static_cast<bool>(params.m_command);
    ServerConnectionInfo serverConnectionInfo;

    do {
        try {
            auto singleWordCommand = SingleWordCommandType::kUnknownCommand;
            std::string commandHolder;
            std::string* command = &commandHolder;
            if (singleCommand) {
                command = params.m_command.get();
            } else {
                std::ostringstream text;
                std::size_t textLength = 0;
                char textLastChar = '\0';

                // Read command text, possibly multiline.
                // Multiline command-text must end with a semicolon.
                std::size_t lineNo = 0;
                if (needPrompt) std::cout << '\n' << kFirstLinePrompt << std::flush;
                do {
                    std::string line;
                    if (!std::getline(std::cin, line)) {
                        hasMoreInput = false;
                        break;
                    }

                    boost::trim(line);
                    if (line.empty()) {
                        if (needPrompt) {
                            std::cout
                                    << (textLength == 0 ? kFirstLinePrompt : kSubsequentLinePrompt)
                                    << std::flush;
                        }
                        continue;
                    }

                    // Do not send comments to siodb
                    if (boost::starts_with(line, kCommentStart)) {
                        if (needPrompt) {
                            std::cout << (textLength ? kFirstLinePrompt : kSubsequentLinePrompt)
                                      << std::flush;
                        }
                        continue;
                    }

                    if (line.back() != ';') {
                        if (needPrompt) std::cout << kSubsequentLinePrompt << std::flush;
                    }

                    if (textLength > 0) {
                        text << '\n';
                        ++textLength;
                    }

                    text << line;
                    textLength += line.length();
                    if (!line.empty()) textLastChar = line.back();

                    ++lineNo;
                    if (lineNo == 1) {
                        auto command1 = boost::to_lower_copy(line);
                        // Remove whitespace before ';' and ';' itself
                        boost::trim_right_if(command1, boost::is_any_of("\t\v\f ;"));
                        singleWordCommand = decodeSingleWordCommand(command1);
                    }
                } while (singleWordCommand == SingleWordCommandType::kUnknownCommand
                         && (textLength == 0 || textLastChar != ';'));
                commandHolder = text.str();
            }  // read command

            // Maybe echo command
            if (!params.m_stdinIsTerminal && params.m_echoCommandsWhenNotOnATerminal)
                std::cout << '\n' << *command << '\n' << std::endl;

            // Handle single-word commands
            switch (singleWordCommand) {
                case SingleWordCommandType::kExit: {
                    std::cout << "Bye" << std::endl;
                    return 0;
                }
                case SingleWordCommandType::kHelp: {
                    std::cout << "Type SQL query with ';' symbol in the end. This query will be "
                                 "sent to the Siodb server.\n"
                                 "Example: SELECT * FROM <table_name>;\n"
                                 "Type exit to stop SioDB client.\n"
                              << std::flush;
                    continue;
                }
                case SingleWordCommandType::kUnknownCommand: break;
            }

            // Connect to server
            if (!connection || !connection->isValid()) {
                if (params.m_instance.empty()) {
                    auto connectionFd = siodb::net::openTcpConnection(params.m_host, params.m_port);
                    std::cout << "Connected to " << params.m_host << ':' << params.m_port
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
                    std::cout << "Connected to SIODB instance " << params.m_instance << " at "
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
                        params.m_exitOnError, params.m_printDebugMessages);
            }
        } catch (std::exception& ex) {
            std::cerr << "Error: " << ex.what() << '.' << std::endl;
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
        throw std::runtime_error(stdext::string_builder()
                                 << "User identity key file " << path << " of size " << st.st_size
                                 << " bytes is longer than allowed maximum size "
                                 << siodb::kMaxUserAccessKeySize << " bytes");
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
    return SingleWordCommandType::kUnknownCommand;
}

}  // namespace siodb::sql_client
