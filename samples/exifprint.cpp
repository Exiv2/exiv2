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

#include <exiv2/exiv2.hpp>
#include <iostream>
#include <iomanip>
#include <set>
#include <cassert>

// https://github.com/Exiv2/exiv2/issues/468
#if defined(EXV_UNICODE_PATH) && defined(__MINGW__)
#undef  EXV_UNICODE_PATH
#endif

#ifdef  EXV_UNICODE_PATH
#define _tchar      wchar_t
#define _tstrcmp    wcscmp
#define _t(s)       L##s
#define _tcout      wcout
#define _tmain      wmain
#else
#define _tchar      char
#define _tstrcmp    strcmp
#define _t(s)       s
#define _tcout      cout
#define _tmain      main
#endif

// copied from src/tiffvisitor_int.cpp
static const Exiv2::TagInfo* findTag(const Exiv2::TagInfo* pList,uint16_t tag)
{
    while ( pList->tag_ != 0xffff && pList->tag_ != tag ) pList++;
    return  pList->tag_ != 0xffff  ? pList : NULL;
}


int _tmain(int argc, _tchar* const argv[])
try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    const _tchar* prog = argv[0];
    if (argc == 1) {
        std::_tcout << _t("Usage: ") << prog << _t(" [ [--lint] path | --version | --version-test ]") << std::endl;
        return 1;
    }

    int    rc          = 0      ;
    const _tchar* file = argv[1];
    bool   bLint       = _tstrcmp(file,_t("--lint")) == 0 && argc == 3;
    if (   bLint ) file= argv[2];


    if ( _tstrcmp(file,_t("--version")) == 0 ) {
        exv_grep_keys_t keys;
        Exiv2::dumpLibraryInfo(std::cout,keys);
        return rc;
    } else if ( _tstrcmp(file,_t("--version-test")) == 0 ) {
        // verifies/test macro EXIV2_TEST_VERSION
        // described in include/exiv2/version.hpp
        std::cout << "EXV_PACKAGE_VERSION             " << EXV_PACKAGE_VERSION             << std::endl
                  << "Exiv2::version()                " << Exiv2::version()                << std::endl
                  << "strlen(Exiv2::version())        " << ::strlen(Exiv2::version())      << std::endl
                  << "Exiv2::versionNumber()          " << Exiv2::versionNumber()          << std::endl
                  << "Exiv2::versionString()          " << Exiv2::versionString()          << std::endl
                  << "Exiv2::versionNumberHexString() " << Exiv2::versionNumberHexString() << std::endl
                  ;

        // Test the Exiv2 version available at runtime but compile the if-clause only if
        // the compile-time version is at least 0.15. Earlier versions didn't have a
        // testVersion() function:
        #if EXIV2_TEST_VERSION(0,15,0)
            if (Exiv2::testVersion(0,13,0)) {
              std::cout << "Available Exiv2 version is equal to or greater than 0.13\n";
            } else {
              std::cout << "Installed Exiv2 version is less than 0.13\n";
            }
        #else
              std::cout << "Compile-time Exiv2 version doesn't have Exiv2::testVersion()\n";
        #endif
        return rc;
    }

    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(file);
    assert(image.get() != 0);
    image->readMetadata();

    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
        std::string error("No Exif data found in file");
        throw Exiv2::Error(Exiv2::kerErrorMessage, error);
    }
    
    std::set<std::string> shortLong;
    shortLong.insert("Exif.Photo.PixelXDimension");
    shortLong.insert("Exif.Photo.PixelYDimension");
    shortLong.insert("Exif.Photo.ImageLength");
    shortLong.insert("Exif.Photo.ImageWidth);
    shortLong.insert("Exif.Photo.RowsPerStrip);
    shortLong.insert("Exif.Photo.StripOffsets);
    shortLong.insert("Exif.Photo.StripByteCounts);

    auto end = exifData.end();
    for (auto i = exifData.begin(); i != end; ++i) {
        if ( ! bLint ) {
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
                      << std::dec << i->toString()
                      << "\n";
        } else {
            const Exiv2::TagInfo* tagInfo = findTag( Exiv2::ExifTags::tagList(i->groupName()),i->tag() );
            if ( tagInfo ) {
                Exiv2::TypeId type = i->typeId();
                if (          type!= tagInfo-> typeId_ 
                &&!(tagInfo->typeId_ == Exiv2::comment && type == Exiv2::undefined) // comment is stored as undefined
                &&!(shortLong.find(i->key())!=shortLong.end() &&(type == Exiv2::unsignedShort||type==Exiv2::unsignedLong)) // can be short or long!
                ) {
                  std::cerr << i->key()
                            << " type " << i->typeName() << " (" << type << ")" 
                            << " expected " << Exiv2::TypeInfo::typeName(tagInfo-> typeId_) << " (" << tagInfo-> typeId_ << ")"
                            << std::endl;
                  rc=2;
                }
            }
        }
    }

    return rc;
}
//catch (std::exception& e) {
//catch (Exiv2::AnyError& e) {
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e.what() << "'\n";
    return -1;
}
