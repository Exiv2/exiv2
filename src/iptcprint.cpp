// ***************************************************************** -*- C++ -*-
// iptcprint.cpp, $Rev$
// Sample program to print the Iptc metadata of an image

#include "iptc.hpp"
#include <iostream>
#include <iomanip>

int main(int argc, char* const argv[])
try {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    Exiv2::IptcData iptcData;
    int rc = iptcData.read(argv[1]);
    if (rc) {
        std::string error = Exiv2::IptcData::strError(rc, argv[1]);
        throw Exiv2::Error(error);
    }

    Exiv2::IptcData::iterator end = iptcData.end();
    for (Exiv2::IptcData::iterator md = iptcData.begin(); md != end; ++md) {
        std::cout << std::setw(36) << std::setfill(' ') << std::left
                  << md->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << md->tag() << "  " 
                  << std::setw(9) << std::setfill(' ') << std::left
                  << md->typeName() << " "
                  << std::dec << std::setw(3) 
                  << std::setfill(' ') << std::right
                  << md->count() << " "
                  << std::dec << md->value() 
                  << std::endl;
    }

    return rc;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
