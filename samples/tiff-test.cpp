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

// tiff-test.cpp
// First and very simple TIFF write test.

#include <exiv2/exiv2.hpp>
#include <enforce.hpp>

#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace Exiv2;

void print(const ExifData& exifData);

void mini1(const char* path);
void mini9(const char* path);

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

    const char* path = argv[1];
    mini1(path);
    mini9(path);

    return 0;
}
catch (const AnyError& e) {
    std::cout << e << "\n";
}

void mini1(const char* path)
{
    ExifData exifData;
    Blob blob;
    WriteMethod wm;

    // Write nothing to a new structure, without a previous binary image
    wm = ExifParser::encode(blob, nullptr, 0, bigEndian, exifData);
    enforce(wm == wmIntrusive, Exiv2::kerErrorMessage, "encode returned an unexpected value");
    assert(blob.empty());
    std::cout << "Test 1: Writing empty Exif data without original binary data: ok.\n";

    // Write nothing, this time with a previous binary image
    DataBuf buf = readFile(path);
    wm = ExifParser::encode(blob, buf.pData_, buf.size_, bigEndian, exifData);
    enforce(wm == wmIntrusive, Exiv2::kerErrorMessage, "encode returned an unexpected value");
    assert(blob.empty());
    std::cout << "Test 2: Writing empty Exif data with original binary data: ok.\n";

    // Write something to a new structure, without a previous binary image
    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";
    wm = ExifParser::encode(blob, nullptr, 0, bigEndian, exifData);
    enforce(wm == wmIntrusive, Exiv2::kerErrorMessage, "encode returned an unexpected value");
    std::cout << "Test 3: Wrote non-empty Exif data without original binary data:\n";
    exifData.clear();
    ByteOrder bo = ExifParser::decode(exifData, &blob[0], static_cast<uint32_t>(blob.size()));
    enforce(bo == bigEndian, Exiv2::kerErrorMessage, "decode returned an unexpected value");
    print(exifData);
}

void mini9(const char* path)
{
    TiffImage tiffImage(BasicIo::UniquePtr(new FileIo(path)), false);
    tiffImage.readMetadata();

    std::cout << "MIME type:  " << tiffImage.mimeType() << "\n";
    std::cout << "Image size: " << tiffImage.pixelWidth() << " x " << tiffImage.pixelHeight() << "\n";

    ExifData& exifData = tiffImage.exifData();
    std::cout << "Before\n";
    print(exifData);
    std::cout << "======\n";

    exifData["Exif.Photo.DateTimeOriginal"] = "Yesterday at noon";

    std::cout << "After\n";
    print(exifData);
    tiffImage.writeMetadata();
}

void print(const ExifData& exifData)
{
    if (exifData.empty()) {
        std::string error("No Exif data found in the file");
        throw Exiv2::Error(kerErrorMessage, error);
    }
    auto end = exifData.end();
    for (auto i = exifData.begin(); i != end; ++i) {
        std::cout << std::setw(44) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << i->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << i->count() << "  "
                  << std::dec << i->value()
                  << "\n";
    }
}
