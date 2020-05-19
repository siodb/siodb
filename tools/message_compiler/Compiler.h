// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "CompilerOptions.h"
#include "Message.h"

#ifndef SIODB_MESSAGE_COMPILER_VERSION
#define SIODB_MESSAGE_COMPILER_VERSION "internal"
#endif

#ifndef SIODB_MESSAGE_COMPILER_COPYRIGHT_YEARS
#define SIODB_MESSAGE_COMPILER_COPYRIGHT_YEARS __DATE__
#endif

bool parseMessages(const CompilerOptions& options, MessageContainer& messages);
bool writeSymbolListFile(const MessageContainer& messages, const CompilerOptions& options);
bool writeMessageListFile(const MessageContainer& messages, const CompilerOptions& options);
bool writeHeaderFile(const MessageContainer& messages, const CompilerOptions& options);
bool writeTextFile(const MessageContainer& messages, const CompilerOptions& options);

std::string& replaceAll(
        std::string& str, const std::string& pattern, const std::string& substitute);
