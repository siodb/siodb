// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/utils/DebugMacros.h>

// Google Test
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    DEBUG_SYSCALLS_LIBRARY_GUARD;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
