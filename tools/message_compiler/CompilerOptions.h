// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

enum class OutputType {
    Unknown,
    SymbolList,
    MessageList,
    Header,
    Text,
};

struct CompilerOptions {
    bool parse(int argc, char** argv);
    void showHelp(const char* argv0) const;

    OutputType m_outputType = OutputType::Unknown;
    std::string m_inputFileName;
    std::string m_outputFileName;
    std::string m_namespaceName = "siodb";
    std::string m_guardSymbol;
    std::string m_enumBaseType = "int";
    std::string m_enumName;
    bool m_validateMessageText = false;
    bool m_guardWithPragmaOnce = false;
};
