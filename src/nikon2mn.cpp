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
  File:      nikon2mn.cpp
  Version:   $Name:  $ $Revision: 1.1 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   17-May-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.1 $ $RCSfile: nikon2mn.cpp,v $")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "nikon2mn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "image.hpp"

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

    const Nikon2MakerNote::RegisterMakerNote Nikon2MakerNote::register_;

    // Nikon2 MakerNote Tag Info
    static const MakerNote::MnTagInfo nikon2MnTagInfo[] = {
        MakerNote::MnTagInfo(0x0003, "Quality", "Image quality setting"),
        MakerNote::MnTagInfo(0x0004, "ColorMode", "Color mode"),
        MakerNote::MnTagInfo(0x0005, "ImageAdjustment", "Image adjustment setting"),
        MakerNote::MnTagInfo(0x0006, "ISOSpeed", "ISO speed setting"),
        MakerNote::MnTagInfo(0x0007, "WhiteBalance", "White balance"),
        MakerNote::MnTagInfo(0x0008, "Focus", "Focus mode"),
        MakerNote::MnTagInfo(0x000a, "DigitalZoom", "Digital zoom setting"),
        MakerNote::MnTagInfo(0x000b, "Adapter", "Adapter used"),
        // End of list marker
        MakerNote::MnTagInfo(0xffff, "(UnknownNikon2MnTag)", "Unknown Nikon2MakerNote tag")
    };

    Nikon2MakerNote::Nikon2MakerNote(bool alloc)
        : IfdMakerNote(nikon2MnTagInfo, alloc), sectionName_("Nikon2")
    {
    }

    int Nikon2MakerNote::readHeader(const char* buf,
                                  long len, 
                                  ByteOrder byteOrder)
    {
        if (len < 8) return 1;

        header_.alloc(8);
        memcpy(header_.pData_, buf, header_.size_);
        adjOffset_ = 8;
        return 0;
    }

    int Nikon2MakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the Nikon prefix
        if (   header_.size_ < 8
            || std::string(header_.pData_, 6) != std::string("Nikon\0", 6)) {
            rc = 2;
        }
        return rc;
    }

    Nikon2MakerNote* Nikon2MakerNote::clone(bool alloc) const 
    {
        Nikon2MakerNote* pMakerNote = new Nikon2MakerNote(alloc); 
        assert(pMakerNote);
        pMakerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return pMakerNote;
    }

    std::ostream& Nikon2MakerNote::printTag(std::ostream& os, 
                                            uint16 tag, 
                                            const Value& value) const
    {
        switch (tag) {
        case 0x0003: print0x0003(os, value); break;
        case 0x0004: print0x0004(os, value); break;
        case 0x0005: print0x0005(os, value); break;
        case 0x0006: print0x0006(os, value); break;
        case 0x0007: print0x0007(os, value); break;
        case 0x000a: print0x000a(os, value); break;
        default:
            // All other tags (known or unknown) go here
            os << value;
            break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x0003(std::ostream& os,
                                               const Value& value)
    {
        long quality = value.toLong();
        switch (quality) {
        case 1: os << "VGA Basic"; break;
        case 2: os << "VGA Normal"; break;
        case 3: os << "VGA Fine"; break;
        case 4: os << "SXGA Basic"; break;
        case 5: os << "SXGA Normal"; break;
        case 6: os << "SXGA Fine"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x0004(std::ostream& os,
                                               const Value& value)
    {
        long color = value.toLong();
        switch (color) {
        case 1: os << "Color"; break;
        case 2: os << "Monochrome"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x0005(std::ostream& os,
                                               const Value& value)
    {
        long adjustment = value.toLong();
        switch (adjustment) {
        case 0: os << "Normal"; break;
        case 1: os << "Bright+"; break;
        case 2: os << "Bright-"; break;
        case 3: os << "Contrast+"; break;
        case 4: os << "Contrast-"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x0006(std::ostream& os,
                                               const Value& value)
    {
        long iso = value.toLong();
        switch (iso) {
        case 0: os << "80"; break;
        case 2: os << "160"; break;
        case 4: os << "320"; break;
        case 5: os << "100"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x0007(std::ostream& os,
                                               const Value& value)
    {
        long wb = value.toLong();
        switch (wb) {
        case 0: os << "Auto"; break;
        case 1: os << "Preset"; break;
        case 2: os << "Daylight"; break;
        case 3: os << "Incandescent"; break;
        case 4: os << "Fluorescent"; break;
        case 5: os << "Cloudy"; break;
        case 6: os << "Speedlight"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon2MakerNote::print0x000a(std::ostream& os,
                                               const Value& value)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << "Not used";
        }
        else if (zoom.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)zoom.first / zoom.second
               << "x";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

// *****************************************************************************
// free functions

    MakerNote* createNikon2MakerNote(bool alloc)
    {
        return new Nikon2MakerNote(alloc);
    }

}                                       // namespace Exiv2
