// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2005-2007 Andreas Huggel <ahuggel@gmx.net>
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
  File:      olympusmn.cpp
  Version:   $Rev$
  Author(s): Will Stokes (wuz) <wstokes@gmail.com>
             Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier.gilles@kdemail.net>
  History:   10-Mar-05, wuz: created
  Credits:   See header file.
 */

// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "olympusmn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {


    //! OffOn, multiple tags
    extern const TagDetails olympusOffOn[] = {
        {  0, N_("Off") },
        {  1, N_("On")  }
    };

    //! Quality, tag 0x0201
    extern const TagDetails olympusQuality[] = {
        { 1, N_("Standard Quality (SQ)")    },
        { 2, N_("High Quality (HQ)")        },
        { 3, N_("Super High Quality (SHQ)") },
        { 6, N_("Raw")                      }
    };

    //! Macro, tag 0x0202
    extern const TagDetails olympusMacro[] = {
        {  0, N_("Off")         },
        {  1, N_("On")          },
        {  2, N_("Super macro") }
    };

    //! OneTouchWB, tag 0x0302
    extern const TagDetails olympusOneTouchWb[] = {
        {  0, N_("Off")         },
        {  1, N_("On")          },
        {  2, N_("On (preset)") }
    };

    //! FlashDevice, tag 0x1005
    extern const TagDetails olympusFlashDevice[] = {
        {  0, N_("None")                },
        {  1, N_("Internal")            },
        {  4, N_("External")            },
        {  5, N_("Internal + External") }
    };

    //! FocusMode, tag 0x100b
    extern const TagDetails olympusFocusMode[] = {
        {  0, N_("Auto")   },
        {  1, N_("Manual") }
    };

    //! Sharpness, tag 0x100f
    extern const TagDetails olympusSharpness[] = {
        { 0, N_("Normal") },
        { 1, N_("Hard")   },
        { 2, N_("Soft")   }
    };

    //! Contrast, tag 0x1029
    extern const TagDetails olympusContrast[] = {
        { 0, N_("High")   },
        { 1, N_("Normal") },
        { 2, N_("Low")    }
    };

    //! CCDScanMode, tag 0x1039
    extern const TagDetails olympusCCDScanMode[] = {
        {  0, N_("Interlaced")  },
        {  1, N_("Progressive") }
    };

    // Olympus Tag Info
    const TagInfo OlympusMakerNote::tagInfo_[] = {

        /* TODO:
           add Minolta makenotes tags here (0x0000-0x0103). See Exiftool database.*/

        TagInfo(0x0200, "SpecialMode", N_("Special Mode"),
                N_("Picture taking mode"),
                olympusIfdId, makerTags, unsignedLong, print0x0200),
        TagInfo(0x0201, "Quality", N_("Quality"),
                N_("Image quality setting"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusQuality)),
        TagInfo(0x0202, "Macro", N_("Macro"),
                N_("Macro mode"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusMacro)),
        TagInfo(0x0203, "BWMode", N_("Black & White Mode"),
                N_("Black and white mode"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x0204, "DigitalZoom", N_("Digital Zoom"),
                N_("Digital zoom ratio"),
                olympusIfdId, makerTags, unsignedRational, print0x0204),
        TagInfo(0x0205, "FocalPlaneDiagonal", N_("Focal Plane Diagonal"),
                N_("Focal plane diagonal"),
                olympusIfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0206, "LensDistortionParams", N_("Lens Distortion Parameters"),
                N_("Lens distortion parameters"),
                olympusIfdId, makerTags, signedShort, printValue),
        TagInfo(0x0207, "FirmwareVersion", N_("Firmware Version"),
                N_("Software firmware version"),
                olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0208, "PictureInfo", N_("Picture Info"),
                N_("ASCII format data such as [PictureInfo]"),
                olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0209, "CameraID", N_("Camera ID"),
                N_("Camera ID data"),
                olympusIfdId, makerTags, undefined, printValue),

        /* TODO: Epson Camera tags. See ExifTool database.

        TagInfo(0x020b, "ImageWidth", N_("Image Width"),
                N_("Image width"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x020c, "ImageHeight", N_("Image Height"),
                N_("Image height"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x020d, "Software", N_("Software"),
                N_("Software"),
                olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0280, "PreviewImage", N_("Preview Image"),
                N_("Preview image"),
                olympusIfdId, makerTags, unsignedByte, printValue),
        */

        TagInfo(0x0300, "PreCaptureFrames", N_("Pre Capture Frames"),
                N_("Pre-capture frames"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0301, "0x0301", "0x0301",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0302, "OneTouchWB", N_("One Touch WB"),
                N_("One touch white balance"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOneTouchWb)),
        TagInfo(0x0303, "0x0303", "0x0303",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0304, "0x0304", "0x0304",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0404, "SerialNumber", N_("Serial Number"),
                N_("Serial number"),
                olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"),
                N_("PrintIM information"),
                olympusIfdId, makerTags, undefined, printValue),
        TagInfo(0x0f00, "DataDump1", N_("Data Dump 1"),
                N_("Various camera settings 1"),
                olympusIfdId, makerTags, undefined, printValue),
        TagInfo(0x0f01, "DataDump2", N_("Data Dump 2"),
                N_("Various camera settings 2"),
                olympusIfdId, makerTags, undefined, printValue),
        TagInfo(0x1000, "ShutterSpeed", N_("Shutter Speed"),
                N_("Shutter speed value"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1001, "ISOSpeed", N_("ISO Speed"),
                N_("ISO speed value"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1002, "ApertureValue", N_("Aperture Value"),
                N_("Aperture value"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1003, "Brightness", N_("Brightness"),
                N_("Brightness value"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1004, "FlashMode", N_("Flash Mode"),
                N_("Flash mode"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x1005, "FlashDevice", N_("Flash Device"),
                N_("Flash device"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusFlashDevice)),
        TagInfo(0x1006, "Bracket", N_("Bracket"),
                N_("Exposure compensation value"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1007, "SensorTemperature", N_("Sensor Temperature"),
                N_("Sensor temperature"),
                olympusIfdId, makerTags, signedShort, printValue),
        TagInfo(0x1008, "LensTemperature", N_("Lens Temperature"),
                N_("Lens temperature"),
                olympusIfdId, makerTags, signedShort, printValue),
        TagInfo(0x1009, "0x1009", "0x1009",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100a, "0x100a", "0x100a",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100b, "FocusMode", N_("Focus Mode"),
                N_("Focus mode"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusFocusMode)),
        TagInfo(0x100c, "FocusDistance", N_("Focus Distance"),
                N_("Manual focus distance"),
                olympusIfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x100d, "Zoom", N_("Zoom"),
                N_("Zoom step count"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100e, "MacroFocus", N_("Macro Focus"),
                N_("Macro focus step count"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x100f, "SharpnessFactor", N_("Sharpness Factor"),
                N_("Sharpness factor"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusSharpness)),
        TagInfo(0x1010, "FlashChargeLevel", N_("Flash Charge Level"),
                N_("Flash charge level"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1011, "ColorMatrix", N_("Color Matrix"),
                N_("Color matrix"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1012, "BlackLevel", N_("BlackLevel"),
                N_("Black level"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1013, "0x1013", "0x1013",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1014, "0x1014", "0x1014",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1015, "WhiteBalance", N_("White Balance"),
                N_("White balance mode"),
                olympusIfdId, makerTags, unsignedShort, print0x1015),
        TagInfo(0x1016, "0x1016", "0x1016",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1017, "RedBalance", N_("Red Balance"),
                N_("Red balance"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1018, "BlueBalance", N_("Blue Balance"),
                N_("Blue balance"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1019, "0x1019", "0x1019",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x101a, "SerialNumber2", N_("Serial Number 2"),
                N_("Serial number 2"),
                olympusIfdId, makerTags, asciiString, printValue),
        TagInfo(0x101b, "0x101b", "0x101b",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101c, "0x101c", "0x101c",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101d, "0x101d", "0x101d",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101e, "0x101e", "0x101e",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x101f, "0x101f", "0x101f",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1020, "0x1020", "0x1020",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1021, "0x1021", "0x1021",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1022, "0x1022", "0x1022",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1023, "FlashBias", N_("Flash Bias"),
                N_("Flash exposure compensation"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1024, "0x1024", "0x1024",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1025, "0x1025", "0x1025",
                N_("Unknown"),
                olympusIfdId, makerTags, signedRational, printValue),
        TagInfo(0x1026, "ExternalFlashBounce", N_("External Flash Bounce"),
                N_("External flash bounce"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x1027, "ExternalFlashZoom", N_("External Flash Zoom"),
                N_("External flash zoom"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1028, "ExternalFlashMode", N_("External Flash Mode"),
                N_("External flash mode"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1029, "Contrast", N_("Contrast"),
                N_("Contrast setting"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusContrast)),
        TagInfo(0x102a, "SharpnessFactor", N_("Sharpness Factor"),
                N_("Sharpness factor"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102b, "ColorControl", N_("Color Control"),
                N_("Color control"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102c, "ValidBits", N_("ValidBits"),
                N_("Valid bits"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102d, "CoringFilter", N_("CoringFilter"),
                N_("Coring filter"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x102e, "ImageWidth", N_("Image Width"),
                N_("Image width"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x102f, "ImageHeight", N_("Image Height"),
                N_("Image height"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1030, "0x1030", "0x1030",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1031, "0x1031", "0x1031",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1032, "0x1032", "0x1032",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x1033, "0x1033", "0x1033",
                N_("Unknown"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1034, "CompressionRatio", N_("Compression Ratio"),
                N_("Compression ratio"),
                olympusIfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x1035, "Thumbnail", N_("Thumbnail"),
                N_("Preview image embedded"),
                olympusIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x1036, "ThumbnailOffset", N_("Thumbnail Offset"),
                N_("Offset of the preview image"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1037, "ThumbnailLength", N_("Thumbnail Length"),
                N_("Size of the preview image"),
                olympusIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x1039, "CCDScanMode", N_("CCD Scan Mode"),
                N_("CCD scan mode"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusCCDScanMode)),
        TagInfo(0x103a, "NoiseReduction", N_("Noise Reduction"),
                N_("Noise reduction"),
                olympusIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(olympusOffOn)),
        TagInfo(0x103b, "InfinityLensStep", N_("Infinity Lens Step"),
                N_("Infinity lens step"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x103c, "NearLensStep", N_("Near Lens Step"),
                N_("Near lens step"),
                olympusIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x2010, "Equipment", N_("Equipment Info"),
                N_("Camera equipment information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x2020, "CameraSettings", N_("Camera Settings"),
                N_("Camera Settings information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x2030, "RawDevelopment", N_("Raw Development"),
                N_("Raw development information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x2040, "ImageProcessing", N_("Image Processing"),
                N_("Image processing information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x2050, "FocusInfo", N_("Focus Info"),
                N_("Focus information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x3000, "RawInfo", N_("Raw Info"),
                N_("Raw information"),
                minoltaIfdId, makerTags, undefined, printValue),

        // End of list marker
        TagInfo(0xffff, "(UnknownOlympusMakerNoteTag)", "(UnknownOlympusMakerNoteTag)",
                N_("Unknown OlympusMakerNote tag"),
                olympusIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* OlympusMakerNote::tagList()
    {
        return tagInfo_;
    }

    //! @cond IGNORE
    OlympusMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("OLYMPUS*", "*", createOlympusMakerNote);
        MakerNoteFactory::registerMakerNote(olympusIfdId, MakerNote::AutoPtr(new OlympusMakerNote));

        ExifTags::registerMakerTagInfo(olympusIfdId, tagInfo_);
    }
    //! @endcond

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

    int OlympusMakerNote::readHeader(const byte* buf, long len, ByteOrder /*byteOrder*/)
    {
        if (len < 8) return 1;

        // Copy the header
        header_.alloc(8);
        std::memcpy(header_.pData_, buf, header_.size_);
        // Adjust the offset of the IFD for the prefix
        start_ = 8;
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
        AutoPtr makerNote(new OlympusMakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    OlympusMakerNote::AutoPtr OlympusMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    OlympusMakerNote* OlympusMakerNote::clone_() const
    {
        return new OlympusMakerNote(*this);
    }

    std::ostream& OlympusMakerNote::print0x0200(std::ostream& os, const Value& value)
    {
        if (value.count() != 3 || value.typeId() != unsignedLong) {
            return os << value;
        }
        long l0 = value.toLong(0);
        switch (l0) {
        case 0: os << _("Normal"); break;
        case 2: os << _("Fast"); break;
        case 3: os << _("Panorama"); break;
        default: os << "(" << l0 << ")"; break;
        }
        if (l0 != 0) {
            os << ", ";
            long l1 = value.toLong(1);
            os << _("Sequence number") << " " << l1;
        }
        if (l0 != 0 && l0 != 2) {
            os << ", ";
            long l2 = value.toLong(2);
            switch (l2) {
            case 1: os << _("Left to right"); break;
            case 2: os << _("Right to left"); break;
            case 3: os << _("Bottom to top"); break;
            case 4: os << _("Top to bottom"); break;
            default: os << "(" << l2 << ")"; break;
            }
        }
        return os;
    } // OlympusMakerNote::print0x0200

    std::ostream& OlympusMakerNote::print0x0204(std::ostream& os, const Value& value)
    {
        float f = value.toFloat();
        if (f == 0.0 || f == 1.0) return os << _("None");
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1) << f << "x";
        os.copyfmt(oss);
        return os;
    } // OlympusMakerNote::print0x0204

    std::ostream& OlympusMakerNote::print0x1015(std::ostream& os, const Value& value)
    {
        if (value.count() != 2 || value.typeId() != unsignedShort) {
            return os << value;
        }
        short l0 = (short)value.toLong(0);
        if (l0 != 1) {
            os << _("Auto");
        }
        else {
            short l1 = (short)value.toLong(1);
            if (l1 != 1) {
                switch (l0) {
                case 0: os << _("Auto"); break;
                default: os << _("Auto") << " (" << l0 << ")"; break;
                }
            }
            else if (l1 != 2) {
                switch (l0) {
                case 2: os << _("3000 Kelvin"); break;
                case 3: os << _("3700 Kelvin"); break;
                case 4: os << _("4000 Kelvin"); break;
                case 5: os << _("4500 Kelvin"); break;
                case 6: os << _("5500 Kelvin"); break;
                case 7: os << _("6500 Kelvin"); break;
                case 8: os << _("7500 Kelvin"); break;
                default: os << value; break;
                }
            }
            else if (l1 != 3) {
                switch (l0) {
                case 0: os << _("One-touch"); break;
                default: os << value; break;
                }
            }
            else {
                return os << value;
            }
        }
        return os;
    } // OlympusMakerNote::print0x1015

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createOlympusMakerNote(bool alloc, const byte* /*buf*/, long /*len*/,
                                              ByteOrder /*byteOrder*/, long /*offset*/)
    {
        return MakerNote::AutoPtr(new OlympusMakerNote(alloc));
    }

}                                       // namespace Exiv2
