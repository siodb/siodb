// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "StartupActions.h"

// Project headers
#include "../stl_wrap/filesystem_wrapper.h"

// Protobuf headers
#include <google/protobuf/stubs/common.h>

namespace siodb::utils {

void performCommonStartupActions()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // We must do it as first thing
    // https://www.boost.org/doc/libs/1_64_0/libs/log/doc/html/log/rationale/why_crash_on_term.html
    fs::path::imbue(std::locale::classic());
}

}  // namespace siodb::utils
