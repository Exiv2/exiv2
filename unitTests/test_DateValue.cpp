// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */

#include "value.hpp"

#include <gtest/gtest.h>

#include <array>
#include <algorithm>
#include <sstream>

using namespace Exiv2;

TEST(ADateValue, isDefaultConstructed)
{
    const DateValue dateValue;
    ASSERT_EQ(0, dateValue.getDate().year);
    ASSERT_EQ(0, dateValue.getDate().month);
    ASSERT_EQ(0, dateValue.getDate().day);
}

TEST(ADateValue, canBeConstructedWithValidDate)
{
    const DateValue dateValue (2018, 4, 2);
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(4, dateValue.getDate().month);
    ASSERT_EQ(2, dateValue.getDate().day);
}

/// \todo Probably we should avoid this ...
TEST(ADateValue, canBeConstructedWithInvalidDate)
{
    const DateValue dateValue (2018, 13, 69);
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(13, dateValue.getDate().month);
    ASSERT_EQ(69, dateValue.getDate().day);
}

TEST(ADateValue, setsValidDateCorrectly)
{
    DateValue dateValue;
    DateValue::Date date;
    date.year = 2018;
    date.month = 4;
    date.day = 2;

    dateValue.setDate(date);
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(4, dateValue.getDate().month);
    ASSERT_EQ(2, dateValue.getDate().day);
}

/// \todo Probably we should avoid this ...
TEST(ADateValue, setsInvalidDateCorrectly)
{
    DateValue dateValue;
    DateValue::Date date;
    date.year = 2018;
    date.month = 13;
    date.day = 69;

    dateValue.setDate(date);
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(13, dateValue.getDate().month);
    ASSERT_EQ(69, dateValue.getDate().day);
}



TEST(ADateValue, readFromByteBufferWithExpectedSize)
{
    DateValue dateValue;
    const byte date[8] = {0x32, 0x30, 0x31, 0x38, 0x30, 0x34, 0x30, 0x32 }; // 20180402
    ASSERT_EQ(0, dateValue.read(date, 8));
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(4, dateValue.getDate().month);
    ASSERT_EQ(2, dateValue.getDate().day);
}

TEST(ADateValue, doNotReadFromByteBufferWithoutExpectedSize)
{
    DateValue dateValue;
    const byte date[8] = {0x32, 0x30, 0x31, 0x38, 0x30, 0x34, 0x30, 0x32 }; // 20180402
    ASSERT_EQ(1, dateValue.read(date, 6));
}

TEST(ADateValue, doNotReadFromByteBufferWithExpectedSizeButNotCorrectContent)
{
    DateValue dateValue;
    const byte date[8] = {0x32, 0x30, 0x31, 0x38, 0x30, 0x34, 0x23, 0x23 }; // 201804##
    ASSERT_EQ(1, dateValue.read(date, 8));
}


TEST(ADateValue, readFromStringWithExpectedSizeAndDashes)
{
    DateValue dateValue;
    const std::string date ("2018-04-02");
    ASSERT_EQ(0, dateValue.read(date));
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(4, dateValue.getDate().month);
    ASSERT_EQ(2, dateValue.getDate().day);
}

TEST(ADateValue, readFromStringWithExpectedSizeWithoutDashes)
{
    DateValue dateValue;
    const std::string date ("20180402");
    ASSERT_EQ(0, dateValue.read(date));
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(4, dateValue.getDate().month);
    ASSERT_EQ(2, dateValue.getDate().day);
}

TEST(ADateValue, readFromStringWithTime)
{
    DateValue dateValue;
    const std::string date ("2018-04-02T12:01:44.999999999");
    ASSERT_EQ(0, dateValue.read(date));
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(4, dateValue.getDate().month);
    ASSERT_EQ(2, dateValue.getDate().day);
}

TEST(ADateValue, doNotReadFromStringWithoutExpectedSize)
{
    DateValue dateValue;
    ASSERT_EQ(1, dateValue.read("2018-04-0"));
    ASSERT_EQ(1, dateValue.read("2018040"));
}

TEST(ADateValue, doNotReadFromStringWithExpectedSizeButNotCorrectContent)
{
    DateValue dateValue;
    ASSERT_EQ(1, dateValue.read("2018-24-02"));
    ASSERT_EQ(1, dateValue.read("2018-aa-bb"));
    ASSERT_EQ(1, dateValue.read("2018aabb"));
}

TEST(ADateValue, writesToExtendedFormat)
{
    const DateValue dateValue (2021, 12, 1);
    std::ostringstream stream;
    dateValue.write(stream);
    ASSERT_EQ("2021-12-01", stream.str());
}

TEST(ADateValue, copiesToByteBufferWithBasicFormat)
{
    const DateValue dateValue (2021, 12, 1);
    std::array<byte, 8> buf;
    buf.fill(0);

    const byte expectedDate[10] = {'2', '0', '2', '1', '1', '2', '0', '1'};
    ASSERT_EQ(8, dateValue.copy(buf.data()));
    ASSERT_TRUE(std::equal(buf.begin(), buf.end(), expectedDate));
}

// I used https://www.epochconverter.com/ for knowing the expectations
/* These functions convert the time to the local calendar time. Find a way to do the conversions with UTC

TEST(ADateValue, toLong)
{
    const DateValue dateValue (2021, 12, 1);
    long val = dateValue.toLong();
    ASSERT_EQ(1638313200, val);
}

TEST(ADateValue, toFloat)
{
    const DateValue dateValue (2021, 12, 1);
    long val = dateValue.toFloat();
    ASSERT_FLOAT_EQ(1638313200.f, val);
}

TEST(ADateValue, toRational)
{
    const DateValue dateValue (2021, 12, 1);
    auto val = dateValue.toRational();
    ASSERT_EQ(1638313200, val.first);
    ASSERT_EQ(1, val.second);
}
*/
