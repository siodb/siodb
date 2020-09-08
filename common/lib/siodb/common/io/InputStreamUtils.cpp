// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "InputStreamUtils.h"

// Project headers
#include "ChunkedInputStream.h"

// STL headers
#include <sstream>

namespace siodb::io {

std::string readChunkedString(InputStream& in)
{
    char buffer[4096];
    std::ostringstream str;
    siodb::io::ChunkedInputStream chunkedInput(in);
    while (!chunkedInput.isEof()) {
        const auto n = chunkedInput.read(buffer, sizeof(buffer) - 1);
        if (n < 1) break;
        buffer[n] = 0;
        str << buffer;
    }
    return str.str();
}

}  // namespace siodb::io
