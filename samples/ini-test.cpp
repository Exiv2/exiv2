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

/*
670 rmills@rmillsmbp:~/gnu/exiv2/trunk/samples $ gcc ../src/ini.cpp  ini-test.cpp -lstdc++ -o ini-test
671 rmills@rmillsmbp:~/gnu/exiv2/trunk/samples $ ./ini-test
Config loaded from : 'initest.ini' version=6, name=Bob Smith, email=bob@smith.com, pi=3.14159, active=1
169=Sigma 35mm F1.4 DG HSM ART, 170=UNDEFINED
672 rmills@rmillsmbp:~/gnu/exiv2/trunk/samples $
*/

// Example that shows simple usage of the INIReader class
#include <exiv2/exiv2.hpp>
#include <iostream>

int main()
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXIV2_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    int              result = 0 ;
    const char*      ini    = "ini-test.ini";
    Exiv2::INIReader reader(ini);

    if (reader.ParseError() < 0) {
        std::cerr << "Can't load '" << ini << "'" << std::endl ;
        result = 1;
    } else {
        std::cout << "Config loaded from : '" << ini << "' "
                  << "version="  << reader.GetInteger("protocol", "version", -1)
                  << ", name="   << reader.Get("user", "name", "UNKNOWN")
                  << ", email="  << reader.Get("user", "email", "UNKNOWN")
                  << ", pi="     << reader.GetReal("user", "pi", -1)
                  << ", active=" << reader.GetBoolean("user", "active", true)
                  << std::endl ;

        std::cout << "169="      << reader.Get("canon",   "169","UNDEFINED")
                  << ", 170="    << reader.Get("canon",   "170","UNDEFINED")
                  << std::endl ;
    }

    return result ;
}
