// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "SystemError.h"

// CRT headers
#include <cstring>

// STL headers
#include <sstream>

namespace siodb::utils {

std::string formatErrorMessage(int errorCode, const char* prefix)
{
    std::ostringstream err;
    err << prefix << ": " << std::strerror(errorCode);
    return err.str();
}

void throwSystemError(int errorCode, const char* description, const char* arg1)
{
    std::ostringstream err;
    err << description << arg1;
    throwSystemError(errorCode, err.str().c_str());
}

}  // namespace siodb::utils
