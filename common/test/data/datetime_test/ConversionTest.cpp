// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/protobuf/RawDateTimeIO.h>

// Boost headers
#include <boost/date_time.hpp>

// Google Test
#include <gtest/gtest.h>

TEST(Conversion, DateTimeToEpoch)
{
    const siodb::RawDateTime dt(2001, 11, 29, 5, 15, 45, 32, 4885223);
    const auto epoch = dt.toEpochTimestamp();
    ASSERT_EQ(epoch, static_cast<std::time_t>(1009727132LL));
}

TEST(Conversion, DateToEpoch)
{
    const siodb::RawDate d(2001, 0, 20, 0);
    const auto epoch = d.toEpochTimestamp();
    ASSERT_EQ(epoch, static_cast<std::time_t>(980035200LL));
}

TEST(Conversion, TimeToEpoch)
{
    const siodb::RawTime t(15, 42, 39, 564543354);
    const auto epoch = t.toEpochTimestamp();
    ASSERT_EQ(epoch, static_cast<std::time_t>(56559LL));
}
