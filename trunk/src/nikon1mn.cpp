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
  File:      nikon1mn.cpp
  Version:   $Name:  $ $Revision: 1.1 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   17-May-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.1 $ $RCSfile: nikon1mn.cpp,v $")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "nikon1mn.hpp"
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

    const Nikon1MakerNote::RegisterMakerNote Nikon1MakerNote::register_;

    // Nikon1 MakerNote Tag Info
    static const MakerNote::MnTagInfo nikon1MnTagInfo[] = {
        MakerNote::MnTagInfo(0x0001, "Version", "Nikon Makernote version"),
        MakerNote::MnTagInfo(0x0002, "ISOSpeed", "ISO speed setting"),
        MakerNote::MnTagInfo(0x0003, "ColorMode", "Color mode"),
        MakerNote::MnTagInfo(0x0004, "Quality", "Image quality setting"),
        MakerNote::MnTagInfo(0x0005, "WhiteBalance", "White balance"),
        MakerNote::MnTagInfo(0x0006, "Sharpening", "Image sharpening setting"),
        MakerNote::MnTagInfo(0x0007, "Focus", "Focus mode"),
        MakerNote::MnTagInfo(0x0008, "Flash", "Flash mode"),
        MakerNote::MnTagInfo(0x000f, "ISOSelection", "ISO selection"),
        MakerNote::MnTagInfo(0x0080, "ImageAdjustment", "Image adjustment setting"),
        MakerNote::MnTagInfo(0x0082, "Adapter", "Adapter used"),
        MakerNote::MnTagInfo(0x0085, "FocusDistance", "Manual focus distance"),
        MakerNote::MnTagInfo(0x0086, "DigitalZoom", "Digital zoom setting"),
        MakerNote::MnTagInfo(0x0088, "AFFocusPos", "AF focus position"),
        // End of list marker
        MakerNote::MnTagInfo(0xffff, "(UnknownNikon1MnTag)", "Unknown Nikon1MakerNote tag")
    };

    Nikon1MakerNote::Nikon1MakerNote(bool alloc)
        : IfdMakerNote(nikon1MnTagInfo, alloc), sectionName_("Nikon1")
    {
    }

    Nikon1MakerNote* Nikon1MakerNote::clone(bool alloc) const 
    {
        return new Nikon1MakerNote(alloc);
    }

    std::ostream& Nikon1MakerNote::printTag(std::ostream& os, 
                                            uint16 tag, 
                                            const Value& value) const
    {
        switch (tag) {
        case 0x0002: print0x0002(os, value); break;
        case 0x0007: print0x0007(os, value); break;
        case 0x0085: print0x0085(os, value); break;
        case 0x0086: print0x0086(os, value); break;
        case 0x0088: print0x0088(os, value); break;
        default:
            // All other tags (known or unknown) go here
            os << value;
            break;
        }
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0002(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() > 1) {
            os << value.toLong(1);
        }
        else {
            os << "(" << value << ")"; 
        }
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0007(std::ostream& os,
                                               const Value& value)
    {
        std::string focus = value.toString(); 
        if      (focus == "AF-C  ") os << "Continuous autofocus";
        else if (focus == "AF-S  ") os << "Single autofocus";
        else                      os << "(" << value << ")";
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0085(std::ostream& os,
                                               const Value& value)
    {
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << "Unknown";
        }
        else if (distance.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(2)
               << (float)distance.first / distance.second
               << " m";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0086(std::ostream& os,
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

    std::ostream& Nikon1MakerNote::print0x0088(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() > 1) {
            switch (value.toLong(1)) {
            case 0: os << "Center"; break;
            case 1: os << "Top"; break;
            case 2: os << "Bottom"; break;
            case 3: os << "Left"; break;
            case 4: os << "Right"; break;
            default: os << "(" << value << ")"; break;
            }
        }
        else {
            os << "(" << value << ")"; 
        }
        return os;
    }

// *****************************************************************************
// free functions

    MakerNote* createNikon1MakerNote(bool alloc)
    {
        return new Nikon1MakerNote(alloc);
    }

}                                       // namespace Exiv2
