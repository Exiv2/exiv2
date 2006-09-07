// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      fujimn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Feb-04, ahu: created
             07-Mar-04, ahu: isolated as a separate component
  Credits:   Fujifilm MakerNote implemented according to the specification
             in "Appendix 4: Makernote of Fujifilm" of the document
             "Exif file format" by TsuruZoh Tachibanaya
             <http://park2.wakwak.com/%7Etsuruzoh/Computer/Digicams/exif-e.html>
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "fujimn.hpp"
#include "makernote.hpp"
#include "value.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! @cond IGNORE
    FujiMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("FUJIFILM", "*", createFujiMakerNote);
        MakerNoteFactory::registerMakerNote(
            fujiIfdId, MakerNote::AutoPtr(new FujiMakerNote));

        ExifTags::registerMakerTagInfo(fujiIfdId, tagInfo_);
    }
    //! @endcond

    //! OffOn, multiple tags
    extern const TagDetails fujiOffOn[] = {
        { 0, "Off" },
        { 1, "On"  }
    };

    //! Sharpness, tag 0x1001
    extern const TagDetails fujiSharpness[] = {
        { 1, "Soft"   },
        { 2, "Soft"   },
        { 3, "Normal" },
        { 4, "Hard"   },
        { 5, "Hard"   }
    };

    //! WhiteBalance, tag 0x1002
    extern const TagDetails fujiWhiteBalance[] = {
        {    0, "Auto"                     },
        {  256, "Daylight"                 },
        {  512, "Cloudy"                   },
        {  768, "Fluorescent (daylight)"   },
        {  769, "Fluorescent (warm white)" },
        {  770, "Fluorescent (cool white)" },
        { 1024, "Incandescent"             },
        { 3480, "Custom"                   }
    };

    //! Color, tag 0x1003
    extern const TagDetails fujiColor[] = {
        {   0, "Standard" },
        { 256, "High"     },
        { 512, "Original" }
    };

    //! Tone, tag 0x1004
    extern const TagDetails fujiTone[] = {
        {   0, "Standard" },
        { 256, "Hard"     },
        { 512, "Original" }
    };

    //! FlashMode, tag 0x1010
    extern const TagDetails fujiFlashMode[] = {
        { 0, "Auto"    },
        { 1, "On"      },
        { 2, "Off"     },
        { 3, "Red-eye" }
    };

    //! FocusMode, tag 0x1021
    extern const TagDetails fujiFocusMode[] = {
        { 0, "Auto"   },
        { 1, "Manual" }
    };

    //! PictureMode, tag 0x1031
    extern const TagDetails fujiPictureMode[] = {
        {   0, "Auto"              },
        {   1, "Portrait"          },
        {   2, "Landscape"         },
        {   4, "Sports"            },
        {   5, "Night"             },
        {   6, "Program"           },
        { 256, "Aperture priority" },
        { 512, "Shutter priority"  },
        { 768, "Manual"            }
    };

    //! FinePixColor, tag 0x1210
    extern const TagDetails fujiFinePixColor[] = {
        { 0, "Normal"   },
        { 16, "Chrome" },
        { 48, "B&W" }
    };

    // Fujifilm MakerNote Tag Info
    const TagInfo FujiMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "Version", "Version", "Fujifilm Makernote version", fujiIfdId, makerTags, undefined, printValue),
        TagInfo(0x1000, "Quality", "Quality", "Image quality setting", fujiIfdId, makerTags, asciiString, printValue),
        TagInfo(0x1001, "Sharpness", "Sharpness", "Sharpness setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiSharpness)),
        TagInfo(0x1002, "WhiteBalance", "WhiteBalance", "White balance setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiWhiteBalance)),
        TagInfo(0x1003, "Color", "Color", "Chroma saturation setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiColor)),
        TagInfo(0x1004, "Tone", "Tone", "Contrast setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiTone)),
        TagInfo(0x1010, "FlashMode", "FlashMode", "Flash firing mode setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiFlashMode)),
        TagInfo(0x1011, "FlashStrength", "FlashStrength", "Flash firing strength compensation setting", fujiIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1020, "Macro", "Macro", "Macro mode setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiOffOn)),
        TagInfo(0x1021, "FocusMode", "FocusMode", "Focusing mode setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiFocusMode)),
        TagInfo(0x1022, "0x1022", "0x1022", "Unknown", fujiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1030, "SlowSync", "SlowSync", "Slow synchro mode setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiOffOn)),
        TagInfo(0x1031, "PictureMode", "PictureMode", "Picture mode setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiPictureMode)),
        TagInfo(0x1032, "0x1032", "0x1032", "Unknown", fujiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1100, "Continuous", "Continuous", "Continuous shooting or auto bracketing setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiOffOn)),
        TagInfo(0x1101, "0x1101", "0x1101", "Unknown", fujiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1200, "0x1200", "0x1200", "Unknown", fujiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1210, "FinePixColor", "FinePixColor", "Fuji FinePix Color setting", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiFinePixColor)),
        TagInfo(0x1300, "BlurWarning", "BlurWarning", "Blur warning status", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiOffOn)),
        TagInfo(0x1301, "FocusWarning", "FocusWarning", "Auto Focus warning status", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiOffOn)),
        TagInfo(0x1302, "AeWarning", "AeWarning", "Auto Exposure warning status", fujiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(fujiOffOn)),
        // End of list marker
        TagInfo(0xffff, "(UnknownFujiMakerNoteTag)", "(UnknownFujiMakerNoteTag)", "Unknown FujiMakerNote tag", fujiIfdId, makerTags, invalidTypeId, printValue)
    };

    FujiMakerNote::FujiMakerNote(bool alloc)
        : IfdMakerNote(fujiIfdId, alloc)
    {
        byteOrder_ = littleEndian;
        absShift_ = false;
        byte buf[] = {
            'F', 'U', 'J', 'I', 'F', 'I', 'L', 'M', 0x0c, 0x00, 0x00, 0x00
        };
        readHeader(buf, 12, byteOrder_);
    }

    FujiMakerNote::FujiMakerNote(const FujiMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int FujiMakerNote::readHeader(const byte* buf,
                                  long len,
                                  ByteOrder /*byteOrder*/)
    {
        if (len < 12) return 1;

        header_.alloc(12);
        memcpy(header_.pData_, buf, header_.size_);
        // Read offset to the IFD relative to the start of the makernote
        // from the header. Note that we ignore the byteOrder paramter
        start_ = getUShort(header_.pData_ + 8, byteOrder_);
        return 0;
    }

    int FujiMakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the FUJIFILM prefix
        if (   header_.size_ < 12
            || std::string(reinterpret_cast<char*>(header_.pData_), 8)
                    != std::string("FUJIFILM", 8)) {
            rc = 2;
        }
        return rc;
    }

    FujiMakerNote::AutoPtr FujiMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    FujiMakerNote* FujiMakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote(new FujiMakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    FujiMakerNote::AutoPtr FujiMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    FujiMakerNote* FujiMakerNote::clone_() const
    {
        return new FujiMakerNote(*this);
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createFujiMakerNote(bool        alloc,
                                           const byte* /*buf*/,
                                           long        /*len*/,
                                           ByteOrder   /*byteOrder*/,
                                           long        /*offset*/)
    {
        return MakerNote::AutoPtr(new FujiMakerNote(alloc));
    }

}                                       // namespace Exiv2
