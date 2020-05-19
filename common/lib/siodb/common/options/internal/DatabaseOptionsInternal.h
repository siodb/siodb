// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "../../utils/Debug.h"

// Boost headers
#include <boost/property_tree/ini_parser.hpp>
namespace siodb::config {
namespace {

/**
 * Reads instance configuration into property tree data sructure.
 * @param instanceName Instance name.
 * @return property tree data structure.
 */
boost::property_tree::ptree readConfiguration(const std::string& instanceName);

}  // anonymous namespace
}  // namespace siodb::config
