// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/utils/DebugMacros.h>
#include <siodb/iomgr/shared/dbengine/Variant.h>

// Google Test
#include <gtest/gtest.h>

namespace dbengine = siodb::iomgr::dbengine;

TEST(Conversion, StringToDateTime)
{
    {
        const dbengine::Variant src("2020-11-06 04:58:04.5254 AM");
        const dbengine::Variant expected("2020-11-06 04:58:04.525400000");
        const dbengine::Variant dt(src.asDateTime());
        const dbengine::Variant dest(dt.asString());
        ASSERT_EQ(dest, expected);
    }

    {
        const dbengine::Variant src("2020-11-06 04:58:04.5254 PM");
        const dbengine::Variant expected("2020-11-06 16:58:04.525400000");
        const dbengine::Variant dt(src.asDateTime());
        const dbengine::Variant dest(dt.asString());
        ASSERT_EQ(dest, expected);
    }

    {
        const dbengine::Variant src("2020-11-06 04:58:04.525495063");
        const dbengine::Variant expected(src);
        const dbengine::Variant dt(src.asDateTime());
        const dbengine::Variant dest(dt.asString());
        ASSERT_EQ(dest, expected);
    }

    {
        const dbengine::Variant src("2020-11-06 16:58:04.525495063");
        const dbengine::Variant expected(src);
        const dbengine::Variant dt(src.asDateTime());
        const dbengine::Variant dest(dt.asString());
        ASSERT_EQ(dest, expected);
    }
}
