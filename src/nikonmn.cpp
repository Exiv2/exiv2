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
  Version:   $Name:  $ $Revision: 1.6 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   17-May-04, ahu: created
             25-May-04, ahu: combined all Nikon formats in one component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.6 $ $RCSfile: nikonmn.cpp,v $");

// *****************************************************************************
// included header files
#include "types.hpp"
#include "nikonmn.hpp"
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
        : IfdMakerNote(nikon1MnTagInfo, alloc), ifdItem_("Nikon1")
    {
    }

    Nikon1MakerNote* Nikon1MakerNote::clone(bool alloc) const 
    {
        return new Nikon1MakerNote(alloc);
    }

    std::ostream& Nikon1MakerNote::printTag(std::ostream& os, 
                                            uint16_t tag, 
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
        : IfdMakerNote(nikon2MnTagInfo, alloc), ifdItem_("Nikon2")
    {
    }

    int Nikon2MakerNote::readHeader(const byte* buf,
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
            || std::string(reinterpret_cast<char*>(header_.pData_), 6) 
                    != std::string("Nikon\0", 6)) {
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
                                            uint16_t tag, 
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

    // Nikon3 MakerNote Tag Info
    static const MakerNote::MnTagInfo nikon3MnTagInfo[] = {
        MakerNote::MnTagInfo(0x0001, "Version", "Nikon Makernote version"),
        MakerNote::MnTagInfo(0x0002, "ISOSpeed", "ISO speed setting"),
        MakerNote::MnTagInfo(0x0003, "ColorMode", "Color mode"),
        MakerNote::MnTagInfo(0x0004, "Quality", "Image quality setting"),
        MakerNote::MnTagInfo(0x0005, "WhiteBalance", "White balance"),
        MakerNote::MnTagInfo(0x0006, "Sharpening", "Image sharpening setting"),
        MakerNote::MnTagInfo(0x0007, "Focus", "Focus mode"),
        MakerNote::MnTagInfo(0x0008, "FlashSetting", "Flash setting"),
        MakerNote::MnTagInfo(0x0009, "FlashMode", "Flash mode"),
        MakerNote::MnTagInfo(0x000b, "WhiteBalanceBias", "White balance bias"),
        MakerNote::MnTagInfo(0x000e, "ExposureDiff", "Exposure difference"),
        MakerNote::MnTagInfo(0x000f, "ISOSelection", "ISO selection"),
        MakerNote::MnTagInfo(0x0011, "ThumbOffset", "Thumbnail IFD offset"),
        MakerNote::MnTagInfo(0x0012, "FlashBias", "Flash bias"),
        MakerNote::MnTagInfo(0x0080, "ImageAdjustment", "Image adjustment setting"),
        MakerNote::MnTagInfo(0x0081, "ToneComp", "Tone compensation setting"),
        MakerNote::MnTagInfo(0x0083, "LensType", "Lens type"),
        MakerNote::MnTagInfo(0x0084, "Lens", "Lens"),
        MakerNote::MnTagInfo(0x0085, "FocusDistance", "Manual focus distance"),
        MakerNote::MnTagInfo(0x0086, "DigitalZoom", "Digital zoom setting"),
        MakerNote::MnTagInfo(0x0087, "FlashType", "Type of flash used"),
        MakerNote::MnTagInfo(0x0088, "AFFocusPos", "AF focus position"),
        MakerNote::MnTagInfo(0x0089, "Bracketing", "Bracketing"),
        MakerNote::MnTagInfo(0x008d, "ColorMode", "Color mode"),
        MakerNote::MnTagInfo(0x0090, "LightingType", "Lighting type"),
        MakerNote::MnTagInfo(0x0092, "Hue", "Hue adjustment"),
        MakerNote::MnTagInfo(0x0094, "Saturation", "Saturation adjustment"),
        MakerNote::MnTagInfo(0x0095, "NoiseReduction", "Noise reduction"),
        // End of list marker
        MakerNote::MnTagInfo(0xffff, "(UnknownNikon3MnTag)", "Unknown Nikon3MakerNote tag")
    };

    Nikon3MakerNote::Nikon3MakerNote(bool alloc)
        : IfdMakerNote(nikon3MnTagInfo, alloc), ifdItem_("Nikon3")
    {
        absOffset_ = false;
    }

    int Nikon3MakerNote::readHeader(const byte* buf,
                                  long len, 
                                  ByteOrder byteOrder)
    {
        if (len < 18) return 1;

        header_.alloc(18);
        memcpy(header_.pData_, buf, header_.size_);
        TiffHeader tiffHeader;
        tiffHeader.read(header_.pData_ + 10);
        byteOrder_ = tiffHeader.byteOrder();
        adjOffset_ = tiffHeader.offset();
        return 0;
    }

    int Nikon3MakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the Nikon prefix
        if (   header_.size_ < 18
            || std::string(reinterpret_cast<char*>(header_.pData_), 6) 
                    != std::string("Nikon\0", 6)) {
            rc = 2;
        }
        return rc;
    }

    Nikon3MakerNote* Nikon3MakerNote::clone(bool alloc) const 
    {
        Nikon3MakerNote* pMakerNote = new Nikon3MakerNote(alloc); 
        assert(pMakerNote);
        pMakerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return pMakerNote;
    }

    std::ostream& Nikon3MakerNote::printTag(std::ostream& os, 
                                            uint16_t tag, 
                                            const Value& value) const
    {
        switch (tag) {
        case 0x0002: print0x0002(os, value); break;
        case 0x0083: print0x0083(os, value); break;
        case 0x0084: print0x0084(os, value); break;
        case 0x0087: print0x0087(os, value); break;
        case 0x0089: print0x0089(os, value); break;
        default:
            // All other tags (known or unknown) go here
            os << value;
            break;
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0002(std::ostream& os,
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

    std::ostream& Nikon3MakerNote::print0x0083(std::ostream& os,
                                               const Value& value)
    {
        long type = value.toLong();
        switch (type) {
        case  0: os << "AF"; break;
        case  1: os << "Manual"; break;
        case  2: os << "AF-D"; break;
        case  6: os << "AF-D G"; break;
        case 10: os << "AF-D VR"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0084(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() == 4) {
            long len1 = value.toLong(0);
            long len2 = value.toLong(1);
            Rational fno1 = value.toRational(2);
            Rational fno2 = value.toRational(3);
            os << len1;
            if (len2 != len1) {
                os << "-" << len2;
            }
            os << "mm " 
               << "F" << (float)fno1.first / fno1.second;
            if (fno2 != fno1) {
                os << "-" << (float)fno2.first / fno2.second;
            }
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0087(std::ostream& os,
                                               const Value& value)
    {
        long flash = value.toLong();
        switch (flash) {
        case 0: os << "None"; break;
        case 7: os << "External"; break;
        case 9: os << "On camera"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0089(std::ostream& os,
                                               const Value& value)
    {
        long b = value.toLong();
        switch (b) {
        case  0: os << "None"; break;
        case  1: os << "None"; break;
        case 17: os << "Exposure"; break;
        case 81: os << "White balance"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

// *****************************************************************************
// free functions

    MakerNote* createNikonMakerNote(bool alloc,
                                    const byte* buf, 
                                    long len, 
                                    ByteOrder byteOrder, 
                                    long offset)
    {
        // If there is no "Nikon" string it must be Nikon1 format
        if (len < 6 || std::string(reinterpret_cast<const char*>(buf), 6) 
                    != std::string("Nikon\0", 6)) {
            return new Nikon1MakerNote(alloc);
        }
        // If the "Nikon" string is not followed by a TIFF header, we assume
        // Nikon2 format
        TiffHeader tiffHeader;
        if (   len < 18 
            || tiffHeader.read(buf + 10) != 0 || tiffHeader.tag() != 0x002a) {
            return new Nikon2MakerNote(alloc);            
        }
        // Else we have a Nikon3 makernote
        return new Nikon3MakerNote(alloc); 
    }

}                                       // namespace Exiv2
