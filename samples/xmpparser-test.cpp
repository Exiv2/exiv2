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
// xmpparser-test.cpp
// Read an XMP packet from a file, parse and re-serialize it.

#include <exiv2/exiv2.hpp>
#include <string>
#include <iostream>
#include <iomanip>

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
    Exiv2::DataBuf buf = Exiv2::readFile(filename);
    std::string xmpPacket;
    xmpPacket.assign(reinterpret_cast<char*>(buf.pData_), buf.size_);
    std::cerr << "-----> Decoding XMP data read from " << filename << " <-----\n";
    Exiv2::XmpData xmpData;
    if (0 != Exiv2::XmpParser::decode(xmpData, xmpPacket)) {
        std::string error(argv[1]);
        error += ": Failed to parse file contents (XMP packet)";
        throw Exiv2::Error(Exiv2::kerErrorMessage, error);
    }
    if (xmpData.empty()) {
        std::string error(argv[1]);
        error += ": No XMP properties found in the XMP packet";
        throw Exiv2::Error(Exiv2::kerErrorMessage, error);
    }
    for (auto&& md : xmpData) {
        std::cout << std::setfill(' ') << std::left << std::setw(44) << md.key() << " " << std::setw(9)
                  << std::setfill(' ') << std::left << md.typeName() << " " << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right << md.count() << "  " << std::dec << md.toString() << std::endl;
    }
    filename += "-new";
    std::cerr << "-----> Encoding XMP data to write to " << filename << " <-----\n";
    if (0 != Exiv2::XmpParser::encode(xmpPacket, xmpData)) {
        std::string error(argv[1]);
        error += ": Failed to encode the XMP data";
        throw Exiv2::Error(Exiv2::kerErrorMessage, error);
    }
    Exiv2::FileIo file(filename);
    if (file.open("wb") != 0) {
        throw Exiv2::Error(Exiv2::kerFileOpenFailed, filename, "wb", Exiv2::strError());
    }
    if (file.write(reinterpret_cast<const Exiv2::byte*>(xmpPacket.data()), static_cast<long>(xmpPacket.size())) == 0) {
        throw Exiv2::Error(Exiv2::kerCallFailed, filename, Exiv2::strError(), "FileIo::write");
    }
    Exiv2::XmpParser::terminate();
    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
