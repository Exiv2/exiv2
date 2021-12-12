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

using namespace Exiv2;

TEST(ADateValue, isDefaultConstructed)
{
    const DateValue dateValue;
    ASSERT_EQ(0, dateValue.getDate().year);
    ASSERT_EQ(0, dateValue.getDate().month);
    ASSERT_EQ(0, dateValue.getDate().day);
}

TEST(ADateValue, isConstructedWithArgs)
{
    const DateValue dateValue (2018, 4, 2);
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(4, dateValue.getDate().month);
    ASSERT_EQ(2, dateValue.getDate().day);
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



TEST(ADateValue, copyToByteBuffer)
{
    const DateValue dateValue (2018, 4, 2);
    const byte expectedDate[8] = {0x32, 0x30, 0x31, 0x38, 0x30, 0x34, 0x30, 0x32 }; // 20180402
    byte buffer[8];
    ASSERT_EQ(8, dateValue.copy(buffer));
    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(expectedDate[i], buffer[i]);
    }
}
