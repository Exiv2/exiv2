// ***************************************************************** -*- C++ -*-
/*
  Abstract : Sample program to print the Iptc metadata of an image

  File     : iptcprint.cpp
  Version  : $Rev$
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History  : 31-Jul-04, brad: created
 */
// *****************************************************************************
// included header files
#include "iptc.hpp"
#include <iostream>
#include <iomanip>

// *****************************************************************************
// Main
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
    for (Exiv2::IptcData::iterator i = iptcData.begin(); i != end; ++i) {
        std::cout << std::setw(40) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << "  " 
                  << std::dec << i->value() 
                  << "\n";
    }

    return rc;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
