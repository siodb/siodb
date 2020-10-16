// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "CompilerOptions.h"
#include "Message.h"

// STL headers
#include <tuple>

bool parseMessages(const CompilerOptions& options, MessageContainer& messages);
void validateMessageText(const std::string& text);

bool writeSymbolListFile(const MessageContainer& messages, const CompilerOptions& options);
bool writeMessageListFile(const MessageContainer& messages, const CompilerOptions& options);
bool writeHeaderFile(const MessageContainer& messages, const CompilerOptions& options);
bool writeTextFile(const MessageContainer& messages, const CompilerOptions& options);

bool renameFile(const std::string& src, const std::string& dest);
std::tuple<std::string, int, int> makeTemporaryFile();
