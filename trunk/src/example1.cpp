// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
 * 
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
  Abstract:  Sample code to add, modify and delete Exif metadata

  File:      example1.cpp
  Version:   $Name:  $ $Revision: 1.2 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.2 $ $RCSfile: example1.cpp,v $")

// *****************************************************************************
// included header files
#include "exif.hpp"
#include <iostream>
#include <iomanip>

// *****************************************************************************
// local declarations

using namespace Exif;

void exifPrint(const ExifData& exifData);

// *****************************************************************************
// Main
int main()
try {
    // Container for all metadata
    ExifData exifData;

    // *************************************************************************
    // Add to the Exif data

    // Create a value of the required type
    Value* v = Value::create(asciiString);
    // Set the value to a string
    v->read("1999:12:31 23:59:59");
    // Add the value together with its key to the Exif data container
    std::string key = "Image.DateTime.DateTimeOriginal";
    exifData.add(key, v);

    std::cout << "Added key \"" << key << "\", value \"" << *v << "\"\n";
    // Delete the memory allocated by Value::create
    delete v;

    // Now create a more interesting value
    v = Value::create(unsignedRational);
    // Set two rational components from a string
    v->read("1/2 1/3");
    // Downcast the Value to its actual type
    URationalValue* rv = dynamic_cast<URationalValue*>(v);
    if (rv == 0) throw Error("Downcast failed");
    // Add more elements through the extended interface of the actual type
    rv->value_.push_back(std::make_pair(2,3));
    rv->value_.push_back(std::make_pair(3,4));
    // Add the key and value pair to the Exif data
    key = "Image.ImageCharacteristics.PrimaryChromaticities";
    exifData.add(key, rv);

    std::cout << "Added key \"" << key << "\", value \"" << *v << "\"\n";
    // Delete the memory allocated by Value::create
    delete v;

    // *************************************************************************
    // Modify Exif data

    // Find the timestamp metadatum by its key
    key = "Image.DateTime.DateTimeOriginal";
    ExifData::iterator pos = exifData.findKey(key);
    if (pos == exifData.end()) throw Error("Key not found");
    // Modify the value
    std::string date = pos->toString();
    date.replace(0,4,"2000");
    pos->setValue(date); 

    // Find the other key
    key = "Image.ImageCharacteristics.PrimaryChromaticities";
    pos = exifData.findKey(key);
    if (pos == exifData.end()) throw Error("Key not found");
    // Get a pointer to a copy of the value
    v = pos->getValue();
    // Downcast the Value pointer to its actual type
    rv = dynamic_cast<URationalValue*>(v);
    if (rv == 0) throw Error("Downcast failed");
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
    if (pos == exifData.end()) throw Error("Key not found");
    exifData.erase(pos);

    std::cout << "---\n";
    exifPrint(exifData);

    return 0;
}
catch (Error& e) {
    std::cout << "Caught Exif exception '" << e << "'\n";
    return 1;
}

// *****************************************************************************
// local definitions

void exifPrint(const ExifData& exifData)
{
    ExifData::const_iterator beg = exifData.begin();
    ExifData::const_iterator end = exifData.end();
    ExifData::const_iterator i = beg;
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
