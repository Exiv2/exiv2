// ***************************************************************** -*- C++ -*-
// xmpdump.cpp
// Sample program to dump the XMP packet of an image

#include <cassert>
#include <iostream>
#include <string>

#include "error.hpp"
#include "image.hpp"

int main(int argc, char* const argv[])
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    try {
        if (argc != 2) {
            std::cout << "Usage: " << argv[0] << " file\n";
            return 1;
        }

        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(argv[1]);
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
