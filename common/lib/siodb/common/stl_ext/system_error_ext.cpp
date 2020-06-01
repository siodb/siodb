// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "system_error_ext.h"

// CRT headers
#include <cstring>

// STL headers
#include <sstream>

namespace stdext {

std::string format_system_error_message(int errorCode, const char* prefix)
{
    std::ostringstream err;
    err << prefix << ": " << std::strerror(errorCode);
    return err.str();
}

[[noreturn]]
void throw_system_error(int errorCode, const char* description, const char* arg1)
{
    std::ostringstream err;
    err << description << arg1;
    throw_system_error(errorCode, err.str().c_str());
}

}  // namespace stdext
