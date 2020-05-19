// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "Uuid.h"

namespace siodb::utils {

namespace {

const Uuid kZeroUuid {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

}  // namespace

const Uuid& getZeroUuid() noexcept
{
    return kZeroUuid;
}

}  // namespace siodb::utils
