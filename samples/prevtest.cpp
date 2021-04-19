// ***************************************************************** -*- C++ -*-
// prevtest.cpp
// Test access to preview images
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
#include <string>
#include <iostream>
#include <cassert>

int main(int argc, char* const argv[])
try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    std::string filename(argv[1]);

    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(filename);
    assert(image.get() != 0);
    image->readMetadata();

    Exiv2::PreviewManager loader(*image);
    Exiv2::PreviewPropertiesList list = loader.getPreviewProperties();
    for (Exiv2::PreviewPropertiesList::iterator pos = list.begin(); pos != list.end(); pos++) {
        std::cout << pos->mimeType_
                  << " preview, type " << pos->id_ << ", "
                  << pos->size_ << " bytes, "
                  << pos->width_ << 'x' << pos->height_ << " pixels"
                  << "\n";

        Exiv2::PreviewImage preview = loader.getPreviewImage(*pos);
        preview.writeFile(filename + "_"
                          + Exiv2::toString(pos->width_) + "x"
                          + Exiv2::toString(pos->height_));
    }

    // Cleanup
    Exiv2::XmpParser::terminate();
    
    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
