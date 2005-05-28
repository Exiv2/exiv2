// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
  File:      nikonmn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   17-May-04, ahu: created
             25-May-04, ahu: combined all Nikon formats in one component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "types.hpp"
#include "nikonmn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "image.hpp"
#include "tags.hpp"
#include "error.hpp"

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

    //! @cond IGNORE
    Nikon1MakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("NIKON*", "*", createNikonMakerNote);
        MakerNoteFactory::registerMakerNote(
            nikon1IfdId, MakerNote::AutoPtr(new Nikon1MakerNote));

        ExifTags::registerMakerTagInfo(nikon1IfdId, tagInfo_);
    }
    //! @endcond

    // Nikon1 MakerNote Tag Info
    const TagInfo Nikon1MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", "Nikon Makernote version", nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x0002, "ISOSpeed", "ISO speed setting", nikon1IfdId, makerTags, unsignedShort, print0x0002),
        TagInfo(0x0003, "ColorMode", "Color mode", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0004, "Quality", "Image quality setting", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "WhiteBalance", "White balance", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "Sharpening", "Image sharpening setting", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "Focus", "Focus mode", nikon1IfdId, makerTags, asciiString, print0x0007),
        TagInfo(0x0008, "Flash", "Flash mode", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "0x000a", "Unknown", nikon1IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x000f, "ISOSelection", "ISO selection", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0010, "DataDump", "Data dump", nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x0080, "ImageAdjustment", "Image adjustment setting", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0082, "Adapter", "Adapter used", nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0085, "FocusDistance", "Manual focus distance", nikon1IfdId, makerTags, unsignedRational, print0x0085),
        TagInfo(0x0086, "DigitalZoom", "Digital zoom setting", nikon1IfdId, makerTags, unsignedRational, print0x0086),
        TagInfo(0x0088, "AFFocusPos", "AF focus position", nikon1IfdId, makerTags, undefined, print0x0088),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon1MnTag)", "Unknown Nikon1MakerNote tag", nikon1IfdId, makerTags, invalidTypeId, printValue)
    };

    Nikon1MakerNote::Nikon1MakerNote(bool alloc)
        : IfdMakerNote(nikon1IfdId, alloc)
    {
    }

    Nikon1MakerNote::Nikon1MakerNote(const Nikon1MakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    Nikon1MakerNote::AutoPtr Nikon1MakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    Nikon1MakerNote* Nikon1MakerNote::create_(bool alloc) const 
    {
        return new Nikon1MakerNote(alloc);
    }

    Nikon1MakerNote::AutoPtr Nikon1MakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    Nikon1MakerNote* Nikon1MakerNote::clone_() const 
    {
        return new Nikon1MakerNote(*this);
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

    //! @cond IGNORE
    Nikon2MakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote(
            nikon2IfdId, MakerNote::AutoPtr(new Nikon2MakerNote));

        ExifTags::registerMakerTagInfo(nikon2IfdId, tagInfo_);
    }
    //! @endcond

    // Nikon2 MakerNote Tag Info
    const TagInfo Nikon2MakerNote::tagInfo_[] = {
        TagInfo(0x0002, "0x0002", "Unknown", nikon2IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0003, "Quality", "Image quality setting", nikon2IfdId, makerTags, unsignedShort, print0x0003),
        TagInfo(0x0004, "ColorMode", "Color mode", nikon2IfdId, makerTags, unsignedShort, print0x0004),
        TagInfo(0x0005, "ImageAdjustment", "Image adjustment setting", nikon2IfdId, makerTags, unsignedShort, print0x0005),
        TagInfo(0x0006, "ISOSpeed", "ISO speed setting", nikon2IfdId, makerTags, unsignedShort, print0x0006),
        TagInfo(0x0007, "WhiteBalance", "White balance", nikon2IfdId, makerTags, unsignedShort, print0x0007),
        TagInfo(0x0008, "Focus", "Focus mode", nikon2IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0009, "0x0009", "Unknown", nikon2IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "DigitalZoom", "Digital zoom setting", nikon2IfdId, makerTags, unsignedRational, print0x000a),
        TagInfo(0x000b, "Adapter", "Adapter used", nikon2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0f00, "0x0f00", "Unknown", nikon2IfdId, makerTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon2MnTag)", "Unknown Nikon2MakerNote tag", nikon2IfdId, makerTags, invalidTypeId, printValue)
    };

    Nikon2MakerNote::Nikon2MakerNote(bool alloc)
        : IfdMakerNote(nikon2IfdId, alloc)
    {
        byte buf[] = {
            'N', 'i', 'k', 'o', 'n', '\0', 0x00, 0x01
        };
        readHeader(buf, 8, byteOrder_);
    }

    Nikon2MakerNote::Nikon2MakerNote(const Nikon2MakerNote& rhs)
        : IfdMakerNote(rhs)
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

    Nikon2MakerNote::AutoPtr Nikon2MakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    Nikon2MakerNote* Nikon2MakerNote::create_(bool alloc) const 
    {
        AutoPtr makerNote(new Nikon2MakerNote(alloc)); 
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    Nikon2MakerNote::AutoPtr Nikon2MakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    Nikon2MakerNote* Nikon2MakerNote::clone_() const 
    {
        return new Nikon2MakerNote(*this);
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

    //! @cond IGNORE
    Nikon3MakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote(
            nikon3IfdId, MakerNote::AutoPtr(new Nikon3MakerNote));

        ExifTags::registerMakerTagInfo(nikon3IfdId, tagInfo_);
    }
    //! @endcond

    // Nikon3 MakerNote Tag Info
    const TagInfo Nikon3MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", "Nikon Makernote version", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0002, "ISOSpeed", "ISO speed used", nikon3IfdId, makerTags, unsignedShort, print0x0002),
        TagInfo(0x0003, "ColorMode", "Color mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0004, "Quality", "Image quality setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "WhiteBalance", "White balance", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "Sharpening", "Image sharpening setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "Focus", "Focus mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0008, "FlashSetting", "Flash setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0009, "FlashMode", "Flash mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000b, "WhiteBalanceBias", "White balance bias", nikon3IfdId, makerTags, signedShort, printValue),
//        TagInfo(0x000c, "ColorBalance1", "Color balance 1", nikon3IfdId, makerTags, xxx, printValue),
        TagInfo(0x000d, "0x000d", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x000e, "ExposureDiff", "Exposure difference", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x000f, "ISOSelection", "ISO selection", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0010, "DataDump", "Data dump", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0011, "ThumbOffset", "Thumbnail IFD offset", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0012, "FlashComp", "Flash compensation setting", nikon3IfdId, makerTags, undefined, print0x0012),
        TagInfo(0x0013, "ISOSetting", "ISO speed setting", nikon3IfdId, makerTags, unsignedShort, print0x0002), // use 0x0002 print fct
        TagInfo(0x0016, "ImageBoundry", "Image boundry", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0017, "0x0017", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0018, "FlashBracketComp", "Flash bracket compensation applied", nikon3IfdId, makerTags, undefined, print0x0012), // use 0x0012 print fct
        TagInfo(0x0019, "ExposureBracketComp", "AE bracket compensation applied", nikon3IfdId, makerTags, signedRational, printValue),
        TagInfo(0x0080, "ImageAdjustment", "Image adjustment setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0081, "ToneComp", "Tone compensation setting (contrast)", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0082, "AuxiliaryLens", "Auxiliary lens (adapter)", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0083, "LensType", "Lens type", nikon3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(0x0084, "Lens", "Lens", nikon3IfdId, makerTags, unsignedRational, print0x0084),
        TagInfo(0x0085, "FocusDistance", "Manual focus distance", nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0086, "DigitalZoom", "Digital zoom setting", nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0087, "FlashType", "Type of flash used", nikon3IfdId, makerTags, unsignedByte, print0x0087),
        TagInfo(0x0088, "AFFocusPos", "AF focus position", nikon3IfdId, makerTags, undefined, print0x0088),
        TagInfo(0x0089, "Bracketing", "Bracketing", nikon3IfdId, makerTags, unsignedShort, print0x0089),
        TagInfo(0x008a, "0x008a", "Unknown", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x008b, "LensFStops", "Number of lens stops", nikon3IfdId, makerTags, undefined, print0x008b),
//        TagInfo(0x008c, "NEFCurve1", "NEF curve 1", nikon3IfdId, makerTags, xxx, printValue),
        TagInfo(0x008d, "ColorMode", "Color mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x008f, "SceneMode", "Scene mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0090, "LightingType", "Lighting type", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0091, "0x0091", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0092, "HueAdjustment", "Hue adjustment", nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0094, "Saturation", "Saturation adjustment", nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0095, "NoiseReduction", "Noise reduction", nikon3IfdId, makerTags, asciiString, printValue),
//        TagInfo(0x0096, "NEFCurve2", "NEF curve 2", nikon3IfdId, makerTags, xxx, printValue),
        TagInfo(0x0097, "ColorBalance2", "Color balance 2", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0098, "0x0098", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0099, "NEFThumbnailSize", "NEF thumbnail size", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009a, "0x009a", "Unknown", nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x009b, "0x009b", "Unknown", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009f, "0x009f", "Unknown", nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x00a0, "SerialNumber", "Camera serial number", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00a2, "0x00a2", "Unknown", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a3, "0x00a3", "Unknown", nikon3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(0x00a5, "0x00a5", "Unknown", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a6, "0x00a6", "Unknown", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a7, "ShutterCount", "Number of shots taken by camera", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a8, "0x00a8", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x00a9, "ImageOptimization", "Image optimization", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00aa, "Saturation", "Saturation", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ab, "VariProgram", "Vari program", nikon3IfdId, makerTags, asciiString, printValue),
//        TagInfo(0x0e00, "PrintIM", "Print image matching", nikon3IfdId, makerTags, xxx, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon3MnTag)", "Unknown Nikon3MakerNote tag", nikon3IfdId, makerTags, invalidTypeId, printValue)
    };

    Nikon3MakerNote::Nikon3MakerNote(bool alloc)
        : IfdMakerNote(nikon3IfdId, alloc)
    {
        absOffset_ = false;
        byte buf[] = {
            'N', 'i', 'k', 'o', 'n', '\0', 
            0x02, 0x10, 0x00, 0x00, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08
        };
        readHeader(buf, 18, byteOrder_);
    }

    Nikon3MakerNote::Nikon3MakerNote(const Nikon3MakerNote& rhs)
        : IfdMakerNote(rhs)
    {
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

    Nikon3MakerNote::AutoPtr Nikon3MakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    Nikon3MakerNote* Nikon3MakerNote::create_(bool alloc) const 
    {
        AutoPtr makerNote(new Nikon3MakerNote(alloc)); 
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    Nikon3MakerNote::AutoPtr Nikon3MakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    Nikon3MakerNote* Nikon3MakerNote::clone_() const 
    {
        return new Nikon3MakerNote(*this);
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

    std::ostream& Nikon3MakerNote::print0x0012(std::ostream& os, 
                                               const Value& value)
    {
        // From the PHP JPEG Metadata Toolkit
        long fec = value.toLong();
        switch (fec) {
        case 0x06: os << "+1.0 EV"; break;
        case 0x04: os << "+0.7 EV"; break;
        case 0x03: os << "+0.5 EV"; break;
        case 0x02: os << "+0.3 EV"; break;
        case 0x00: os << "0.0 EV"; break;
        case 0xfe: os << "-0.3 EV"; break;
        case 0xfd: os << "-0.5 EV"; break;
        case 0xfc: os << "-0.7 EV"; break;
        case 0xfa: os << "-1.0 EV"; break;
        case 0xf8: os << "-1.3 EV"; break;
        case 0xf7: os << "-1.5 EV"; break;
        case 0xf6: os << "-1.7 EV"; break;
        case 0xf4: os << "-2.0 EV"; break;
        case 0xf2: os << "-2.3 EV"; break;
        case 0xf1: os << "-2.5 EV"; break;
        case 0xf0: os << "-2.7 EV"; break;
        case 0xee: os << "-3.0 EV"; break;
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
        // From Exiftool
        long flash = value.toLong();
        switch (flash) {
        case 0: os << "Not used"; break;
        case 8: os << "Fired, commander mode"; break;
        case 9: os << "Fired, TTL mode"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0088(std::ostream& os,
                                               const Value& value)
    {
        // Mappings taken from Exiftool 
        long afpos = value.toLong();
        switch (afpos) {
        case 0x0000: os << "Center"; break;
        case 0x0100: os << "Top"; break;
        case 0x0200: os << "Bottom"; break;
        case 0x0300: os << "Left"; break;
        case 0x0400: os << "Right"; break;
        
        // D70
        case 0x00001: os << "Single area, center"; break;
        case 0x10002: os << "Single area, top"; break;
        case 0x20004: os << "Single area, bottom"; break;
        case 0x30008: os << "Single area, left"; break;
        case 0x40010: os << "Single area, right"; break;

        case 0x1000001: os << "Dynamic area, center"; break;
        case 0x1010002: os << "Dynamic area, top"; break;
        case 0x1020004: os << "Dynamic area, bottom"; break;
        case 0x1030008: os << "Dynamic area, left"; break;
        case 0x1040010: os << "Dynamic area, right"; break;

        case 0x2000001: os << "Closest subject, center"; break;
        case 0x2010002: os << "Closest subject, top"; break;
        case 0x2020004: os << "Closest subject, bottom"; break;
        case 0x2030008: os << "Closest subject, left"; break;
        case 0x2040010: os << "Closest subject, right"; break;

        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0089(std::ostream& os,
                                               const Value& value)
    {
        // From Exiftool
        long b = value.toLong();
        switch (b) {
        case 0x00: os << "Single"; break;
        case 0x01: os << "Continuous"; break;
        case 0x02: os << "Delay"; break;
        case 0x03: os << "Remote with delay"; break;
        case 0x04: os << "Remote"; break;
        case 0x16: os << "Exposure bracketing"; break;
        case 0x64: os << "White balance bracketing"; break;
        default: os << "(" << value << ")"; break;
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x008b(std::ostream& os,
                                               const Value& value)
    {
        // Decoded by Robert Rottmerhusen <email@rottmerhusen.com>
        if (value.size() != 4) return os << "(" << value << ")";
        float a = value.toFloat(0);
        long  b = value.toLong(1);
        long  c = value.toLong(2);
        if (c == 0) return os << "(" << value << ")";
        return os << a * b / c;
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createNikonMakerNote(bool alloc,
                                            const byte* buf, 
                                            long len, 
                                            ByteOrder byteOrder, 
                                            long offset)
    {
        // If there is no "Nikon" string it must be Nikon1 format
        if (len < 6 || std::string(reinterpret_cast<const char*>(buf), 6) 
                    != std::string("Nikon\0", 6)) {
            return MakerNote::AutoPtr(new Nikon1MakerNote(alloc));
        }
        // If the "Nikon" string is not followed by a TIFF header, we assume
        // Nikon2 format
        TiffHeader tiffHeader;
        if (   len < 18 
            || tiffHeader.read(buf + 10) != 0 || tiffHeader.tag() != 0x002a) {
            return MakerNote::AutoPtr(new Nikon2MakerNote(alloc)); 
        }
        // Else we have a Nikon3 makernote
        return MakerNote::AutoPtr(new Nikon3MakerNote(alloc)); 
    }

}                                       // namespace Exiv2
