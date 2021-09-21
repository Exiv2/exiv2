// ***************************************************************** -*- C++ -*-
// remotetest.cpp
// Tester application for testing remote i/o.
// It makes some modifications on the metadata of remote file, reads new metadata from that file
// and reset the metadata back to the original status.
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
#include <cassert>

int main(int argc, char* const argv[])
try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " file {--nocurl | --curl}\n\n";
        return 1;
    }

    bool useCurlFromExiv2TestApps = true;
    for ( int a = 1 ; a < argc ; a++ ) {
        std::string arg(argv[a]);
        if (arg == "--nocurl")  useCurlFromExiv2TestApps = false;
        else if (arg == "--curl") useCurlFromExiv2TestApps = true;
    }

    std::string file(argv[1]);

    // set/add metadata
    std::cout << "Modify the metadata ...\n";
    Exiv2::ExifData exifData;
    exifData["Exif.Photo.UserComment"]  = "Hello World";           // AsciiValue
    exifData["Exif.Image.Software"]     = "Exiv2";                 // AsciiValue
    exifData["Exif.Image.Copyright"]    = "Exiv2";                 // AsciiValue
    exifData["Exif.Image.Make"]         = "Canon";                 // AsciiValue
    exifData["Exif.Canon.OwnerName"]    = "Tuan";                  // UShortValue
    exifData["Exif.CanonCs.LensType"]   = uint16_t(65535);         // LongValue
    Exiv2::Value::UniquePtr v = Exiv2::Value::create(Exiv2::asciiString);
    v->read("2013:06:09 14:30:30");
    Exiv2::ExifKey key("Exif.Image.DateTime");
    exifData.add(key, v.get());

    Exiv2::Image::UniquePtr writeTest = Exiv2::ImageFactory::open(file, useCurlFromExiv2TestApps);
    assert(writeTest.get() != 0);
    writeTest->setExifData(exifData);
    writeTest->writeMetadata();

    // read the result to make sure everything fine
    std::cout << "Print out the new metadata ...\n";
    Exiv2::Image::UniquePtr readTest = Exiv2::ImageFactory::open(file, useCurlFromExiv2TestApps);
    assert(readTest.get() != 0);
    readTest->readMetadata();
    Exiv2::ExifData &exifReadData = readTest->exifData();
    if (exifReadData.empty()) {
        std::string error(argv[1]);
        error += ": No Exif data found in the file";
        throw Exiv2::Error(Exiv2::kerErrorMessage, error);
    }
    auto end = exifReadData.end();
    for (auto i = exifReadData.begin(); i != end; ++i) {
        const char* tn = i->typeName();
        std::cout << std::setw(44) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << (tn ? tn : "Unknown") << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << i->count() << "  "
                  << std::dec << i->value()
                  << "\n";
    }


    // del, reset the metadata
    std::cout << "Reset ...\n";
    exifReadData["Exif.Photo.UserComment"]  = "Have a nice day";           // AsciiValue
    exifReadData["Exif.Image.Software"]     = "Exiv2.org";                 // AsciiValue
    exifReadData["Exif.Image.Copyright"]    = "Exiv2.org";                 // AsciiValue
    key = Exiv2::ExifKey("Exif.Image.Make");
    auto pos = exifReadData.findKey(key);
    if (pos == exifReadData.end()) throw Exiv2::Error(Exiv2::kerErrorMessage, "Exif.Image.Make not found");
    exifReadData.erase(pos);
    key = Exiv2::ExifKey("Exif.Image.DateTime");
    pos = exifReadData.findKey(key);
    if (pos == exifReadData.end()) throw Exiv2::Error(Exiv2::kerErrorMessage, "Exif.Image.DateTime not found");
    exifReadData.erase(pos);
    key = Exiv2::ExifKey("Exif.Canon.OwnerName");
    pos = exifReadData.findKey(key);
    if (pos == exifReadData.end()) throw Exiv2::Error(Exiv2::kerErrorMessage, "Exif.Canon.OwnerName not found");
    exifReadData.erase(pos);
    key = Exiv2::ExifKey("Exif.CanonCs.LensType");
    pos = exifReadData.findKey(key);
    if (pos == exifReadData.end()) throw Exiv2::Error(Exiv2::kerErrorMessage, "Exif.CanonCs.LensType not found");
    exifReadData.erase(pos);
    readTest->setExifData(exifReadData);
    readTest->writeMetadata();

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
