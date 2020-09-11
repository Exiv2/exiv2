// ***************************************************************** -*- C++ -*-
// iptcprint.cpp
// Sample program to print the IPTC metadata of an image

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

int main(int argc, char* const argv[])
try {

    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(argv[1]);
    assert (image.get() != nullptr);
    image->readMetadata();

    Exiv2::IptcData &iptcData = image->iptcData();
    if (iptcData.empty()) {
        std::string error(argv[1]);
        error += ": No IPTC data found in the file";
        throw Exiv2::Error(Exiv2::kerErrorMessage, error);
    }

    auto end = iptcData.end();
    for (auto md = iptcData.begin(); md != end; ++md) {
        std::cout << std::setw(44) << std::setfill(' ') << std::left
                  << md->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << md->tag() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << md->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << md->count() << "  "
                  << std::dec << md->value()
                  << std::endl;
    }

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
