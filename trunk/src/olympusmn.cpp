// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2005 Andreas Huggel <ahuggel@gmx.net>
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
  File:      olympusmn.cpp
  Version:   $Rev$
  Author(s): Will Stokes (wuz) <wstokes@gmail.com>
             Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   10-Mar-05, wuz: created
  Credits:   Olympus MakerNote implemented according to the specification
             in "???" by ???.
             <http://???>
 */

// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id: olympusmn.cpp 1 2005-03-10 10:04:00 wuz $");

// *****************************************************************************
// included header files
#include "types.hpp"
#include "olympusmn.hpp"
#include "makernote.hpp"
#include "value.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

// Define DEBUG_MAKERNOTE to output debug information to std::cerr
#undef DEBUG_MAKERNOTE

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const OlympusMakerNote::RegisterMakerNote OlympusMakerNote::register_;

    // Olympus Tag Info
    const TagInfo OlympusMakerNote::tagInfo_[] = {
        TagInfo(0x0200, "SpecialMode", "Picture taking mode (First Value: 0=normal, 2=fast, 3=panorama Second Value: sequence number Third Value: panorma direction (1=left to right, 2=right to left, 3 = bottom to top, 4=top to bottom)", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0201, "JpegQual", "JPEG quality 1=SQ 2=HQ 3=SHQ", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0202, "Macro", "Macro mode 0=normal, 1=macro", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0203, "0x0203", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0204, "DigiZoom", "Digital Zoom Ratio 0=normal 2=2x digital zoom", olympusIfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0205, "0x0205", "Unknown", olympusIfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0206, "0x0206", "Unknown", olympusIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0207, "SoftwareRelease", "Software firmware version", olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0208, "PictInfo", "ASCII format data such as [PictureInfo]", olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0209, "CameraID", "CameraID data", olympusIfdId, makerTags, undefined, printValue),
        TagInfo(0x0300, "0x0300", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0301, "0x0301", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0302, "0x0302", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0303, "0x0303", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0304, "0x0304", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0f00, "DataDump", "Various camera settings", olympusIfdId, makerTags, undefined, print0x0f00),
        TagInfo(0x1000, "0x1000", "Unknown", olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1001, "0x1001", "Unknown", olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1002, "0x1002", "Unknown", olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1003, "0x1003", "Unknown", olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1004, "0x1004", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1005, "0x1005", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1006, "0x1006", "Unknown", olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1007, "0x1007", "Unknown", olympusIfdId, makerTags, signedShort, printValue),
        TagInfo(0x1008, "0x1008", "Unknown", olympusIfdId, makerTags, signedShort, printValue),
        TagInfo(0x1009, "0x1009", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100a, "0x100a", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100b, "0x100b", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100c, "0x100c", "Unknown", olympusIfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x100d, "0x100d", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100e, "0x100e", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100f, "0x100f", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1010, "0x1010", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1011, "0x1011", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1012, "0x1012", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1013, "0x1013", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1014, "0x1014", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1015, "0x1015", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1016, "0x1016", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1017, "0x1017", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1018, "0x1018", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1019, "0x1019", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x101a, "0x101a", "Unknown", olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x101b, "0x101b", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101c, "0x101c", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101d, "0x101d", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101e, "0x101e", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101f, "0x101f", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1020, "0x1020", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1021, "0x1021", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1022, "0x1022", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1023, "0x1023", "Unknown", olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1024, "0x1024", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1025, "0x1025", "Unknown", olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1026, "0x1026", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1027, "0x1027", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1028, "0x1028", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1029, "0x1029", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102a, "0x102a", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102b, "0x102b", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102c, "0x102c", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102d, "0x102d", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102e, "0x102e", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x102f, "0x102f", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1030, "0x1030", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1031, "0x1031", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1032, "0x1032", "Unknown", olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1033, "0x1033", "Unknown", olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1034, "0x1034", "Unknown", olympusIfdId, makerTags, unsignedRational, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownOlympusMakerNoteTag)", "Unknown OlympusMakerNote tag", olympusIfdId, makerTags, invalidTypeId, printValue)
    };

    OlympusMakerNote::OlympusMakerNote(bool alloc)
        : IfdMakerNote(olympusIfdId, alloc)
    {
        byte buf[] = {
            'O', 'L', 'Y', 'M', 'P', 0x00, 0x01, 0x00
        };
        readHeader(buf, 8, byteOrder_);
    }

    OlympusMakerNote::OlympusMakerNote(const OlympusMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int OlympusMakerNote::readHeader(const byte* buf,
                                   long len, 
                                   ByteOrder byteOrder)
    {
        if (len < 8) return 1;
  
        // Copy the header
        header_.alloc(8);
        memcpy(header_.pData_, buf, header_.size_);
        // Adjust the offset of the IFD for the prefix
        adjOffset_ = 8;

        return 0;
    }

    int OlympusMakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the OLYMPUS prefix
        if (   header_.size_ < 8
            || std::string(reinterpret_cast<char*>(header_.pData_), 5) 
               != std::string("OLYMP", 5)) {
            rc = 2;
        }
        return rc;
    }

    OlympusMakerNote::AutoPtr OlympusMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    OlympusMakerNote* OlympusMakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote = AutoPtr(new OlympusMakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    OlympusMakerNote::AutoPtr OlympusMakerNote::clone() const
    {
        return AutoPtr(clone());
    }

    OlympusMakerNote* OlympusMakerNote::clone_() const 
    {
        return new OlympusMakerNote(*this); 
    }

    std::ostream& OlympusMakerNote::print0x0f00(std::ostream& os, 
                                                const Value& value)
    {
        if (value.typeId() != undefined) return os << value;

        long count = value.count();
        long lA, lB;
        
        if (count < 11) return os;
        lA = value.toLong(11);
        os << std::setw(23) << "\n   Function ";
        print0x0f00_011(os, lA);
        
        if (count < 138) return os;
        lA = value.toLong(138);
        os << std::setw(23) << "\n   White balance mode ";
        print0x0f00_138(os, lA);

        if (count < 150) return os;
        lA = value.toLong(150);
        lB = value.toLong(151);
        os << std::setw(23) << "\n   Sharpness ";
        print0x0f00_150_151(os, lA, lB);
      
        // Meaning of any further ushorts is unknown - ignore them
        return os;

    } // OlympusMakerNote::print0x0f00
    
    std::ostream& OlympusMakerNote::print0x0f00_011(std::ostream& os, long l)
    {
        switch (l) {
        case  0: os << "Off";             break;
        case  1: os << "Black and White"; break;
        case  2: os << "Sepia";           break;
        case  3: os << "White Board";     break;
        case  4: os << "Black Board";     break;
        default: os << "Unknown (" << l << ")";
        }
        return os;
    }

    std::ostream& OlympusMakerNote::print0x0f00_138(std::ostream& os, long l)
    {
        switch (l) {
        case 0:  os << "Auto";        break;
        case 16: os << "Daylight";    break;
        case 32: os << "Tungsten";    break;
        case 48: os << "Fluorescent"; break;
        case 64: os << "Shade";       break;
        default: os << "Unknown (" << l << ")";
        }
        return os;
    }
    
    std::ostream& OlympusMakerNote::print0x0f00_150_151(std::ostream& os, 
                                                        long l150, long l151)
    {
        if( l150 == 24 && l151 == 6 ) {
            os << "Soft"; 
        }
        else if ((l150 == 32 && l151 == 12) ||
                 (l150 == 30 && l151 == 11) ||
                 (l150 == 24 && l151 == 8)) {
            os << "Normal"; 
        }
        else if ((l150 == 40 && l151 == 16) ||
                 (l150 == 38 && l151 == 15) ||
                 (l150 == 32 && l151 == 12)) {
            os << "Hard"; 
        }
        else {
            os << "Unknown"; 
        }
        return os;
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createOlympusMakerNote(bool alloc,
                                              const byte* buf, 
                                              long len, 
                                              ByteOrder byteOrder, 
                                              long offset)
    {
        return MakerNote::AutoPtr(new OlympusMakerNote(alloc));
    }

}                                       // namespace Exiv2
