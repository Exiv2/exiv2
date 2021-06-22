// ***************************************************************** -*- C++ -*-
// stringto-test.cpp
// Test conversions from string to long, float and Rational types.
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
#include <iostream>
#include <iomanip>

static constexpr const char* testcases[] = {
    // bool
    "True",
    "False",
    "t",
    "f",
    // long
    "-1",
    "0",
    "1",
    // float
    "0.0",
    "0.1",
    "0.01",
    "0.001",
    "-1.49999",
    "-1.5",
    "1.49999",
    "1.5",
    // Rational
    "0/1",
    "1/1",
    "1/3",
    "-1/3",
    "4/3",
    "-4/3",
    "0/0",
    // nok
    "text",
};

int main()
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    std::cout << std::setfill(' ');

    std::cout << std::setw(12) << std::left << "string";
    std::cout << std::setw(12) << std::left << "long";
    std::cout << std::setw(12) << std::left << "float";
    std::cout << std::setw(12) << std::left << "Rational";

    std::cout << std::endl;

    for (auto&& testcase : testcases) {
        try {
            std::string s(testcase);
            std::cout << std::setw(12) << std::left << s;
            bool ok = false;

            long l = Exiv2::parseLong(s, ok);
            std::cout << std::setw(12) << std::left;
            if (ok)
                std::cout << l;
            else
                std::cout << "nok";

            float f = Exiv2::parseFloat(s, ok);
            std::cout << std::setw(12) << std::left;
            if (ok)
                std::cout << f;
            else
                std::cout << "nok";

            Exiv2::Rational r = Exiv2::parseRational(s, ok);
            if (ok)
                std::cout << r.first << "/" << r.second;
            else
                std::cout << "nok";

            std::cout << std::endl;
        } catch (Exiv2::AnyError& e) {
            std::cout << "Caught Exiv2 exception '" << e << "'\n";
            return -1;
        }
    }

    return 0;
}
