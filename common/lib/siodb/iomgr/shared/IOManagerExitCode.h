// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb::iomgr {

/** IO Manager exit codes */
enum IOManagerExitCode {
    kIOManagerExitCode_Success = 0,
    kIOManagerExitCode_InvalidConfig = 1,
    kIOManagerExitCode_LogInitializationFailed = 2,
    kIOManagerExitCode_InitializationFailed = 3
};

}  // namespace siodb::iomgr
