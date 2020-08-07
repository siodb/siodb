// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/protobuf/RawDateTimeIO.h>

// Boost headers
#include <boost/date_time.hpp>

// Google Test
#include <gtest/gtest.h>

namespace {

siodb::RawDateTime makeSampleDateTime() noexcept
{
    siodb::RawDateTime dt;
    dt.m_datePart.m_hasTimePart = false;
    dt.m_datePart.m_dayOfWeek = boost::gregorian::date(2019, 12, 22).day_of_week();
    dt.m_datePart.m_dayOfMonth = 21;
    dt.m_datePart.m_month = 11;
    dt.m_datePart.m_year = 2019;
    dt.m_timePart.m_nanos = 0;
    dt.m_timePart.m_seconds = 59;
    dt.m_timePart.m_minutes = 12;
    dt.m_timePart.m_hours = 12;
    dt.m_timePart.m_reserved1 = 0;
    dt.m_timePart.m_reserved2 = 0;
    return dt;
}

}  // namespace

TEST(RelationalOperators, YearDifference_NoTimePart)
{
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_year += 1;
    EXPECT_TRUE(dt1 > dt2);
    EXPECT_TRUE(dt2 < dt1);
    EXPECT_TRUE(dt1 >= dt2);
    EXPECT_TRUE(dt2 <= dt1);
}

TEST(RelationalOperators, MonthDifference_NoTimePart)
{
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_month += 1;
    EXPECT_TRUE(dt1 > dt2);
    EXPECT_TRUE(dt2 < dt1);
    EXPECT_TRUE(dt1 >= dt2);
    EXPECT_TRUE(dt2 <= dt1);
}

TEST(RelationalOperators, DayDifference_NoTimePart)
{
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_dayOfMonth += 1;
    EXPECT_TRUE(dt1 > dt2);
    EXPECT_TRUE(dt2 < dt1);
    EXPECT_TRUE(dt1 >= dt2);
    EXPECT_TRUE(dt2 <= dt1);
}

TEST(RelationalOperators, DateEqual_NoTimePart)
{
    auto dt1 = makeSampleDateTime();
    const auto dt2 = makeSampleDateTime();
    dt1.m_timePart.m_hours += 1;
    EXPECT_TRUE(dt1 >= dt2);
    EXPECT_TRUE(dt2 <= dt1);
    EXPECT_FALSE(dt1 > dt2);
    EXPECT_FALSE(dt2 < dt1);
}

TEST(RelationalOperators, DateEqual_HasTimePart)
{
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_hasTimePart = true;
    dt2.m_datePart.m_hasTimePart = true;
    EXPECT_TRUE(dt1 >= dt2);
    EXPECT_TRUE(dt1 <= dt2);
    EXPECT_FALSE(dt1 > dt2);
    EXPECT_FALSE(dt2 < dt1);
}

TEST(RelationalOperators, DateEqual_HoursDifference)
{
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_hasTimePart = true;
    dt2.m_datePart.m_hasTimePart = true;
    dt1.m_timePart.m_hours += 1;
    EXPECT_TRUE(dt1 > dt2);
    EXPECT_TRUE(dt1 >= dt2);
    EXPECT_TRUE(dt2 < dt1);
    EXPECT_TRUE(dt2 <= dt1);
}

TEST(RelationalOperators, DateEqual_MinutesDifference)
{
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_hasTimePart = true;
    dt2.m_datePart.m_hasTimePart = true;
    dt1.m_timePart.m_minutes += 1;
    EXPECT_TRUE(dt1 > dt2);
    EXPECT_TRUE(dt1 >= dt2);
    EXPECT_TRUE(dt2 < dt1);
    EXPECT_TRUE(dt2 <= dt1);
}

TEST(RelationalOperators, DateEqual_SecondsDifference)
{
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_hasTimePart = true;
    dt2.m_datePart.m_hasTimePart = true;
    dt1.m_timePart.m_seconds += 1;
    EXPECT_TRUE(dt1 > dt2);
    EXPECT_TRUE(dt1 >= dt2);
    EXPECT_TRUE(dt2 < dt1);
    EXPECT_TRUE(dt2 <= dt1);
}

TEST(RelationalOperators, DateEqual_NanosDifference)
{
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_hasTimePart = true;
    dt2.m_datePart.m_hasTimePart = true;
    dt1.m_timePart.m_nanos += 1;
    EXPECT_TRUE(dt1 > dt2);
    EXPECT_TRUE(dt1 >= dt2);
    EXPECT_TRUE(dt2 < dt1);
    EXPECT_TRUE(dt1 <= dt1);
}
