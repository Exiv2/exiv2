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

#include <exiv2/exiv2.hpp>
#include <gtest/gtest.h>
#include <image_int.hpp>

using namespace Exiv2::Internal;
using Exiv2::makeSlice;
using Exiv2::Slice;

static const unsigned char buf[10] = {'a', 'b', 'c', 1, 4, 0, 'e', 136, 0, 'a'};

template <typename T>
void checkBinaryToString(const Exiv2::Slice<T> sl, const char* expectedOutput)
{
    // construct the helper manually so that we catch potential invalidation of
    // temporaries
    std::stringstream ss;
    const binaryToStringHelper<T> helper = binaryToString(sl);
    ss << helper;

    ASSERT_STREQ(ss.str().c_str(), expectedOutput);
}

TEST(binaryToString, zeroStart)
{
    // a, b, c are printable, 1 & 4 are not => '.', 0 at last position => skipped
    checkBinaryToString(makeSlice(buf, 0, 6), "abc..");

    // same as previous, but now last element is not ignored since it is not 0
    checkBinaryToString(makeSlice(buf, 0, 5), "abc..");

    // same as first, only now the 0 & 136 are converted to '.'
    checkBinaryToString(makeSlice(buf, 0, 8), "abc...e.");

    // should result in the same as previously, as trailing zero is ignored
    checkBinaryToString(makeSlice(buf, 0, 9), "abc...e.");

    // ensure that the function does not overread when last element != 0
    checkBinaryToString(makeSlice(buf, 0, sizeof(buf)), "abc...e..a");
}

TEST(binaryToString, nonZeroStart)
{
    // start @ index 1, read 6 characters (until e)
    checkBinaryToString(makeSlice(buf, 1, 7), "bc...e");

    // start @ index 3, read until end
    checkBinaryToString(makeSlice(buf, 3, sizeof(buf)), "...e..a");
}
