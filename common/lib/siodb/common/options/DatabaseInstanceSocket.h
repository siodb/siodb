// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <string>

namespace siodb {

/**
 * Compose database instance socket path.
 * @param instanceName Database instance name.
 * @return Database instance configuration file path.
 */
std::string composeInstanceSocketPath(const std::string& instanceName);

}  // namespace siodb
