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
// exifvalue.cpp
// Sample program to print value of an exif key in an image

#include <exiv2/exiv2.hpp>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <string>

int main(int argc, char* const argv[])
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " file key\n";
        return 1;
    }

    const char* file = argv[1];
    const char* key  = argv[2];

    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(file);
    assert(image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();

    if ( exifData.empty()) {
		std::cerr << "no metadata found in file " << file << std::endl;
		exit(2);
	}

	try {
     	std::cout << exifData[key] << std::endl;
	} catch (Exiv2::AnyError& e) {
    	std::cerr << "Caught Exiv2 exception '" << e << "'" << std::endl;
    	exit(3);
	} catch ( ... ) {
		std::cerr << "Caught a cold!" << std::endl;
    	exit(4);
	}

    return 0;
}
