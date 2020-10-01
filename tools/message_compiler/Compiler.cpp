// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Compiler.h"

// Common project headers
#include <siodb/common/utils/FDGuard.h>

// STL headers
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <vector>

// Boost headers
#include <boost/algorithm/string/trim.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

static constexpr const char* kAllWhitespaces = " \t\n\r\f\v";

static const std::unordered_set<std::string> g_knownSeverities {
        "Debug",
        "Trace",
        "Info",
        "Warning",
        "Error",
        "Fatal",
};

int main(int argc, char** argv)
{
    std::cout << "Siodb Message Compiler v." << COMPILER_VERSION << ".\nCopyright (C) Siodb GmbH, "
              << COMPILER_COPYRIGHT_YEARS << ". All rights reserved.\n"
              << "Compiled on " << __DATE__ << ' ' << __TIME__ << std::endl;

    CompilerOptions options;
    if (!options.parse(argc, argv)) return 1;

    MessageContainer messages;
    if (!parseMessages(options, messages)) return 2;

    if (messages.empty()) {
        std::cerr << options.m_inputFileName << ":1:1: error: There are no messages." << std::endl;
        return 2;
    }

    bool result = false;
    switch (options.m_outputType) {
        case OutputType::Unknown: break;
        case OutputType::SymbolList: {
            result = writeSymbolListFile(messages, options);
            break;
        }
        case OutputType::MessageList: {
            result = writeMessageListFile(messages, options);
            break;
        }
        case OutputType::Header: {
            result = writeHeaderFile(messages, options);
            break;
        }
        case OutputType::Text: {
            result = writeTextFile(messages, options);
            break;
        }
    }

    return result ? 0 : 2;
}

bool parseMessages(const CompilerOptions& options, MessageContainer& messages)
{
    std::cout << "Parsing " << options.m_inputFileName << std::endl;

    std::ifstream ifs(options.m_inputFileName);
    if (!ifs.is_open()) {
        std::cerr << "Can't open input file " << options.m_inputFileName << std::endl;
        return false;
    }

    int step = 1;
    std::int64_t id = 1;
    std::string line;
    std::uint64_t lineNo = 0;
    std::uint64_t errorCount = 0;

    const auto& messagesById = messages.get<ByIdTag>();
    const auto& messagesBySymbol = messages.get<BySymbolTag>();

    while (true) {
        try {
            ++lineNo;
            if (!std::getline(ifs, line)) break;

            // Skip empty lines and comments
            boost::trim(line);
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;

            if (cmd == "ID") {
                iss >> id;
                if (!iss) throw std::runtime_error("Invalid ID");
                step = 1;
            } else if (cmd == "STEP") {
                iss >> step;
                if (!iss) throw std::runtime_error("Invalid step");
            } else if (cmd == "MSG" || cmd.find("MSG:") == 0) {
                Message msg;

                // Get explicit message ID of specified.
                if (cmd != "MSG") {
                    if (cmd.length() == 4) throw std::runtime_error("Missing explicit message ID");
                    const auto idStr = cmd.substr(4);
                    try {
                        id = std::stoll(idStr);
                    } catch (std::exception& ex) {
                        throw std::runtime_error("Invalid explicit message ID " + idStr);
                    }
                }

                // Check message ID duplication
                if (messagesById.count(id) > 0)
                    throw std::runtime_error("Duplicate message ID " + std::to_string(id));

                msg.m_id = id;
                id += step;

                // Get severity
                while (iss.good() && msg.m_severity.empty())
                    iss >> msg.m_severity;
                if (msg.m_severity.empty()) throw std::runtime_error("Severity not specified");

                // Validate severity
                if (g_knownSeverities.count(msg.m_severity) == 0) {
                    std::ostringstream err;
                    err << "Unknown severity '" << msg.m_severity << "'";
                    throw std::runtime_error(err.str());
                }

                // Get symbol
                while (iss.good() && msg.m_symbol.empty())
                    iss >> msg.m_symbol;
                if (msg.m_symbol.empty()) throw std::runtime_error("Symbol not specified");

                if (messagesBySymbol.count(msg.m_symbol) > 0)
                    throw std::runtime_error("Duplicate symbol '" + msg.m_symbol + "'");

                // Get text
                std::getline(iss, msg.m_text);
                boost::trim(msg.m_text);
                if (msg.m_text.empty()) throw std::runtime_error("Text not specified");

                if (options.m_validateMessageText) {
                    // Validate text
                    std::vector<int> parameterIndices;
                    auto pos = msg.m_text.find_first_of('%');
                    while (pos != std::string::npos) {
                        if (pos == msg.m_text.length() - 1)
                            throw std::runtime_error("Trailing % not closed");
                        const auto pos2 = msg.m_text.find_first_of('%', pos + 1);
                        if (pos2 == std::string::npos)
                            throw std::runtime_error("Last % not closed");
                        const auto len = pos2 - pos - 1;
                        if (len > 0) {
                            const auto parameterIndexStr = msg.m_text.substr(pos + 1, len);
                            int parameterIndex;
                            try {
                                parameterIndex = std::stoi(parameterIndexStr);
                                if (parameterIndex < 1)
                                    throw std::invalid_argument("Non-positive parameter index");
                            } catch (std::exception& ex) {
                                std::ostringstream err;
                                err << "Invalid parameter index in the parameter expansion #"
                                    << (parameterIndices.size() + 1) << ": " << ex.what();
                                throw std::runtime_error(err.str());
                            }
                            parameterIndices.push_back(parameterIndex);
                        }
                        if (pos2 == msg.m_text.length() - 1) break;
                        pos = msg.m_text.find_first_of('%', pos2 + 1);
                    }
                    if (!parameterIndices.empty()) {
                        std::sort(parameterIndices.begin(), parameterIndices.end());
                        parameterIndices.erase(
                                std::unique(parameterIndices.begin(), parameterIndices.end()),
                                parameterIndices.end());
                        for (std::size_t i = 0, n = parameterIndices.size(); i < n; ++i) {
                            const auto v = static_cast<int>(i + 1);
                            if (parameterIndices[i] == v) continue;
                            std::ostringstream err;
                            err << "Missing usage of parameter #" << v;
                            throw std::runtime_error(err.str());
                        }
                    }
                }

                // Add message to map
                messages.insert(std::move(msg));
            } else {
                std::ostringstream err;
                err << "Unknown command '" << cmd << "'";
                throw std::runtime_error(err.str());
            }
        } catch (std::exception& ex) {
            std::cerr << options.m_inputFileName << ':' << lineNo << ":1: error: " << ex.what()
                      << std::endl;
            ++errorCount;
        }
    }

    return errorCount == 0;
}

bool writeSymbolListFile(const MessageContainer& messages, const CompilerOptions& options)
{
    std::cout << "Writing ID list: " << options.m_outputFileName << std::endl;

    const auto tmpFileInfo = makeTemporaryFile();
    if (std::get<1>(tmpFileInfo) < 0) {
        std::cerr << "Can't open temporary file: " << std::strerror(std::get<2>(tmpFileInfo))
                  << std::endl;
        return false;
    }
    boost::iostreams::stream<boost::iostreams::file_descriptor_sink> ofs(
            std::get<1>(tmpFileInfo), boost::iostreams::close_handle);
    if (!ofs.is_open()) {
        std::cerr << "Can't open temporary file" << std::endl;
        return false;
    }

    for (const auto& message : messages.get<BySymbolTag>())
        ofs << message.m_symbol << '\n';

    ofs << std::flush;
    ofs.close();

    if (::rename(std::get<0>(tmpFileInfo).c_str(), options.m_outputFileName.c_str())) {
        const int errorCode = errno;
        std::cerr << "Can't rename temporary file " << std::get<0>(tmpFileInfo) << " into "
                  << options.m_outputFileName << ": " << std::strerror(errorCode) << std::endl;
        return false;
    }

    return true;
}

bool writeMessageListFile(const MessageContainer& messages, const CompilerOptions& options)
{
    std::cout << "Writing ID list file " << options.m_outputFileName << std::endl;

    const auto tmpFileInfo = makeTemporaryFile();
    if (std::get<1>(tmpFileInfo) < 0) {
        std::cerr << "Can't open temporary file: " << std::strerror(std::get<2>(tmpFileInfo))
                  << std::endl;
        return false;
    }
    boost::iostreams::stream<boost::iostreams::file_descriptor_sink> ofs(
            std::get<1>(tmpFileInfo), boost::iostreams::close_handle);
    if (!ofs.is_open()) {
        std::cerr << "Can't open temporary file" << std::endl;
        return false;
    }

    for (const auto& message : messages.get<BySymbolTag>())
        ofs << message.m_symbol << ' ' << message.m_text << '\n';

    ofs << std::flush;
    ofs.close();

    if (::rename(std::get<0>(tmpFileInfo).c_str(), options.m_outputFileName.c_str())) {
        const int errorCode = errno;
        std::cerr << "Can't rename temporary file " << std::get<0>(tmpFileInfo) << " into "
                  << options.m_outputFileName << ": " << std::strerror(errorCode) << std::endl;
        return false;
    }

    return true;
}

bool writeHeaderFile(const MessageContainer& messages, const CompilerOptions& options)
{
    std::cout << "Writing header file " << options.m_outputFileName << std::endl;

    const auto tmpFileInfo = makeTemporaryFile();
    if (std::get<1>(tmpFileInfo) < 0) {
        std::cerr << "Can't open temporary file: " << std::strerror(std::get<2>(tmpFileInfo))
                  << std::endl;
        return false;
    }
    boost::iostreams::stream<boost::iostreams::file_descriptor_sink> ofs(
            std::get<1>(tmpFileInfo), boost::iostreams::close_handle);
    if (!ofs.is_open()) {
        std::cerr << "Can't open temporary file" << std::endl;
        return false;
    }

    ofs << "// THIS FILE IS GENERATED AUTOMATICALLY. PLEASE DO NOT EDIT.\n// Copyright (C) Siodb "
           "GmbH, "
        << COMPILER_COPYRIGHT_YEARS << ". All rights reserved.\n\n";

    if (options.m_guardWithPragmaOnce) {
        ofs << "#pragma once\n\n";
    } else {
        ofs << "#ifndef " << options.m_guardSymbol << "\n#define " << options.m_guardSymbol
            << "\n\n";
    }
    ofs << "namespace " << options.m_namespaceName << " {\n\nenum class " << options.m_enumName
        << " : " << options.m_enumBaseType << " {\n";

    for (const auto& message : messages)
        ofs << "    k" << message.m_severity << message.m_symbol << " = " << message.m_id << ",\n";

    ofs << "};\n\n} // namespace " << options.m_namespaceName << '\n';

    if (!options.m_guardWithPragmaOnce) ofs << "\n#endif\n";

    ofs << std::flush;
    ofs.close();

    if (::rename(std::get<0>(tmpFileInfo).c_str(), options.m_outputFileName.c_str())) {
        const int errorCode = errno;
        std::cerr << "Can't rename temporary file " << std::get<0>(tmpFileInfo) << " into "
                  << options.m_outputFileName << ": " << std::strerror(errorCode) << std::endl;
        return false;
    }

    return true;
}

bool writeTextFile(const MessageContainer& messages, const CompilerOptions& options)
{
    std::cout << "Writing message text file " << options.m_outputFileName << std::endl;

    const auto tmpFileInfo = makeTemporaryFile();
    if (std::get<1>(tmpFileInfo) < 0) {
        std::cerr << "Can't open temporary file: " << std::strerror(std::get<2>(tmpFileInfo))
                  << std::endl;
        return false;
    }
    boost::iostreams::stream<boost::iostreams::file_descriptor_sink> ofs(
            std::get<1>(tmpFileInfo), boost::iostreams::close_handle);
    if (!ofs.is_open()) {
        std::cerr << "Can't open temporary file" << std::endl;
        return false;
    }

    ofs << "# THIS FILE IS GENERATED AUTOMATICALLY. PLEASE DO NOT EDIT.\n# Copyright (C) Siodb "
           "GmbH, "
        << COMPILER_COPYRIGHT_YEARS << ". All rights reserved.\n\n";

    for (const auto& message : messages)
        ofs << message.m_id << ", " << message.m_severity << ", " << message.m_text << '\n';

    ofs << std::flush;
    ofs.close();

    if (::rename(std::get<0>(tmpFileInfo).c_str(), options.m_outputFileName.c_str())) {
        const int errorCode = errno;
        std::cerr << "Can't rename temporary file " << std::get<0>(tmpFileInfo) << " into "
                  << options.m_outputFileName << ": " << std::strerror(errorCode) << std::endl;
        return false;
    }

    return true;
}

std::tuple<std::string, int, int> makeTemporaryFile()
{
    std::string tmpFilePath;
    const char* tmpDir = ::getenv("TMP");
    if (tmpDir && *tmpDir) {
        tmpFilePath = tmpDir;
        if (tmpFilePath.back() != '/') tmpFilePath += '/';
    } else
        tmpFilePath = "/tmp/";
    tmpFilePath += "siodb_message_compiler-XXXXXX";
    const int fd = ::mkstemp(tmpFilePath.data());
    const int errorCode = errno;
    return std::make_tuple(std::move(tmpFilePath), fd, errorCode);
}
