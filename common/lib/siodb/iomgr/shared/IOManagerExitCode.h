// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#pragma once

namespace siodb {

/** IO Manager exit codes */
enum IOManagerExitCode {
    kIOManagerExitCode_Success = 0,
    kIOManagerExitCode_InvalidConfig = 1,
    kIOManagerExitCode_DatabaseEngineIntializationFailed = 2,
    kIOManagerExitCode_ConnectionCreationFailed = 3,
    kIOManagerExitCode_LogInitializationFailed = 4,
    kIOManagerExitCode_InitializationFailed = 5,
};

}  // namespace siodb
