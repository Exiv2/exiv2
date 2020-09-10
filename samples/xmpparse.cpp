// ***************************************************************** -*- C++ -*-
// xmpparse.cpp
// Read an XMP packet from a file, parse it and print all (known) properties.

#include <exiv2/exiv2.hpp>

#include <string>
#include <iostream>
#include <iomanip>

int main(int argc, char* const argv[])
try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    Exiv2::DataBuf buf = Exiv2::readFile(argv[1]);
    std::string xmpPacket;
    xmpPacket.assign(reinterpret_cast<char*>(buf.pData_), buf.size_);
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
    for (const auto& md : xmpData) {
        std::cout << std::setfill(' ') << std::left << std::setw(44) << md.key() << " " << std::setw(9)
                  << std::setfill(' ') << std::left << md.typeName() << " " << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right << md.count() << "  " << std::dec << md.value() << std::endl;
    }
    Exiv2::XmpParser::terminate();
    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
