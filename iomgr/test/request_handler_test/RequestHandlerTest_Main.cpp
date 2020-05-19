// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Project headers
#include "RequestHandlerTest_TestEnv.h"

// Common project headers
#include <siodb/common/utils/Debug.h>
#include <siodb/common/utils/StartupActions.h>

int main(int argc, char** argv)
{
    // Must be called very first!
    siodb::utils::performCommonStartupActions();

    DEBUG_SYSCALLS_LIBRARY_GUARD;

    // Run tests
    testing::InitGoogleTest(&argc, argv);
    // Note: gtest takes ownership of the TestEnvironment object
    testing::AddGlobalTestEnvironment(new TestEnvironment(argv[0]));
    return RUN_ALL_TESTS();
}
