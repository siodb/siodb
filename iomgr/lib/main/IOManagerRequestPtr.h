// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

// STL headers
#include <memory>

namespace siodb::iomgr {

class IOManagerRequest;

/** Database IO Manager shared pointer shortcut type */
using IOManagerRequestPtr = std::shared_ptr<IOManagerRequest>;

/** Read-only IO Manager request shared pointer shortcut type */
using ConstIOManagerRequestPtr = std::shared_ptr<const IOManagerRequest>;

}  // namespace siodb::iomgr
