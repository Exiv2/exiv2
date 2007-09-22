// ***************************************************************** -*- C++ -*-
// xmpsample.cpp, $Rev$
// Sample/test for high level XMP classes

#include "value.hpp"
#include "xmp.hpp"
#include "error.hpp"

#include <string>
#include <iostream>
#include <iomanip>

using namespace Exiv2;

int main()
try {
    // The XMP property container
    Exiv2::XmpData xmpData;

    // Add a simple XMP property in a known namespace    
    Exiv2::Value::AutoPtr v = Exiv2::Value::create(xmpText);
    v->read("image/jpeg");
    xmpData.add(Exiv2::XmpKey("Xmp.dc.format"), v.get());

    // Add an ordered array of text values
    v = Exiv2::Value::create(xmpSeq);
    v->read("1) The first creator");  // the sequence in which the array elements
    v->read("2) The second creator"); // are added is relevant
    v->read("3) And another one");
    xmpData.add(Exiv2::XmpKey("Xmp.dc.creator"), v.get());

    // Add a language alternative property
    v = Exiv2::Value::create(langAlt);
    v->read("lang=de-DE Hallo, Welt"); // the default doesn't need a qualifier
    v->read("Hello, World");           // and it will become the first element
    xmpData.add(Exiv2::XmpKey("Xmp.dc.description"), v.get());

    // Output XMP properties
    for (Exiv2::XmpData::const_iterator md = xmpData.begin(); 
         md != xmpData.end(); ++md) {
        std::cout << std::setfill(' ') << std::left
                  << std::setw(44)
                  << md->key() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << md->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << md->count() << "  "
                  << std::dec << md->value()
                  << std::endl;
    }

    // Serialize the XMP data and output the XMP packet
    std::string xmpPacket;
    if (0 != Exiv2::XmpParser::encode(xmpPacket, xmpData)) {
        throw Exiv2::Error(1, "Failed to serialize XMP data");
    }
    std::cout << xmpPacket << "\n";

    // Cleanup
    Exiv2::XmpParser::terminate();
    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
