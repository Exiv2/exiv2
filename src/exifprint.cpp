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
  Abstract : Sample program to print the Exif metadata of an image

  File:      exifprint.cpp
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  Version  : $Name:  $ $Revision: 1.10 $
  History  : 26-Jan-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.10 $ $RCSfile: exifprint.cpp,v $")

// *****************************************************************************
// included header files
#include "exif.hpp"

// + standard includes
#include <iostream>
#include <iomanip>
#include <cstring>

// *****************************************************************************
// local declarations

using namespace Exif;

std::string readError(int rc, const char* file);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
try {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    ExifData exifData;
    int rc = exifData.read(argv[1]);
    if (rc) {
        std::string error = readError(rc, argv[1]);
        throw Error(error);
    }

    ExifData::const_iterator end = exifData.end();
    for (ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
        std::cout << std::setw(53) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << "  " 
                  << std::dec << i->value() 
                  << "\n";
    }

    return rc;
}
catch (Error& e) {
    std::cout << "Caught Exif exception '" << e << "'\n";
    return -1;
}

// *****************************************************************************
// local definitions

std::string readError(int rc, const char* file)
{
    std::string error;
    switch (rc) {
    case -1:
        error = "Couldn't open file `" + std::string(file) + "'";
        break;
    case 1:
        error = "Couldn't read from the input stream";
        break;
    case 2:
        error = "This does not look like a JPEG image";
        break;
    case 3:
        error = "No Exif data found in the file";
        break;
    case -99:
        error = "Unsupported Exif or GPS data found in IFD 1";
        break;
    default:
        error = "Reading Exif data failed, rc = " + toString(rc);
        break;
    }
    return error;
}
