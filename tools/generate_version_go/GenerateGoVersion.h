// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// STL headers
#include <string>

#ifndef GGV_VERSION
#define GGV_VERSION "internal"
#endif

#define GGV_COPYRIGHT_YEARS "2019-2020"

void printUsage(const char* program);

std::tuple<std::string, int, int> makeTemporaryFile();
