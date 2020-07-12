// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include "IOManagerRequest.h"

namespace siodb::iomgr {

std::atomic<std::uint64_t> IOManagerRequest::s_idCounter(0);

}  // namespace siodb::iomgr
