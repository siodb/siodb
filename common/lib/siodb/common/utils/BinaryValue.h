// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

//#define SIODB_BINARY_VALUE_AS_VECTOR

// Project headers
#include "MemoryBuffer.h"

// CRT headers
#include <cstdint>

namespace siodb {

using BinaryValue = utils::MemoryBuffer<std::uint8_t>;

}  // namespace siodb
