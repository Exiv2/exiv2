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
#include "gtestwrapper.h"
using namespace Exiv2;

TEST(ATimeValue, isDefaultConstructed)
{
    const TimeValue value;
    ASSERT_EQ(0, value.getTime().hour);
    ASSERT_EQ(0, value.getTime().minute);
    ASSERT_EQ(0, value.getTime().second);
    ASSERT_EQ(0, value.getTime().tzHour);
    ASSERT_EQ(0, value.getTime().tzMinute);
}

TEST(ATimeValue, isConstructedWithArgs)
{
    const TimeValue value (23, 55, 2);
    ASSERT_EQ(23, value.getTime().hour);
    ASSERT_EQ(55, value.getTime().minute);
    ASSERT_EQ(2,  value.getTime().second);
    ASSERT_EQ(0, value.getTime().tzHour);
    ASSERT_EQ(0, value.getTime().tzMinute);
}

/// \todo add tests to check what happen with values out of valid ranges

TEST(ATimeValue, canBeReadFromStringHMS)
{
    TimeValue value;
    const std::string hms("23:55:02");
    ASSERT_EQ(0, value.read(hms));
    ASSERT_EQ(23, value.getTime().hour);
    ASSERT_EQ(55, value.getTime().minute);
    ASSERT_EQ(2,  value.getTime().second);
    ASSERT_EQ(0, value.getTime().tzHour);
    ASSERT_EQ(0, value.getTime().tzMinute);
}

TEST(ATimeValue, canBeReadFromWideString)
{
    TimeValue value;
    const std::string hms("23:55:02+04:04");
    ASSERT_EQ(0, value.read(hms));
    ASSERT_EQ(23, value.getTime().hour);
    ASSERT_EQ(55, value.getTime().minute);
    ASSERT_EQ(2,  value.getTime().second);
    ASSERT_EQ(4, value.getTime().tzHour);
    ASSERT_EQ(4, value.getTime().tzMinute);
}

TEST(ATimeValue, canBeReadFromWideStringNegative)
{
    TimeValue value;
    const std::string hms("23:55:02-04:04");
    ASSERT_EQ(0, value.read(hms));
    ASSERT_EQ(23, value.getTime().hour);
    ASSERT_EQ(55, value.getTime().minute);
    ASSERT_EQ(2,  value.getTime().second);
    ASSERT_EQ(-4, value.getTime().tzHour);
    ASSERT_EQ(-4, value.getTime().tzMinute);
}

/// \todo check what we should do here.
TEST(ATimeValue, canBeReadFromWideStringOther)
{
    TimeValue value;
    const std::string hms("23:55:02?04:04");
    ASSERT_EQ(0, value.read(hms));
    ASSERT_EQ(23, value.getTime().hour);
    ASSERT_EQ(55, value.getTime().minute);
    ASSERT_EQ(2,  value.getTime().second);
    ASSERT_EQ(4, value.getTime().tzHour);
    ASSERT_EQ(4, value.getTime().tzMinute);
}

TEST(ATimeValue, cannotReadFromStringWithBadFormat)
{
    TimeValue value;
    ASSERT_EQ(1, value.read("aa:55:02")); // String with non-digit chars
    ASSERT_EQ(1, value.read("25:55:02")); // Hours >= 24
    ASSERT_EQ(1, value.read("23:65:02")); // Minutes >= 60
    ASSERT_EQ(1, value.read("23:55:62")); // Seconds >= 60
    ASSERT_EQ(1, value.read("23:55:02+25:04")); // tzHour >= 24
    ASSERT_EQ(1, value.read("23:55:02+04:66")); // tzMinutes >= 60

    /// \todo This one does not fail
    //ASSERT_EQ(1, value.read("23:55:02+04:06:06")); // More components than expected
}

TEST(ATimeValue, isCopiedToBuffer)
{
    const TimeValue value (23, 55, 2);
    byte buffer[11];
    ASSERT_EQ(11, value.copy(buffer));

    const byte expectedDate[11] = {'2', '3', '5', '5', '0', '2',
                                   '+', '0', '0', '0', '0'};
    for (int i = 0; i < 11; ++i) {
        ASSERT_EQ(expectedDate[i], buffer[i]) << "i: " << i;
    }
}
