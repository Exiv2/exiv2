// ***************************************************************** -*- C++ -*-
// xmpsample.cpp, $Rev$
// Sample/test for high level XMP classes. See also addmoddel.cpp

#include "xmp.hpp"
#include "error.hpp"

#include <string>
#include <iostream>
#include <iomanip>

int main()
try {
    // The XMP property container
    Exiv2::XmpData xmpData;

    // -------------------------------------------------------------------------
    // Exiv2 has specialized values for simple XMP properties, arrays of simple
    // properties and language alternatives.

    // Add a simple XMP property in a known namespace    
    Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::xmpText);
    v->read("image/jpeg");
    xmpData.add(Exiv2::XmpKey("Xmp.dc.format"), v.get());

    // Add an ordered array of text values.
    v = Exiv2::Value::create(Exiv2::xmpSeq); // or xmpBag or xmpAlt.
    v->read("1) The first creator");         // The sequence in which the array
    v->read("2) The second creator");        // elements are added is their
    v->read("3) And another one");           // order in the array.
    xmpData.add(Exiv2::XmpKey("Xmp.dc.creator"), v.get());

    // Add a language alternative property
    v = Exiv2::Value::create(Exiv2::langAlt);
    v->read("lang=de-DE Hallo, Welt");       // The default doesn't need a 
    v->read("Hello, World");                 // qualifier
    xmpData.add(Exiv2::XmpKey("Xmp.dc.description"), v.get());

    // Or all of the above but using associative array style syntax
    xmpData["Xmp.dc.format"] = "image/tiff";      // overwrites existing value
    xmpData["Xmp.dc.subject"] = "Palmtree";       // adds a new array value 
    xmpData["Xmp.dc.subject"] = "Rubbertree";     // adds a second array item
    xmpData["Xmp.dc.title"] = "lang=en-US Beach"; // a language alternative

    // -------------------------------------------------------------------------
    // Register a namespace which Exiv2 doesn't know yet. This is only needed
    // when properties are added manually. If the XMP metadata is read from an
    // image, namespaces are decoded and registered at the same time.
    Exiv2::XmpProperties::registerNs("myNamespace/", "ns");

    // -------------------------------------------------------------------------
    // There are no specialized values for structures, qualifiers and nested
    // types. However, these can be added by using a XmpTextValue and a path as
    // the key.

    // Add a structure
    Exiv2::XmpTextValue tv("16");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpDM.videoFrameSize/stDim:w"), &tv);
    tv.read("9");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpDM.videoFrameSize/stDim:h"), &tv);
    tv.read("inch");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpDM.videoFrameSize/stDim:unit"), &tv);

    // Add an element with a qualifier (using the namespace registered earlier)
    tv.read("James Bond");
    xmpData.add(Exiv2::XmpKey("Xmp.dc.publisher"), &tv);
    tv.read("secret agent");
    xmpData.add(Exiv2::XmpKey("Xmp.dc.publisher/?ns:role"), &tv);

    // Add a qualifer to an array element of Xmp.dc.creator (added above)
    tv.read("programmer");
    xmpData.add(Exiv2::XmpKey("Xmp.dc.creator[2]/?ns:role"), &tv);

    // Add an array of structures
    tv.read("");
    tv.setXmpArrayType(Exiv2::XmpValue::xaBag);
    xmpData.add(Exiv2::XmpKey("Xmp.xmpBJ.JobRef"), &tv); // Set the array type.
    tv.setXmpArrayType(Exiv2::XmpValue::xaNone);

    tv.read("Birtday party");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpBJ.JobRef[1]/stJob:name"), &tv);
    tv.read("Photographer");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpBJ.JobRef[1]/stJob:role"), &tv);

    tv.read("Wedding ceremony");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpBJ.JobRef[2]/stJob:name"), &tv);
    tv.read("Best man");
    xmpData.add(Exiv2::XmpKey("Xmp.xmpBJ.JobRef[2]/stJob:role"), &tv);

    // -------------------------------------------------------------------------
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

    // -------------------------------------------------------------------------
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
