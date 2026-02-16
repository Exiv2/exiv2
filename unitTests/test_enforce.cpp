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
#include <enforce.hpp>

#include <gtest/gtest.h>

#include <stdexcept>

TEST(enforce, errMessage)
{
    try {
        enforce(false, Exiv2::kerErrorMessage, "an error occurred");
    } catch (const Exiv2::Error& e) {
        ASSERT_STREQ(e.what(), "an error occurred");
    }
}

TEST(enforce, exceptionThrown)
{
    ASSERT_NO_THROW(enforce(true, Exiv2::kerErrorMessage));

    ASSERT_THROW(enforce(false, Exiv2::kerErrorMessage), Exiv2::Error);
    ASSERT_THROW(enforce<std::overflow_error>(false, "error message"), std::overflow_error);
    ASSERT_THROW(enforce(false, Exiv2::kerMallocFailed), Exiv2::Error);
    ASSERT_THROW(enforce(false, Exiv2::kerErrorMessage, "error message"), Exiv2::Error);
    ASSERT_THROW(enforce(false, Exiv2::kerDataSourceOpenFailed, "path", "strerror"), Exiv2::Error);
    ASSERT_THROW(enforce(false, Exiv2::kerCallFailed, "path", "strerror", "function"), Exiv2::Error);
}
