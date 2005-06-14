// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
 *
 * Lens database to decode Exif.Nikon3.LensData 
 * Copyright (C) 2005 Robert Rottmerhusen <email@rottmerhusen.com>
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
#include <cstring>

#define EXV_HAVE_LENSDATA

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
        TagInfo(0x0098, "LensData", "Lens data", nikon3IfdId, makerTags, undefined, print0x0098),
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

    std::ostream& Nikon3MakerNote::print0x0098(std::ostream& os,
                                               const Value& value)
    {
#ifdef EXV_HAVE_LENSDATA
        //# List of AF F-Mount lenses - version 1.12
        //#-----------------------------------------
        //# created by Robert Rottmerhusen 2005
        //# for use in non-commercial, GPL or open source software only!
        //# please contact me for adding lenses or use in commercial software.
        //#
        //#"data from TAG 0x98" "ltyp""manuf" "lens name from manuf";
        //#
        struct {unsigned char lid,stps,focs,focl,aps,apl,lfw, ltype; char *manuf, *lensname;}
        fmountlens[] = {
            {0x01,0x58,0x50,0x50,0x14,0x14,0x02,0x00, "Nikon", "AF Nikkor 50mm f/1.8"},
            {0x02,0x42,0x44,0x5C,0x2A,0x34,0x08,0x00, "Nikon", "AF Zoom-Nikkor 35-70mm f/3.3-4.5"},
            {0x04,0x48,0x3C,0x3C,0x24,0x24,0x03,0x00, "Nikon", "AF Nikkor 28mm f/2.8"},
            {0x05,0x54,0x50,0x50,0x0C,0x0C,0x04,0x00, "Nikon", "AF Nikkor 50mm f/1.4"},
            {0x07,0x40,0x3C,0x62,0x2C,0x34,0x03,0x00, "Nikon", "AF Zoom-Nikkor 28-85mm f/3.5-4.5"},
            {0x08,0x40,0x44,0x6A,0x2C,0x34,0x04,0x00, "Nikon", "AF Zoom-Nikkor 35-105mm f/3.5-4.5"},
            {0x09,0x48,0x37,0x37,0x24,0x24,0x04,0x00, "Nikon", "AF Nikkor 24mm f/2.8"},
            {0x0A,0x48,0x8E,0x8E,0x24,0x24,0x03,0x00, "Nikon", "AF Nikkor 300mm f/2.8 IF-ED"},
            {0x0B,0x48,0x7C,0x7C,0x24,0x24,0x05,0x00, "Nikon", "AF Nikkor 180mm f/2.8 IF-ED"},
            {0x0E,0x48,0x5C,0x81,0x30,0x30,0x05,0x00, "Nikon", "AF Zoom-Nikkor 70-210mm f/4"},
            {0x0F,0x58,0x50,0x50,0x14,0x14,0x05,0x00, "Nikon", "AF Nikkor 50mm f/1.8 N"},
            {0x10,0x48,0x8E,0x8E,0x30,0x30,0x08,0x00, "Nikon", "AF Nikkor 300mm f/4 IF-ED"},
            {0x12,0x48,0x5C,0x81,0x30,0x3C,0x09,0x00, "Nikon", "AF Nikkor 70-210mm f/4-5.6"},
            {0x13,0x42,0x37,0x50,0x2A,0x34,0x0B,0x00, "Nikon", "AF Zoom-Nikkor 24-50mm f/3.3-4.5"},
            {0x14,0x48,0x60,0x80,0x24,0x24,0x0B,0x00, "Nikon", "AF Zoom-Nikkor 80-200mm f/2.8 ED"},
            {0x15,0x4C,0x62,0x62,0x14,0x14,0x0C,0x00, "Nikon", "AF Nikkor 85mm f/1.8"},
            {0x1B,0x44,0x5E,0x8E,0x34,0x3C,0x10,0x00, "Nikon", "AF Zoom-Nikkor 75-300mm f/4.5-5.6"},
            {0x1C,0x48,0x30,0x30,0x24,0x24,0x12,0x00, "Nikon", "AF Nikkor 20mm f/2.8"},
            {0x1D,0x42,0x44,0x5C,0x2A,0x34,0x12,0x00, "Nikon", "AF Zoom-Nikkor 35-70mm f/3.3-4.5 N"},
            {0x1E,0x54,0x56,0x56,0x24,0x24,0x13,0x00, "Nikon", "AF Micro-Nikkor 60mm f/2.8"},
            {0x25,0x48,0x44,0x5c,0x24,0x24,0x1B,0x02, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8D"},
            {0x25,0x48,0x44,0x5c,0x24,0x24,0x52,0x02, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8D N"},
            {0x27,0x48,0x8E,0x8E,0x24,0x24,0xF2,0x02, "Nikon", "AF-I Nikkor 300mm f/2.8D IF-ED"},
            {0x2A,0x54,0x3C,0x3C,0x0C,0x0C,0x26,0x02, "Nikon", "AF Nikkor 28mm f/1.4D"},
            {0x2C,0x48,0x6A,0x6A,0x18,0x18,0x27,0x02, "Nikon", "AF DC-Nikkor 105mm f/2D"},
            {0x2D,0x48,0x80,0x80,0x30,0x30,0x21,0x02, "Nikon", "AF Micro-Nikkor 200mm f/4D IF-ED"},
            {0x31,0x54,0x56,0x56,0x24,0x24,0x25,0x02, "Nikon", "AF Micro-Nikkor 60mm f/2.8D"},
            {0x32,0x54,0x6A,0x6A,0x24,0x24,0x35,0x02, "Nikon", "AF Micro-Nikkor 105mm f/2.8D"},
            {0x33,0x48,0x2D,0x2D,0x24,0x24,0x31,0x02, "Nikon", "AF Nikkor 18mm f/2.8D"},
            {0x36,0x48,0x37,0x37,0x24,0x24,0x34,0x02, "Nikon", "AF Nikkor 24mm f/2.8D"},
            {0x37,0x48,0x30,0x30,0x24,0x24,0x36,0x02, "Nikon", "AF Nikkor 20mm f/2.8D"},
            {0x38,0x4C,0x62,0x62,0x14,0x14,0x37,0x02, "Nikon", "AF Nikkor 85mm f/1.8D"},
            {0x3B,0x48,0x44,0x5C,0x24,0x24,0x3A,0x02, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8D N"},
            {0x3E,0x48,0x3C,0x3C,0x24,0x24,0x3D,0x02, "Nikon", "AF Nikkor 28mm f/2.8D"},
            {0x41,0x48,0x7c,0x7c,0x24,0x24,0x43,0x02, "Nikon", "AF Nikkor 180mm f/2.8D IF-ED"},
            {0x42,0x54,0x44,0x44,0x18,0x18,0x44,0x02, "Nikon", "AF Nikkor 35mm f/2D"},
            {0x43,0x54,0x50,0x50,0x0C,0x0C,0x46,0x02, "Nikon", "AF Nikkor 50mm f/1.4D"},
            {0x46,0x3C,0x44,0x60,0x30,0x3C,0x49,0x02, "Nikon", "AF Zoom-Nikkor 35-80mm f/4-5.6D"},
            {0x48,0x48,0x8E,0x8E,0x24,0x24,0x4B,0x02, "Nikon", "AF-S Nikkor 300mm f/2.8D IF-ED"},
            {0x4A,0x54,0x62,0x62,0x0C,0x0C,0x4D,0x02, "Nikon", "AF Nikkor 85mm f/1.4D IF"},
            {0x4C,0x40,0x37,0x6E,0x2C,0x3C,0x4F,0x02, "Nikon", "AF Zoom-Nikkor 24-120mm f/3.5-5.6D IF"},
            {0x4D,0x40,0x3C,0x80,0x2C,0x3C,0x62,0x02, "Nikon", "AF Zoom-Nikkor 28-200mm f/3.5-5.6D IF"},
            {0x4E,0x48,0x72,0x72,0x18,0x18,0x51,0x02, "Nikon", "AF DC-Nikkor 135mm f/2D"},
            {0x53,0x48,0x60,0x80,0x24,0x24,0x60,0x02, "Nikon", "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
            {0x54,0x44,0x5C,0x7C,0x34,0x3C,0x58,0x02, "Nikon", "AF Zoom-Micro Nikkor 70-180mm f/4.5-5.6D ED"},
            {0x56,0x48,0x5C,0x8E,0x30,0x3C,0x5A,0x02, "Nikon", "AF Zoom-Nikkor 70-300mm f/4-5.6D ED"},
            {0x59,0x48,0x98,0x98,0x24,0x24,0x5D,0x02, "Nikon", "AF-S Nikkor 400mm f/2.8D IF-ED"},
            {0x5A,0x3C,0x3E,0x56,0x30,0x3C,0x5E,0x06, "Nikon", "IX-Nikkor 30-60mm f/4-5.6"},
            {0x5D,0x48,0x3C,0x5C,0x24,0x24,0x63,0x02, "Nikon", "AF-S Zoom-Nikkor 28-70mm f/2.8D IF-ED"},
            {0x5E,0x48,0x60,0x80,0x24,0x24,0x64,0x02, "Nikon", "AF-S Zoom-Nikkor 80-200mm f/2.8D IF-ED"},
            {0x5F,0x40,0x3C,0x6A,0x2C,0x34,0x65,0x02, "Nikon", "AF Zoom-Nikkor 28-105mm f/3.5-4.5D IF"},
            {0x63,0x48,0x2B,0x44,0x24,0x24,0x68,0x02, "Nikon", "AF-S Nikkor 17-35mm f/2.8D IF-ED"},
            {0x64,0x00,0x62,0x62,0x24,0x24,0x6A,0x02, "Nikon", "PC Micro-Nikkor 85mm f/2.8D"},
            {0x65,0x44,0x60,0x98,0x34,0x3C,0x6B,0x0A, "Nikon", "AF VR Zoom-Nikkor 80-400mm f/4.5-5.6D ED"},
            {0x66,0x40,0x2D,0x44,0x2C,0x34,0x6C,0x02, "Nikon", "AF Zoom-Nikkor 18-35mm f/3.5-4.5D IF-ED"},
            {0x67,0x48,0x37,0x62,0x24,0x30,0x6D,0x02, "Nikon", "AF Zoom-Nikkor 24-85mm f/2.8-4D IF"},
            {0x68,0x42,0x3C,0x60,0x2A,0x3C,0x6E,0x06, "Nikon", "AF Zoom-Nikkor 28-80mm f/3.3-5.6G"},
            {0x69,0x48,0x5C,0x8E,0x30,0x3C,0x6F,0x06, "Nikon", "AF Zoom-Nikkor 70-300mm f/4-5.6G"},
            {0x6A,0x48,0x8E,0x8E,0x30,0x30,0x70,0x02, "Nikon", "AF-S Nikkor 300mm f/4D IF-ED"},
            {0x6D,0x48,0x8E,0x8E,0x24,0x24,0x73,0x02, "Nikon", "AF-S Nikkor 300mm f/2.8D IF-ED II"},
            {0x6E,0x48,0x98,0x98,0x24,0x24,0x74,0x02, "Nikon", "AF-S Nikkor 400mm f/2.8D IF-ED II"},
            {0x70,0x3C,0xA6,0xA6,0x30,0x30,0x76,0x02, "Nikon", "AF-S Nikkor 600mm f/4D IF-ED"},
            {0x72,0x48,0x4C,0x4C,0x24,0x24,0x77,0x00, "Nikon", "Nikkor 45mm f/2.8 P"},
            {0x74,0x40,0x37,0x62,0x2C,0x34,0x78,0x06, "Nikon", "AF-S Zoom-Nikkor 24-85mm f/3.5-4.5G IF-ED"},
            {0x75,0x40,0x3C,0x68,0x2C,0x3C,0x79,0x06, "Nikon", "AF Zoom-Nikkor 28-100mm f/3.5-5.6G"},
            {0x76,0x58,0x50,0x50,0x14,0x14,0x7A,0x02, "Nikon", "AF Nikkor 50mm f/1.8D"},
            {0x77,0x48,0x5C,0x80,0x24,0x24,0x7B,0x0E, "Nikon", "AF-S VR Zoom-Nikkor 70-200mm f/2.8G IF-ED"},
            {0x78,0x40,0x37,0x6E,0x2C,0x3C,0x7C,0x0E, "Nikon", "AF-S VR Zoom-Nikkor 24-120mm f/3.5-5.6G IF-ED"},
            {0x79,0x40,0x3C,0x80,0x2C,0x3C,0x7F,0x06, "Nikon", "AF Zoom-Nikkor 28-200mm f/3.5-5.6G IF-ED"},
            {0x7A,0x3C,0x1F,0x37,0x30,0x30,0x7E,0x06, "Nikon", "AF-S DX Zoom-Nikkor 12-24mm f/4G IF-ED"},
            {0x7B,0x48,0x80,0x98,0x30,0x30,0x80,0x0E, "Nikon", "AF-S VR Zoom-Nikkor 200-400mm f/4G IF-ED"},
            {0x7D,0x48,0x2B,0x53,0x24,0x24,0x82,0x06, "Nikon", "AF-S DX Zoom-Nikkor 17-55mm f/2.8G IF-ED"},
            {0x7F,0x40,0x2D,0x5C,0x2C,0x34,0x84,0x06, "Nikon", "AF-S DX Zoom-Nikkor 18-70mm f/3.5-4.5G IF-ED"},
            {0x80,0x48,0x1A,0x1A,0x24,0x24,0x85,0x06, "Nikon", "AF DX Fisheye-Nikkor 10.5mm f/2.8G ED"},
            {0x81,0x54,0x80,0x80,0x18,0x18,0x86,0x0E, "Nikon", "AF-S VR Nikkor 200mm f/2G IF-ED"},
            {0x82,0x48,0x8E,0x8E,0x24,0x24,0x87,0x0E, "Nikon", "AF-S VR Nikkor 300mm f/2.8G IF-ED"},
            {0x89,0x3C,0x53,0x80,0x30,0x3C,0x8B,0x06, "Nikon", "AF-S DX Zoom-Nikkor 55-200mm f/4-5.6G ED"},
            {0x8C,0x40,0x2D,0x53,0x2C,0x3C,0x8E,0x06, "Nikon", "AF-S DX Zoom-Nikkor 18-55mm f/3.5-5.6G ED"},
            {0x06,0x3F,0x68,0x68,0x2C,0x2C,0x06,0x00, "Cosina", "100mm F/3.5 Macro"},
            {0x02,0x3F,0x24,0x24,0x2C,0x2C,0x02,0x00, "Sigma", "14mm F3.5"},
            {0x02,0x46,0x37,0x37,0x25,0x25,0x02,0x00, "Sigma", "24mm F2.8 Macro"},
            {0x02,0x3F,0x3C,0x5C,0x2D,0x35,0x02,0x00, "Sigma", "28-70mm F3.5-4.5 UC"},
            {0x02,0x40,0x44,0x73,0x2B,0x36,0x02,0x00, "Sigma", "35-135mm F3.5-4.5 a"},
            {0x02,0x37,0x5E,0x8E,0x35,0x3D,0x02,0x00, "Sigma", "75-300mm F4.5-5.6 APO"},
            {0x02,0x48,0x65,0x65,0x24,0x24,0x02,0x00, "Sigma", "90mm F2.8 Macro"},
            {0x02,0x2F,0x98,0x98,0x3D,0x3D,0x02,0x00, "Sigma", "400mm F5.6 APO"},
            {0x26,0x40,0x3C,0x8E,0x2C,0x40,0x1C,0x02, "Sigma", "28-300mm F3.5-6.3 Macro D"},
            {0x26,0x40,0x3C,0x80,0x2B,0x3C,0x1C,0x02, "Sigma", "28-200mm F3.5-5.6 Compact Aspherical Hyperzoom Macro D"},
            {0x26,0x40,0x3C,0x60,0x2C,0x3C,0x1C,0x02, "Sigma", "28-80mm F3.5-5.6 Mini Zoom Macro II Aspherical D"},
            {0x26,0x54,0x37,0x5C,0x24,0x24,0x1C,0x02, "Sigma", "24-70mm F2.8 EX DG Macro D"},
            {0x26,0x40,0x2D,0x70,0x2B,0x3C,0x1C,0x06, "Sigma", "18-125mm F3.5-5.6 DC G"},
            {0x26,0x48,0x2D,0x50,0x24,0x24,0x1C,0x06, "Sigma", "18-50mm F2.8 EX DC G"},
            {0x26,0x40,0x2D,0x50,0x2C,0x3C,0x1C,0x06, "Sigma", "18-50mm F3.5-5.6 DC G"},
            {0x48,0x38,0x1F,0x37,0x34,0x3C,0x4B,0x06, "Sigma", "12-24mm F4.5-5.6 EX Aspherical DG HSM G"},
            {0x48,0x48,0x2B,0x44,0x24,0x30,0x4B,0x06, "Sigma", "17-35mm F2.8-4 EX DG  Aspherical HSM G"},
            {0x48,0x3C,0x50,0xA0,0x30,0x40,0x4B,0x02, "Sigma", "50-500mmF4-6.3 EX APO RF HSM D"},
            {0x48,0x54,0x5C,0x80,0x24,0x24,0x4B,0x02, "Sigma", "70-200mm F2.8 EX APO IF HSM D"},
            {0x48,0x48,0x68,0x8E,0x30,0x30,0x4B,0x02, "Sigma", "100-300mm F4 EX IF HSM D"},
            {0x48,0x48,0x76,0x76,0x24,0x24,0x4B,0x06, "Sigma", "150mm F2.8 EX DG APO Macro HSM G"},
            {0x77,0x44,0x61,0x98,0x34,0x3C,0x7B,0x0E, "Sigma", "80-400mm f4.5-5.6 EX OS G"},
            {0x03,0x43,0x5C,0x81,0x35,0x35,0x02,0x00, "Soligor", "AF C/D ZOOM UMCS 70-210mm 1:4.5"},
            {0x00,0x3C,0x1F,0x37,0x30,0x30,0x00,0x06, "Tokina", "AT-X 124 AF PRO DX - AF 12-24mm f/4"},
            {0x00,0x40,0x2B,0x2B,0x2C,0x2C,0x00,0x02, "Tokina", "AT-X 17 AF PRO - AF 17mm f/3.5"},
            {0x00,0x54,0x68,0x68,0x24,0x24,0x00,0x02, "Tokina", "AT-X M100 PRO D - 100mm F2.8"},
            {0x4D,0x41,0x3C,0x8E,0x2B,0x40,0x62,0x02, "Tamron", "AF28-300mm F/3.5-6.3 XR Di LD Aspherical (IF)"},
            {0x00,0x3F,0x2D,0x80,0x2B,0x40,0x00,0x06, "Tamron", "AF18-200mm F/3.5-6.3 XR Di II LD Aspherical (IF)"},
            {0x32,0x53,0x64,0x64,0x24,0x24,0x35,0x02, "Tamron", "SP AF90mm F/2.8 Di 1:1 Macro"},
            {0x00,0x48,0x3C,0x6A,0x24,0x24,0x00,0x02, "Unknown", "28-105mm F/2.8D"},
            {0x00,0x49,0x30,0x48,0x22,0x2B,0x00,0x02, "Unknown", "20-40mm F/2.7-3.3D"},
            {0x07,0x46,0x2B,0x44,0x24,0x30,0x03,0x02, "Unknown", "AF17-35mm D"},
            {0x1E,0x5D,0x64,0x64,0x20,0x20,0x13,0x00, "Unknown", "90mm F/2.5"},
            {0x20,0x3C,0x80,0x98,0x3D,0x3D,0x1E,0x02, "Unknown", "200-400mm F/5.6D"},
            {0x2F,0x40,0x30,0x44,0x2C,0x34,0x29,0x02, "Unknown", "20-35mm F/3.5-4.5D"},
            {0,0,0,0,0,0,0,0, NULL, NULL}
        };

        if (value.typeId() != undefined) return os << value;

        DataBuf lens(value.size());
        // ByteOrder is only to satisfy the interface
        value.copy(lens.pData_, invalidByteOrder);

        int idx = 0;
        if (0 == memcmp(lens.pData_, "0100", 4)) {
            idx = 6;
        }
        else if (0 == memcmp(lens.pData_, "0101", 4)) {
            idx = 11;            
        }
        else if (0 == memcmp(lens.pData_, "0201", 4)) {
            // Here we should decrypt(lens.pData_ + 4, lens.size_ - 4);
            // however, the decrypt algorithm requires access to serial number
            // and shutter count tags but print functions are static...
            idx = 11;
        }
        if (idx == 0 || lens.size_ < idx + 7) {
            // Unknown version or not enough data
            return os << value;
        }
        for (int i = 0; fmountlens[i].lensname != NULL; ++i) {
            if (   lens.pData_[idx]   == fmountlens[i].lid
                && lens.pData_[idx+1] == fmountlens[i].stps
                && lens.pData_[idx+2] == fmountlens[i].focs
                && lens.pData_[idx+3] == fmountlens[i].focl
                && lens.pData_[idx+4] == fmountlens[i].aps
                && lens.pData_[idx+5] == fmountlens[i].apl
                && lens.pData_[idx+6] == fmountlens[i].lfw) {
                // Lens found in database
                return os << fmountlens[i].manuf << " " << fmountlens[i].lensname;
            }
        }
        // Lens not found in database
        return os << value;
#else 
        return os << value;
#endif // EXV_HAVE_LENSDATA
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
