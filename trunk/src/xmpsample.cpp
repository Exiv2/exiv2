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
    // Teaser: Setting XMP properties doesn't get much easier than this:

    xmpData["Xmp.dc.source"]  = "xmpsample.cpp";    // a simple text value
    xmpData["Xmp.dc.subject"] = "Palmtree";         // an array item
    xmpData["Xmp.dc.subject"] = "Rubbertree";       // add a 2nd array item
    xmpData["Xmp.dc.title"]   = "lang=en-US Beach"; // a language alternative

    // -------------------------------------------------------------------------
    // Any properties can be set provided the namespace is known. Values of any
    // type can be assigned to an Xmpdatum, if they have an output operator. The
    // default XMP value type for unknown properties is a simple text value.

    xmpData["Xmp.dc.one"]     = -1;
    xmpData["Xmp.dc.two"]     = 3.1415;
    xmpData["Xmp.dc.three"]   = Exiv2::Rational(5, 7);
    xmpData["Xmp.dc.four"]    = uint16_t(255);
    xmpData["Xmp.dc.five"]    = int32_t(256);
    xmpData["Xmp.dc.six"]     = false;

    // In addition, there is a dedicated assignment operator for Exiv2::Value
    Exiv2::XmpTextValue val("Seven");
    xmpData["Xmp.dc.seven"]   = val;

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

    // According to the XMP specification, Xmp.tiff.ImageDescription is an
    // alias for Xmp.dc.description. Exiv2 treats an alias just like any
    // other property and leaves it to the application to implement specific
    // behaviour if desired.
    xmpData["Xmp.tiff.ImageDescription"] = "TIFF image description";
    xmpData["Xmp.tiff.ImageDescription"] = "lang=de-DE TIFF Bildbeschreibung";

    // -------------------------------------------------------------------------
    // Register a namespace which Exiv2 doesn't know yet. This is only needed
    // when properties are added manually. If the XMP metadata is read from an
    // image, namespaces are decoded and registered at the same time.
    Exiv2::XmpProperties::registerNs("myNamespace/", "ns");

    // -------------------------------------------------------------------------
    // There are no specialized values for structures, qualifiers and nested
    // types. However, these can be added by using an XmpTextValue and a path as
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
    tv.read("");                                         // Clear the value
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
