// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Restcli.h"

// Common project headers
#include <siodb/common/config/SiodbDefs.h>
#include <siodb/common/config/SiodbVersion.h>
#include <siodb/common/io/BufferedChunkedOutputStream.h>
#include <siodb/common/io/ChunkedInputStream.h>
#include <siodb/common/io/FDStream.h>
#include <siodb/common/net/NetConstants.h>
#include <siodb/common/net/TcpConnection.h>
#include <siodb/common/options/SiodbOptions.h>
#include <siodb/common/protobuf/ProtobufMessageIO.h>
#include <siodb/common/stl_ext/system_error_ext.h>
#include <siodb/common/utils/Debug.h>
#include <siodb/common/utils/StartupActions.h>

// STL headers
#include <fstream>
#include <iostream>
#include <sstream>

// System headers
#include <fcntl.h>
#include <signal.h>

// Boost headers
#include <boost/algorithm/string.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

// Protobuf messages
#include <siodb/common/proto/IOManagerProtocol.pb.h>

extern "C" int restcliMain(int argc, char** argv)
{
    // Must be called very first!
    siodb::utils::performCommonStartupActions();

    DEBUG_SYSCALLS_LIBRARY_GUARD;

    //const bool stdinIsTerminal = ::isatty(::fileno(stdin)) == 1;

    try {
        // Declare options
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()("host,H",
                boost::program_options::value<std::string>()->default_value(siodb::net::kLocalhost),
                "Server host name or IP address");
        desc.add_options()("port,p",
                boost::program_options::value<int>()->default_value(
                        siodb::config::kDefaultIOManagerIpv4RestPortNumber),
                "Server port");
        desc.add_options()("method,m",
                boost::program_options::value<std::string>()->default_value("GET"),
                "Request method");
        desc.add_options()("request-id,r",
                boost::program_options::value<std::uint64_t>()->default_value(1),
                "Request identifier");
        desc.add_options()("object-type,t",
                boost::program_options::value<std::string>()->default_value("ROW"), "Object type");
        desc.add_options()("object-name,n",
                boost::program_options::value<std::string>()->default_value(""), "Object name");
        desc.add_options()("object-id,i",
                boost::program_options::value<std::uint64_t>()->default_value(0),
                "Object identifier");
        desc.add_options()("user,u",
                boost::program_options::value<std::string>()->default_value("root"), "User name");
        desc.add_options()("token,T",
                boost::program_options::value<std::string>()->default_value(""),
                "User token (takes precendece over token file)");
        desc.add_options()("token-file,F",
                boost::program_options::value<std::string>()->default_value(""), "User token file");
        desc.add_options()("payload,P",
                boost::program_options::value<std::string>()->default_value(""),
                "Payload string (takes precendece over payload file)");
        desc.add_options()("file,f",
                boost::program_options::value<std::string>()->default_value(""), "Payload file");
        desc.add_options()("help,h", "Produce help message");
        desc.add_options()("nologo", "Do not print logo");
        desc.add_options()("debug,d", "Print debug messages");

        // Parse options
        boost::program_options::variables_map vm;
        boost::program_options::store(
                boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        // Handle help options
        if (argc == 1 || vm.count("help") > 0) {
            siodb::rest_client::printLogo();
            std::cout << '\n' << desc << std::endl;
            return 0;
        }

        // Handle options
        siodb::rest_client::RestClientParameters params;
        params.m_host = vm["host"].as<std::string>();
        params.m_port = vm["port"].as<int>();
        params.m_method = vm["method"].as<std::string>();
        boost::to_upper(params.m_method);
        params.m_requestId = vm["request-id"].as<std::uint64_t>();
        params.m_objectType = vm["object-type"].as<std::string>();
        boost::to_upper(params.m_objectType);
        params.m_objectName = vm["object-name"].as<std::string>();
        params.m_objectId = vm["object-id"].as<std::uint64_t>();
        params.m_user = vm["user"].as<std::string>();
        params.m_token = vm["token"].as<std::string>();
        params.m_tokenFile = vm["token-file"].as<std::string>();
        params.m_payload = vm["payload"].as<std::string>();
        params.m_payloadFile = vm["file"].as<std::string>();
        params.m_noLogo = vm.count("nologo") > 0;
        params.m_printDebugMessages = vm.count("debug") > 0;

        // Ignore SIGPIPE
        signal(SIGPIPE, SIG_IGN);

        // Print logo
        if (!params.m_noLogo) siodb::rest_client::printLogo();

        // Ignore SIGPIPE
        signal(SIGPIPE, SIG_IGN);

        // Execute request
        return siodb::rest_client::executeRestRequest(params, std::cout);

    } catch (std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '.' << std::endl;
        return 2;
    }

    return 0;
}

namespace siodb::rest_client {

void printLogo()
{
    std::cout << "Siodb IO Manager REST Protocol Client v." << SIODB_VERSION_MAJOR << '.'
              << SIODB_VERSION_MINOR << '.' << SIODB_VERSION_PATCH
#ifdef _DEBUG
              << " (debug build)"
#endif
              << "\nCompiled on " << __DATE__ << ' ' << __TIME__ << "\nCopyright (C) "
              << SIODB_COPYRIGHT_YEARS << " Siodb GmbH. All rights reserved." << std::endl;
}

int executeRestRequest(const RestClientParameters& params, std::ostream& os)
{
    // Open connection
    if (params.m_printDebugMessages) {
        std::cerr << "debug: Connecting to " << params.m_host << ':' << params.m_port << std::endl;
    }
    int fd = net::openTcpConnection(params.m_host, params.m_port);
    if (fd < 0) stdext::throw_system_error("Can't establish connection to IO Manager");
    io::FDStream connection(fd, true);
    if (params.m_printDebugMessages) {
        std::cerr << "debug: Connected to " << params.m_host << ':' << params.m_port
                  << ": fd=" << fd << std::endl;
    }

    // Fill requst message
    iomgr_protocol::DatabaseEngineRestRequest restRequest;
    restRequest.set_request_id(params.m_requestId);

    iomgr_protocol::RestVerb verb = iomgr_protocol::GET;
    if (params.m_method == "POST")
        verb = iomgr_protocol::POST;
    else if (params.m_method == "PATCH" || params.m_method == "PUT")
        verb = iomgr_protocol::PATCH;
    else if (params.m_method == "DELETE")
        verb = iomgr_protocol::DELETE;
    else if (params.m_method != "GET")
        throw std::invalid_argument("Invalid request method: " + params.m_method);
    restRequest.set_verb(verb);

    iomgr_protocol::DatabaseObjectType objectType = iomgr_protocol::DATABASE;
    if (params.m_objectType == "TABLE" || params.m_objectType == "TABLES")
        objectType = iomgr_protocol::TABLE;
    else if (params.m_objectType == "ROW" || params.m_objectType == "ROWS")
        objectType = iomgr_protocol::ROW;
    else if (!(params.m_objectType == "DATABASE" || params.m_objectType == "DATABASES"
                     || params.m_objectType == "DB")) {
        throw std::invalid_argument("Invalid object type: " + params.m_objectType);
    }
    restRequest.set_object_type(objectType);

    restRequest.set_object_name(params.m_objectName);
    if (params.m_objectId != 0) restRequest.set_object_id(params.m_objectId);

    restRequest.set_user_name(params.m_user);
    if (!params.m_token.empty())
        restRequest.set_token(params.m_token);
    else {
        if (params.m_printDebugMessages) {
            std::cerr << "debug: Loading token from file " << params.m_tokenFile << std::endl;
        }
        std::ifstream ifs(params.m_tokenFile);
        if (!ifs.is_open()) throw std::runtime_error("Can't open token file " + params.m_tokenFile);
        std::string token;
        if (!std::getline(ifs, token))
            throw std::runtime_error("Can't read token from file " + params.m_tokenFile);
        restRequest.set_token(std::move(token));
    }

    if (params.m_printDebugMessages) {
        std::cerr << "debug: Sending request message (DatabaseEngineRestRequest)..." << std::endl;
    }
    protobuf::writeMessage(
            protobuf::ProtocolMessageType::kDatabaseEngineRestRequest, restRequest, connection);

    const utils::DefaultErrorCodeChecker defaultErrorCodeChecker;
    protobuf::StreamInputStream input(connection, defaultErrorCodeChecker);

    // Send payload
    bool hadRequestPayload = false;
    std::uint32_t expectedResponseCount = 1;
    if (verb == iomgr_protocol::POST || verb == iomgr_protocol::PATCH) {
        constexpr std::uint64_t kExpectedResponseId = 0;
        expectedResponseCount = 2;
        hadRequestPayload = true;

        // Wait for authentication response
        iomgr_protocol::DatabaseEngineResponse response;
        if (params.m_printDebugMessages) {
            std::cerr << "debug: Receiving response message (DatabaseEngineResponse) [1]..."
                      << std::endl;
        }
        protobuf::readMessage(
                siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, input);
        if (params.m_printDebugMessages) {
            std::cerr << "\ndebug: "
                         "==================================================================="
                         "====\n"
                      << "debug: Expecting response: requestId=" << params.m_requestId
                      << " responseId=" << kExpectedResponseId
                      << "\ndebug: Received response: requestId=" << response.request_id()
                      << " responseId=" << response.response_id()
                      << "\ndebug: "
                         "==================================================================="
                         "====\n"
                      << std::flush;
        }

        // Check request ID
        if (response.request_id() != params.m_requestId) {
            std::ostringstream err;
            err << "Wrong request ID in the server response: expecting " << params.m_requestId
                << ", but received " << response.request_id();
            throw std::runtime_error(err.str());
        }

        // Check reponse ID
        if (response.response_id() != kExpectedResponseId) {
            std::ostringstream err;
            err << "Wrong response ID in the server response: expecting " << kExpectedResponseId
                << ", but received " << response.response_id();
            throw std::runtime_error(err.str());
        }

        // Check reponse count
        if (response.message_size() == 0 && response.response_count() != expectedResponseCount) {
            std::ostringstream err;
            err << "Wrong response count in the server response: expecting "
                << expectedResponseCount << ", but received " << response.response_count();
            throw std::runtime_error(err.str());
        }

        // Print "freetext" messages
        const int freeTextMessageCount = response.freetext_message_size();
        if (freeTextMessageCount > 0) {
            os << '\n';
            for (int i = 0; i < freeTextMessageCount; ++i) {
                os << "Server: " << response.freetext_message(i) << '\n';
            }
            os << std::endl;  // newline + flush stream at this point
        }

        bool errorOccurred = false;
        // Print messages
        const int messageCount = response.message_size();
        if (messageCount > 0) {
            os << std::endl;
            for (int i = 0; i < messageCount; ++i) {
                const auto& message = response.message(i);
                os << "Status " << message.status_code() << ": " << message.text() << '\n';
                errorOccurred |= message.status_code() != 0;
            }
            os << std::endl;  // newline + flush stream at this point
        }
        if (errorOccurred) return 3;

        // Send payload
        constexpr std::size_t kChunkSize = 65536;
        io::BufferedChunkedOutputStream chunkedOutput(kChunkSize, connection);
        if (!params.m_payload.empty()) {
            if (params.m_printDebugMessages) {
                std::cerr << "debug: Sending payload:\ndebug: ===== PAYLOAD ("
                          << params.m_payload.length() << " bytes) ======";
                std::istringstream iss(params.m_payload);
                std::string s;
                while (std::getline(iss, s)) {
                    std::cerr << "\ndebug: " << s;
                }
                std::cerr << "\ndebug: ===== END OF PAYLOAD ======" << std::endl;
            }
            if (static_cast<std::size_t>(
                        chunkedOutput.write(params.m_payload.c_str(), params.m_payload.length()))
                    != params.m_payload.length()) {
                stdext::throw_system_error("Failed to send payload");
            }
        } else if (!params.m_payloadFile.empty()) {
            if (params.m_printDebugMessages) {
                std::cerr << "debug: Opening payload file " << params.m_payloadFile << std::endl;
            }
            io::FDStream payloadFile(::open(params.m_payload.c_str(), O_RDONLY), true);
            if (!payloadFile.isValid()) stdext::throw_system_error("Can't open payload file");
            std::uint8_t buffer[4096];
            std::ptrdiff_t n;
            while (true) {
                n = payloadFile.read(buffer, sizeof(buffer));
                if (params.m_printDebugMessages) {
                    std::cerr << "debug: Reading payload: " << n << " bytes" << std::endl;
                }
                if (n < 1) break;
                if (params.m_printDebugMessages) {
                    std::cerr << "debug: Sending payload: " << n << " bytes" << std::endl;
                }
                if (chunkedOutput.write(buffer, n) != n)
                    stdext::throw_system_error("Failed to send payload");
            }
            if (n < 0) stdext::throw_system_error("Failed to read payload file");
        }
        if (params.m_printDebugMessages) {
            std::cerr << "debug: Flushing pending payload from the buffer: "
                      << chunkedOutput.getDataSize() << " bytes..." << std::endl;
        }
        if (chunkedOutput.close() != 0)
            stdext::throw_system_error("Failed to send last part of the payload");
    }

    // Read server response
    iomgr_protocol::DatabaseEngineResponse response;
    if (params.m_printDebugMessages) {
        std::cerr << "debug: Receiving response message (DatabaseEngineResponse) [2]..."
                  << std::endl;
    }
    protobuf::readMessage(
            siodb::protobuf::ProtocolMessageType::kDatabaseEngineResponse, response, input);

    const std::uint32_t expectedResponseId = hadRequestPayload ? 1 : 0;

    if (params.m_printDebugMessages) {
        std::cerr << "\ndebug: ==================================================================="
                     "====\n"
                  << "debug: Expecting response: requestId=" << params.m_requestId
                  << " responseId=" << expectedResponseId
                  << "\ndebug: Received response: requestId=" << response.request_id()
                  << " responseId=" << response.response_id()
                  << "\ndebug: ==================================================================="
                     "====\n"
                  << std::flush;
    }

    // Check request ID
    if (response.request_id() != params.m_requestId) {
        std::ostringstream err;
        err << "Wrong request ID in the server response: expecting " << params.m_requestId
            << ", but received " << response.request_id();
        throw std::runtime_error(err.str());
    }

    // Check reponse ID
    if (response.response_id() != expectedResponseId) {
        std::ostringstream err;
        err << "Wrong response ID in the server response: expecting " << expectedResponseId
            << ", but received " << response.response_id();
        throw std::runtime_error(err.str());
    }

    // Capture response count
    if (!hadRequestPayload) {
        auto responseCount = response.response_count();
        if (responseCount == 0) responseCount = 1;
        if (params.m_printDebugMessages) {
            os << "debug: Number of responses:" << responseCount << std::endl;
        }
        if (responseCount != expectedResponseCount) {
            std::ostringstream err;
            err << "Wrong response count in the server response: expecting "
                << expectedResponseCount << ", but received " << responseCount;
            throw std::runtime_error(err.str());
        }
    }

    // Print "freetext" messages
    const int freeTextMessageCount = response.freetext_message_size();
    if (freeTextMessageCount > 0) {
        os << '\n';
        for (int i = 0; i < freeTextMessageCount; ++i) {
            os << "Server: " << response.freetext_message(i) << '\n';
        }
        os << std::endl;  // newline + flush stream at this point
    }

    bool errorOccurred = false;
    // Print messages
    const int messageCount = response.message_size();
    if (messageCount > 0) {
        os << std::endl;
        for (int i = 0; i < messageCount; ++i) {
            const auto& message = response.message(i);
            os << "Status " << message.status_code() << ": " << message.text() << '\n';
            errorOccurred |= message.status_code() != 0;
        }
        os << std::endl;  // newline + flush stream at this point
    }
    if (errorOccurred) return 3;

    // Receive JSON payload
    if (params.m_printDebugMessages) {
        std::cerr << "debug: Receiving payload..." << std::endl;
    }

    std::ostringstream payloadStream;

    io::ChunkedInputStream chunkedInput(input);
    char buffer[4096];
    buffer[sizeof(buffer) - 1] = 0;
    while (!chunkedInput.isEof()) {
        const auto n = chunkedInput.read(buffer, sizeof(buffer) - 1);
        if (params.m_printDebugMessages) {
            std::cerr << "debug: Received payload: " << n << " bytes" << std::endl;
        }
        if (n < 1) break;
        buffer[n] = 0;
        payloadStream << buffer;
    }
    chunkedInput.close();

    os << payloadStream.str() << std::endl;

    return 0;
}

}  // namespace siodb::rest_client
