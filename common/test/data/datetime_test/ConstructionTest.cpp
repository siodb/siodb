// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

// Common project headers
#include <siodb/common/protobuf/RawDateTimeIO.h>

// Boost headers
#include <boost/date_time.hpp>

// Google Test
#include <gtest/gtest.h>

TEST(Construction, ParseWithDefaultFormat)
{
    const siodb::RawDateTime dt("2020-08-03 14:05:17");
    EXPECT_TRUE(dt.m_datePart.m_hasTimePart);
    EXPECT_EQ(dt.m_datePart.m_year, 2020);
    EXPECT_EQ(dt.m_datePart.m_month, 7U);
    EXPECT_EQ(dt.m_datePart.m_dayOfMonth, 2U);
    EXPECT_EQ(dt.m_datePart.m_dayOfWeek, 1U);
    EXPECT_EQ(dt.m_timePart.m_hours, 14U);
    EXPECT_EQ(dt.m_timePart.m_minutes, 5U);
    EXPECT_EQ(dt.m_timePart.m_seconds, 17U);
    EXPECT_EQ(dt.m_timePart.m_nanos, 0U);
    EXPECT_FALSE(dt.m_timePart.m_reserved1);
    EXPECT_EQ(dt.m_timePart.m_reserved2, 0);
}

TEST(Construction, ParseWithCustomFormat)
{
    const siodb::RawDateTime dt("08/03/2020 14:05.17", "%m/%d/%Y %H:%M.%S");
    EXPECT_TRUE(dt.m_datePart.m_hasTimePart);
    EXPECT_EQ(dt.m_datePart.m_year, 2020);
    EXPECT_EQ(dt.m_datePart.m_month, 7U);
    EXPECT_EQ(dt.m_datePart.m_dayOfMonth, 2U);
    EXPECT_EQ(dt.m_datePart.m_dayOfWeek, 1U);
    EXPECT_EQ(dt.m_timePart.m_hours, 14U);
    EXPECT_EQ(dt.m_timePart.m_minutes, 5U);
    EXPECT_EQ(dt.m_timePart.m_seconds, 17U);
    EXPECT_EQ(dt.m_timePart.m_nanos, 0U);
    EXPECT_FALSE(dt.m_timePart.m_reserved1);
    EXPECT_EQ(dt.m_timePart.m_reserved2, 0);
}

TEST(Construction, DateTime_FromEpochTime)
{
    const siodb::RawDateTime dt(1596499517);
    EXPECT_TRUE(dt.m_datePart.m_hasTimePart);
    EXPECT_EQ(dt.m_datePart.m_year, 2020);
    EXPECT_EQ(dt.m_datePart.m_month, 7U);
    EXPECT_EQ(dt.m_datePart.m_dayOfMonth, 3U);
    EXPECT_EQ(dt.m_datePart.m_dayOfWeek, 2U);
    EXPECT_EQ(dt.m_timePart.m_hours, 0U);
    EXPECT_EQ(dt.m_timePart.m_minutes, 5U);
    EXPECT_EQ(dt.m_timePart.m_seconds, 17U);
    EXPECT_EQ(dt.m_timePart.m_nanos, 0U);
    EXPECT_FALSE(dt.m_timePart.m_reserved1);
    EXPECT_EQ(dt.m_timePart.m_reserved2, 0);
}

TEST(Construction, Date_FromEpochTime)
{
    const siodb::RawDate d(1596499517);
    EXPECT_FALSE(d.m_hasTimePart);
    EXPECT_EQ(d.m_year, 2020);
    EXPECT_EQ(d.m_month, 7U);
    EXPECT_EQ(d.m_dayOfMonth, 3U);
    EXPECT_EQ(d.m_dayOfWeek, 2U);
}

TEST(Construction, Time_FromEpochTime)
{
    const siodb::RawTime t(1596499517);
    EXPECT_EQ(t.m_hours, 0U);
    EXPECT_EQ(t.m_minutes, 5U);
    EXPECT_EQ(t.m_seconds, 17U);
    EXPECT_EQ(t.m_nanos, 0U);
    EXPECT_FALSE(t.m_reserved1);
    EXPECT_EQ(t.m_reserved2, 0);
}
