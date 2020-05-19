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

TEST(EqualityOperator, SameDateParts_NoTimePart)
{
    // Case #1: First and second datetime do not have time part and have same date part → true
    const auto dt1 = makeSampleDateTime();
    const auto dt2 = makeSampleDateTime();
    EXPECT_TRUE(dt1 == dt2);
    EXPECT_FALSE(dt1 != dt2);
}

TEST(EqualityOperator, SameDatParts_SameTimeParts)
{
    // Case #2: This and other have same date and time parts → true
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_hasTimePart = true;
    dt2.m_datePart.m_hasTimePart = true;
    EXPECT_TRUE(dt1 == dt2);
    EXPECT_FALSE(dt1 != dt2);
}

TEST(EqualityOperator, DifferentDateParts_NoTimePart)
{
    // Case #3: This and other do not have time part and other differs in date part → false
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt2.m_datePart.m_year = 2018;
    EXPECT_FALSE(dt1 == dt2);
    EXPECT_TRUE(dt1 != dt2);
}

TEST(EqualityOperator, DifferentDateParts_SameTimePart)
{
    // Case #4: This and other have time part, other differs in date part,
    // time parts are the same → false
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_hasTimePart = true;
    dt2.m_datePart.m_hasTimePart = true;
    dt2.m_datePart.m_year = 2018;
    EXPECT_FALSE(dt1 == dt2);
    EXPECT_TRUE(dt1 != dt2);
}

TEST(EqualityOperator, SameDateParts_ThisHasTimePart_OtherHasNoTimePart)
{
    // Case #5: This and other have same date parts, this has nonzero time part,
    // other doesn’t have time part → false
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt1.m_datePart.m_hasTimePart = true;
    EXPECT_FALSE(dt1 == dt2);
    EXPECT_TRUE(dt1 != dt2);
}

TEST(EqualityOperator, SameDateParts_DifferentTimeParts)
{
    // Case #6: This and other have same date parts, this has nonzero time part,
    // other has different nonzero time part → false
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt2.m_datePart.m_hasTimePart = true;
    dt2.m_datePart.m_hasTimePart = true;
    dt2.m_timePart.m_hours = 11;
    EXPECT_FALSE(dt1 == dt2);
    EXPECT_TRUE(dt1 != dt2);
}

TEST(EqualityOperator, SameDateParts_ThisHasNoTimePart_OtherHasTimePart)
{
    // Case #7: This and other have same date parts, this doesn’t have time part,
    // other has nonzero time part → false
    auto dt1 = makeSampleDateTime();
    auto dt2 = makeSampleDateTime();
    dt2.m_datePart.m_hasTimePart = true;
    EXPECT_FALSE(dt1 == dt2);
    EXPECT_TRUE(dt1 != dt2);
}
