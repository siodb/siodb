// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// Project headers
#include "InputStream.h"
#include "OutputStream.h"

namespace siodb::io {

/** Common interface for the combined input and output stream classes. */
class InputOutputStream : public InputStream, public OutputStream {
};

}  // namespace siodb::io
