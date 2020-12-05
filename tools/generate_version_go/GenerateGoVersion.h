// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// STL headers
#include <string>

void printUsage(const char* program);

bool renameFile(const std::string& src, const std::string& dest);
bool adjustPermissions(const std::string& file);
std::tuple<std::string, int, int> makeTemporaryFile();
