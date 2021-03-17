// ***************************************************************** -*- C++ -*-
// path-test.cpp
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
#include <fstream>
#include <sstream>
#include <string>

#include "utils.hpp"

int main(int argc, char* const argv[])
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << *argv[1] << ": Failed to open file for reading\n";
        return 1;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::string path, dir, base;
        std::istringstream is(line);
        is >> path >> dir >> base;
        std::string d = Util::dirname(path);
        std::string b = Util::basename(path);

        if (d != dir || b != base) {
            std::cout << path << "\t'" << d << "'\t '" << b
                      << "'\t ==> Testcase failed\n";
        }
    }

    return 0;
}
