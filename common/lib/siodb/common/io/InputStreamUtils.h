// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "InputStream.h"

// STL headers
#include <string>

namespace siodb::io {

/**
 * Reads chunked string from the given input stream.
 * @param in Input stream.
 * @return The string.
 */
std::string readChunkedString(InputStream& in);

}  // namespace siodb::io
