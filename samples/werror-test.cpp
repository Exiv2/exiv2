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

// ***************************************************************** -*- C++ -*-
// werror-test.cpp
// Simple tests for the wide-string error class WError

#include <exiv2/exiv2.hpp>

#include <iostream>

int main()
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    try {
        throw Exiv2::Error(Exiv2::kerGeneralError, "ARG1", "ARG2", "ARG3");
    }
    catch (const Exiv2::Error& e) {
        std::cout << "Caught Error '" << e.what() << "'\n";
    }

#ifdef EXV_UNICODE_PATH
    try {
        throw Exiv2::WError(Exiv2::kerGeneralError, L"WARG1", L"WARG2", L"WARG3");
    }
    catch (const Exiv2::WError& e) {
        std::wstring wmsg = e.wwhat();
        std::string msg(wmsg.begin(), wmsg.end());
        std::cout << "Caught WError '" << msg << "'\n";
    }
#endif

    return 0;
}
