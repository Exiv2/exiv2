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

#include "helper_functions.hpp"
#include <gtest/gtest.h>

TEST(string_from_unterminated, terminatedArray)
{
    const char data[5] = {'a', 'b', 'c', 0, 'd'};
    const std::string res = string_from_unterminated(data, 5);

    ASSERT_EQ(res.size(), 3u);
    ASSERT_STREQ(res.c_str(), "abc");
}

TEST(string_from_unterminated, unterminatedArray)
{
    const char data[4] = {'a', 'b', 'c', 'd'};
    const std::string res = string_from_unterminated(data, 4);

    ASSERT_EQ(res.size(), 4u);
    ASSERT_STREQ(res.c_str(), "abcd");
}
