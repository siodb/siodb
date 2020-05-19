// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "chrono_ext.h"

namespace stdext::chrono {

high_capacity_system_clock::time_point high_capacity_system_clock::now() noexcept
{
    return high_capacity_system_clock::time_point(
            std::chrono::system_clock::now().time_since_epoch());
}

}  // namespace stdext::chrono
