// ***************************************************************** -*- C++ -*-
/*
  Abstract:  Example showing how to add, modify and delete Exif metadata

  File:      example1.cpp
  Version:   $Name:  $ $Revision: 1.4 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
 */
// *****************************************************************************
// included header files
#include "exif.hpp"
#include <iostream>
#include <iomanip>

// *****************************************************************************
// local declarations

// Simple Exif data print function
void exifPrint(const Exif::ExifData& exifData);

// *****************************************************************************
// Main
int main()
try {
    // Container for all metadata
    Exif::ExifData exifData;

    // *************************************************************************
    // Add to the Exif data

    // Create a value of the required type
    Exif::Value* v = Exif::Value::create(Exif::asciiString);
    // Set the value to a string
    v->read("1999:12:31 23:59:59");
    // Add the value together with its key to the Exif data container
    std::string key = "Image.DateTime.DateTimeOriginal";
    exifData.add(key, v);

    std::cout << "Added key \"" << key << "\", value \"" << *v << "\"\n";
    // Delete the memory allocated by Value::create
    delete v;

    // Now create a more interesting value
    Exif::URationalValue* rv = new Exif::URationalValue;
    // Set two rational components from a string
    rv->read("1/2 1/3");
    // Add more elements through the extended interface of rational value 
    rv->value_.push_back(std::make_pair(2,3));
    rv->value_.push_back(std::make_pair(3,4));
    // Add the key and value pair to the Exif data
    key = "Image.ImageCharacteristics.PrimaryChromaticities";
    exifData.add(key, rv);

    std::cout << "Added key \"" << key << "\", value \"" << *rv << "\"\n";
    // Delete memory allocated on the heap
    delete rv;

    // *************************************************************************
    // Modify Exif data

    // Find the timestamp metadatum by its key
    key = "Image.DateTime.DateTimeOriginal";
    Exif::ExifData::iterator pos = exifData.findKey(key);
    if (pos == exifData.end()) throw Exif::Error("Key not found");
    // Modify the value
    std::string date = pos->toString();
    date.replace(0,4,"2000");
    pos->setValue(date); 

    // Find the other key
    key = "Image.ImageCharacteristics.PrimaryChromaticities";
    pos = exifData.findKey(key);
    if (pos == exifData.end()) throw Exif::Error("Key not found");
    // Get a pointer to a copy of the value
    v = pos->getValue();
    // Downcast the Value pointer to its actual type
    rv = dynamic_cast<Exif::URationalValue*>(v);
    if (rv == 0) throw Exif::Error("Downcast failed");
    // Modify elements through the extended interface of the actual type
    rv->value_[2] = std::make_pair(88,77);
    // Copy the modified value back to the metadatum
    pos->setValue(rv);
    // Delete the memory allocated by getValue
    delete v;

    std::cout << "---\n";
    exifPrint(exifData);

    // *************************************************************************
    // Delete metadata from the Exif data container

    // Delete a metadatum by its key
    key = "Image.DateTime.DateTimeOriginal";
    exifData.erase(key);

    // Delete the metadatum at iterator position pos
    key = "Image.ImageCharacteristics.PrimaryChromaticities";
    pos = exifData.findKey(key);
    if (pos == exifData.end()) throw Exif::Error("Key not found");
    exifData.erase(pos);

    std::cout << "---\n";
    exifPrint(exifData);

    return 0;
}
catch (Exif::Error& e) {
    std::cout << "Caught Exif exception '" << e << "'\n";
    return 1;
}

// *****************************************************************************
// local definitions

void exifPrint(const Exif::ExifData& exifData)
{
    Exif::ExifData::const_iterator beg = exifData.begin();
    Exif::ExifData::const_iterator end = exifData.end();
    Exif::ExifData::const_iterator i = beg;
    for (; i != end; ++i) {
        std::cout << "0x" 
                  << std::hex << std::setw(4) << std::setfill('0') << std::right
                  << i->tag() << " " 
                  << std::setw(27) << std::setfill(' ') << std::left
                  << i->tagName() << " "
                  << std::setw(17) << std::setfill(' ') << std::left
                  << i->typeName() << " "
                  << std::dec << std::setw(3) 
                  << std::setfill(' ') << std::right
                  << i->count() << "   " 
                  << std::dec << i->value() << "\n";
    }
}
