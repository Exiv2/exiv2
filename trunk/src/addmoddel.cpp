// ***************************************************************** -*- C++ -*-
/*
  Abstract:  Sample program showing how to add, modify and delete Exif metadata.

  File:      addmoddel.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
 */
// *****************************************************************************
// included header files
#include "exif.hpp"
#include <iostream>
#include <iomanip>

// *****************************************************************************
// Main
int main()
try {
    // Container for all metadata
    Exiv2::ExifData exifData;

    // *************************************************************************
    // Add to the Exif data

    // Create a ASCII string value (note the use of create)
    Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::asciiString);
    // Set the value to a string
    v->read("1999:12:31 23:59:59");
    // Add the value together with its key to the Exif data container
    Exiv2::ExifKey key("Exif.Photo.DateTimeOriginal");
    exifData.add(key, v.get());
    std::cout << "Added key \"" << key << "\", value \"" << *v << "\"\n";

    // Now create a more interesting value (without using the create method)
    Exiv2::URationalValue::AutoPtr rv(new Exiv2::URationalValue);
    // Set two rational components from a string
    rv->read("1/2 1/3");
    // Add more elements through the extended interface of rational value 
    rv->value_.push_back(std::make_pair(2,3));
    rv->value_.push_back(std::make_pair(3,4));
    // Add the key and value pair to the Exif data
    key = Exiv2::ExifKey("Exif.Image.PrimaryChromaticities");
    exifData.add(key, rv.get());
    std::cout << "Added key \"" << key << "\", value \"" << *rv << "\"\n";

    // *************************************************************************
    // Modify Exif data

    // Find the timestamp metadatum by its key
    key = Exiv2::ExifKey("Exif.Photo.DateTimeOriginal");
    Exiv2::ExifData::iterator pos = exifData.findKey(key);
    if (pos == exifData.end()) throw Exiv2::Error("Key not found");
    // Modify the value
    std::string date = pos->toString();
    date.replace(0, 4, "2000");
    pos->setValue(date);
    std::cout << "Modified key \"" << key 
              << "\", new value \"" << pos->value() << "\"\n";

    // Find the other key
    key = Exiv2::ExifKey("Exif.Image.PrimaryChromaticities");
    pos = exifData.findKey(key);
    if (pos == exifData.end()) throw Exiv2::Error("Key not found");
    // Get a pointer to a copy of the value
    v = pos->getValue();
    // Downcast the Value pointer to its actual type
    Exiv2::URationalValue* prv = dynamic_cast<Exiv2::URationalValue*>(v.release());
    if (prv == 0) throw Exiv2::Error("Downcast failed");
    rv = Exiv2::URationalValue::AutoPtr(prv);
    // Modify the value directly through the interface of URationalValue
    rv->value_[2] = std::make_pair(88,77);
    // Copy the modified value back to the metadatum
    pos->setValue(rv.get());
    std::cout << "Modified key \"" << key 
              << "\", new value \"" << pos->value() << "\"\n";

    // *************************************************************************
    // Delete metadata from the Exif data container

    // Delete the metadatum at iterator position pos
    key = Exiv2::ExifKey("Exif.Image.PrimaryChromaticities");
    pos = exifData.findKey(key);
    if (pos == exifData.end()) throw Exiv2::Error("Key not found");
    exifData.erase(pos);
    std::cout << "Deleted key \"" << key << "\"\n";

    // *************************************************************************
    // Finally, write the remaining Exif data to an image file
    int rc = exifData.write("img_2158.jpg");
    if (rc) {
        std::string error = Exiv2::ExifData::strError(rc, "img_2158.jpg");
        throw Exiv2::Error(error);
    }

    return 0;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
