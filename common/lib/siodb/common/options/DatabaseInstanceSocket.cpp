// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "DatabaseInstanceSocket.h"

// Project headers
#include "../config/SiodbDefs.h"

// STL headers
#include <sstream>

namespace siodb {

std::string composeInstanceSocketPath(const std::string& instanceName)
{
    std::ostringstream str;
    str << kInstanceSocketPrefix << instanceName << ".socket";
    return str.str();
}

}  // namespace siodb
