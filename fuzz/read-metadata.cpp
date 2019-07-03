// ***************************************************************** -*- C++ -*-
// exifprint.cpp
// Sample program to print the Exif metadata of an image

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
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

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * Data, size_t Size)
try {
    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(Data, Size);
    assert(image.get() != 0);
    image->readMetadata();

    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
        std::string error("No Exif data found in file");
        throw Exiv2::Error(Exiv2::kerErrorMessage, error);
    }

    Exiv2::ExifData::const_iterator end = exifData.end();
    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
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

    return 0;
}
//catch (std::exception& e) {
//catch (Exiv2::AnyError& e) {
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e.what() << "'\n";
    return -1;
}
