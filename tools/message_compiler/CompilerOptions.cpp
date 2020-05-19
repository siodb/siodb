// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "CompilerOptions.h"

// CRT headers
#include <cstring>

// STL headers
#include <iostream>

// Boost headers
#include <boost/algorithm/string.hpp>

bool CompilerOptions::parse(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (std::strcmp(arg, "-i") == 0 || std::strcmp(arg, "--input") == 0) {
            if (++i == argc) {
                std::cerr << "error: Missing input file name after option '" << arg << "'."
                          << std::endl;
                return false;
            }
            m_inputFileName = argv[i];
        } else if (std::strcmp(arg, "-o") == 0 || std::strcmp(arg, "--output") == 0) {
            if (++i == argc) {
                std::cerr << "error: Missing output file name after option '" << arg << "'."
                          << std::endl;
                return false;
            }
            m_outputFileName = argv[i];
        } else if (std::strcmp(arg, "-n") == 0 || std::strcmp(arg, "--namespace") == 0) {
            if (++i == argc) {
                std::cerr << "error: Missing namespace name after option '" << arg << "'."
                          << std::endl;
                return false;
            }
            m_namespaceName = argv[i];
            boost::replace_all(m_namespaceName, ".", "::");
        } else if (std::strcmp(arg, "-g") == 0 || std::strcmp(arg, "--guard") == 0) {
            if (++i == argc) {
                std::cerr << "error: Missing guard symbol name after option '" << arg << "'."
                          << std::endl;
                return false;
            }
            m_guardSymbol = argv[i];
        } else if (std::strcmp(arg, "-gp") == 0
                   || std::strcmp(arg, "--guard-with-pragma-once") == 0) {
            m_guardWithPragmaOnce = true;
        } else if (std::strcmp(arg, "-b") == 0 || std::strcmp(arg, "--enum-base") == 0) {
            if (++i == argc) {
                std::cerr << "error: Missing type name after option '" << arg << "'." << std::endl;
                return false;
            }
            m_enumBaseType = argv[i];
        } else if (std::strcmp(arg, "-e") == 0 || std::strcmp(arg, "--enum") == 0) {
            if (++i == argc) {
                std::cerr << "error: Missing enumeration class name after option '" << arg << "'."
                          << std::endl;
                return false;
            }
            m_enumName = argv[i];
        } else if (std::strcmp(arg, "-LS") == 0 || std::strcmp(arg, "--symlist") == 0) {
            m_outputType = OutputType::SymbolList;
        } else if (std::strcmp(arg, "-LM") == 0 || std::strcmp(arg, "--msglist") == 0) {
            m_outputType = OutputType::MessageList;
        } else if (std::strcmp(arg, "-H") == 0 || std::strcmp(arg, "--header") == 0) {
            m_outputType = OutputType::Header;
        } else if (std::strcmp(arg, "-T") == 0 || std::strcmp(arg, "--text") == 0) {
            m_outputType = OutputType::Text;
        } else if (std::strcmp(arg, "-VM") == 0
                   || std::strcmp(arg, "--validate-message-text") == 0) {
            m_validateMessageText = false;
        } else if (std::strcmp(arg, "-h") == 0 || std::strcmp(arg, "--help") == 0) {
            showHelp(argv[0]);
            exit(0);
        } else {
            std::cerr << "error: Unknown option: " << arg << std::endl;
            return false;
        }
    }

    if (m_inputFileName.empty()) {
        std::cerr << "error: Input file name not specified." << std::endl;
        return false;
    }

    if (m_outputFileName.empty()) {
        std::cerr << "error: Output file name not specified." << std::endl;
        return false;
    }

    if (m_outputType == OutputType::Unknown) {
        std::cerr << "error: Output type not specified." << std::endl;
        return false;
    }

    if (m_outputType == OutputType::Header) {
        if (m_enumName.empty()) {
            std::cerr << "error: Enumeration class name not specified." << std::endl;
            return false;
        }

        if (!m_guardWithPragmaOnce && m_guardSymbol.empty()) {
            std::cerr << "error: Guard symbol not specified." << std::endl;
            return false;
        }
    }

    return true;
}

void CompilerOptions::showHelp(const char* argv0) const
{
    const char* program = std::strrchr(argv0, '/');
    program = program ? program + 1 : argv0;
    std::cout << "Usage: " << program << " OPTIONS ... \n"
              << "\nOptions:\n"
              << "-b,  --base TYPE                 C++ enumeration base type, default is int\n"
              << "-e,  --enum NAME                 C++ enumeration name\n"
              << "-g,  --guard SYMBOL              C++ header guard symbol\n"
              << "-gp, --guard-with-pragma-once    Guard C++ header with #pragma once\n"
              << "-h,  --help                      Show help and exit\n"
              << "-H,  --header                    Produce header file\n"
              << "-i,  --input FILE                Input file\n"
              << "-LS, --symlist                   Produce symbol list\n"
              << "-LM, --msglist                   Produce message list\n"
              << "-n,  --namespace NAMESPACE       C++ namespace name\n"
              << "-o,  --output FILE               Output file\n"
              << "-T,  --text                      Produce text file\n"
              << "-VM, --validate-message-text     Produce text file\n"
              << std::endl;
}
