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

// xmpdump.cpp
// Sample program to dump the XMP packet of an image
#include <exiv2/exiv2.hpp>
#include <cassert>
#include <iostream>
#include <string>

int main(int argc, char* const argv[])
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    try {
        if (argc != 2) {
            std::cout << "Usage: " << argv[0] << " file\n";
            return 1;
        }

        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1]);
        assert(image.get() != 0);
        image->readMetadata();

        const std::string& xmpPacket = image->xmpPacket();
        if (xmpPacket.empty()) {
            std::string error(argv[1]);
            error += ": No XMP packet found in the file";
            throw Exiv2::Error(Exiv2::kerErrorMessage, error);
        }
        std::cout << xmpPacket << "\n";

        return 0;
    } catch (Exiv2::AnyError& e) {
        std::cout << "Caught Exiv2 exception '" << e << "'\n";
        return -1;
    }
}
