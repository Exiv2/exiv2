// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2009 Andreas Huggel <ahuggel@gmx.net>
 *
 * Lens database to decode Exif.Nikon3.LensData
 * Copyright (C) 2005-2008 Robert Rottmerhusen <lens_id@rottmerhusen.com>
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
  File:      nikonmn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier.gilles@kdemail.net>
  History:   17-May-04, ahu: created
             25-May-04, ahu: combined all Nikon formats in one component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "nikonmn.hpp"
#include "value.hpp"
#include "image.hpp"
#include "tags.hpp"
#include "error.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! OffOn, multiple tags
    extern const TagDetails nikonOffOn[] = {
        {  0, N_("Off") },
        {  1, N_("On")  }
    };

    //! Off, Low, Normal, High, multiple tags
    extern const TagDetails nikonOlnh[] = {
        {  0, N_("Off")    },
        {  1, N_("Low")    },
        {  3, N_("Normal") },
        {  5, N_("High")   }
    };

    //! Focus area for Nikon cameras.
    extern const char *nikonFocusarea[] = {
        N_("Single area"),
        N_("Dynamic area"),
        N_("Dynamic area, closest subject"),
        N_("Group dynamic"),
        N_("Single area (wide)"),
        N_("Dynamic area (wide)")
    };

    // Roger Larsson: My guess is that focuspoints will follow autofocus sensor
    // module. Note that relative size and position will vary depending on if
    // "wide" or not
    //! Focus points for Nikon cameras, used for Nikon 1 and Nikon 3 makernotes.
    extern const char *nikonFocuspoints[] = {
        N_("Center"),
        N_("Top"),
        N_("Bottom"),
        N_("Left"),
        N_("Right"),
        N_("Upper-left"),
        N_("Upper-right"),
        N_("Lower-left"),
        N_("Lower-right"),
        N_("Left-most"),
        N_("Right-most")
    };

    //! FlashComp, tag 0x0012
    extern const TagDetails nikonFlashComp[] = {
        // From the PHP JPEG Metadata Toolkit
        { 0x06, "+1.0 EV" },
        { 0x04, "+0.7 EV" },
        { 0x03, "+0.5 EV" },
        { 0x02, "+0.3 EV" },
        { 0x00,  "0.0 EV" },
        { 0xfe, "-0.3 EV" },
        { 0xfd, "-0.5 EV" },
        { 0xfc, "-0.7 EV" },
        { 0xfa, "-1.0 EV" },
        { 0xf8, "-1.3 EV" },
        { 0xf7, "-1.5 EV" },
        { 0xf6, "-1.7 EV" },
        { 0xf4, "-2.0 EV" },
        { 0xf2, "-2.3 EV" },
        { 0xf1, "-2.5 EV" },
        { 0xf0, "-2.7 EV" },
        { 0xee, "-3.0 EV" }
    };

    //! ColorSpace, tag 0x001e
    extern const TagDetails nikonColorSpace[] = {
        { 1, N_("sRGB")      },
        { 2, N_("Adobe RGB") }
    };

    //! FlashMode, tag 0x0087
    extern const TagDetails nikonFlashMode[] = {
        { 0, N_("Did not fire")         },
        { 1, N_("Fire, manual")         },
        { 7, N_("Fire, external")       },
        { 8, N_("Fire, commander mode") },
        { 9, N_("Fire, TTL mode")       }
    };

    //! ShootingMode, tag 0x0089
    extern const TagDetailsBitmask nikonShootingMode[] = {
        { 0x0001, N_("Continuous")               },
        { 0x0002, N_("Delay")                    },
        { 0x0004, N_("PC control")               },
        { 0x0010, N_("Exposure bracketing")      },
        { 0x0020, N_("Auto ISO") },
        { 0x0040, N_("White balance bracketing") },
        { 0x0080, N_("IR control")               }
    };

    //! ShootingMode D70, tag 0x0089
    extern const TagDetailsBitmask nikonShootingModeD70[] = {
        { 0x0001, N_("Continuous")               },
        { 0x0002, N_("Delay")                    },
        { 0x0004, N_("PC control")               },
        { 0x0010, N_("Exposure bracketing")      },
        { 0x0020, N_("Unused LE-NR slowdown") },
        { 0x0040, N_("White balance bracketing") },
        { 0x0080, N_("IR control")               }
    };

    //! AutoBracketRelease, tag 0x008a
    extern const TagDetails nikonAutoBracketRelease[] = {
        { 0, N_("None")           },
        { 1, N_("Auto release")   },
        { 2, N_("Manual release") }
    };

    //! NEFCompression, tag 0x0093
    extern const TagDetails nikonNefCompression[] = {
        {  1, N_("Lossy (type 1)") },
        {  2, N_("Uncompressed")   },
        {  3, N_("Lossless")       },
        {  4, N_("Lossy (type 2)") }
    };

    //! RetouchHistory, tag 0x009e
    extern const TagDetails nikonRetouchHistory[] = {
        {  0, N_("None")          },
        {  3, N_("B & W")         },
        {  4, N_("Sepia")         },
        {  5, N_("Trim")          },
        {  6, N_("Small picture") },
        {  7, N_("D-Lighting")    },
        {  8, N_("Red eye")       },
        {  9, N_("Cyanotype")     },
        { 10, N_("Sky light")     },
        { 11, N_("Warm tone")     },
        { 12, N_("Color custom")  },
        { 13, N_("Image overlay") }
    };

    //! HighISONoiseReduction, tag 0x00b1
    extern const TagDetails nikonHighISONoiseReduction[] = {
        { 0, N_("Off")     },
        { 1, N_("Minimal") },
        { 2, N_("Low")     },
        { 4, N_("Normal")  },
        { 6, N_("High")    }
    };

    // Nikon1 MakerNote Tag Info
    const TagInfo Nikon1MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", N_("Version"),
                N_("Nikon Makernote version"),
                nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x0002, "ISOSpeed", N_("ISO Speed"),
                N_("ISO speed setting"),
                nikon1IfdId, makerTags, unsignedShort, print0x0002),
        TagInfo(0x0003, "ColorMode", N_("Color Mode"),
                N_("Color mode"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0004, "Quality", N_("Quality"),
                N_("Image quality setting"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "Sharpening", N_("Sharpening"),
                N_("Image sharpening setting"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "Focus", N_("Focus"),
                N_("Focus mode"),
                nikon1IfdId, makerTags, asciiString, print0x0007),
        TagInfo(0x0008, "FlashSetting", N_("Flash Setting"),
                N_("Flash setting"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "0x000a", "0x000a",
                N_("Unknown"),
                nikon1IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x000f, "ISOSelection", N_("ISO Selection"),
                N_("ISO selection"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0010, "DataDump", N_("Data Dump"),
                N_("Data dump"),
                nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x0080, "ImageAdjustment", N_("Image Adjustment"),
                N_("Image adjustment setting"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0082, "AuxiliaryLens", N_("Auxiliary Lens"),
                N_("Auxiliary lens (adapter)"),
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0085, "FocusDistance", N_("Focus Distance"),
                N_("Manual focus distance"),
                nikon1IfdId, makerTags, unsignedRational, print0x0085),
        TagInfo(0x0086, "DigitalZoom", N_("Digital Zoom"),
                N_("Digital zoom setting"),
                nikon1IfdId, makerTags, unsignedRational, print0x0086),
        TagInfo(0x0088, "AFFocusPos", N_("AF Focus Position"),
                N_("AF focus position information"),
                nikon1IfdId, makerTags, undefined, print0x0088),

        // End of list marker
        TagInfo(0xffff, "(UnknownNikon1MnTag)", "(UnknownNikon1MnTag)",
                N_("Unknown Nikon1MakerNote tag"),
                nikon1IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon1MakerNote::tagList()
    {
        return tagInfo_;
    }

    std::ostream& Nikon1MakerNote::print0x0002(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
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
                                               const Value& value,
                                               const ExifData*)
    {
        std::string focus = value.toString();
        if      (focus == "AF-C  ") os << _("Continuous autofocus");
        else if (focus == "AF-S  ") os << _("Single autofocus");
        else                        os << "(" << value << ")";
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0085(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << _("Unknown");
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
                                               const Value& value,
                                               const ExifData*)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << _("Not used");
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
                                               const Value& value,
                                               const ExifData*)
    {
        if (value.count() >= 1) {
            unsigned long focusArea = value.toLong(0);
            os << nikonFocusarea[focusArea] ;
        }
        if (value.count() >= 2) {
            os << "; ";
            unsigned long focusPoint = value.toLong(1);

            switch (focusPoint) {
            // Could use array nikonFokuspoints
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                os << nikonFocuspoints[focusPoint];
                break;
            default:
                os << value;
                if (focusPoint < sizeof(nikonFocuspoints)/sizeof(nikonFocuspoints[0]))
                    os << " " << _("guess") << " " << nikonFocuspoints[focusPoint];
                break;
            }
        }
        if (value.count() >= 3) {
            unsigned long focusPointsUsed1 = value.toLong(2);
            unsigned long focusPointsUsed2 = value.toLong(3);

            if (focusPointsUsed1 != 0 && focusPointsUsed2 != 0)
            {
                os << "; [";

                if (focusPointsUsed1 & 1)
                    os << nikonFocuspoints[0] << " ";
                if (focusPointsUsed1 & 2)
                    os << nikonFocuspoints[1] << " ";
                if (focusPointsUsed1 & 4)
                    os << nikonFocuspoints[2] << " ";
                if (focusPointsUsed1 & 8)
                    os << nikonFocuspoints[3] << " ";
                if (focusPointsUsed1 & 16)
                    os << nikonFocuspoints[4] << " ";
                if (focusPointsUsed1 & 32)
                    os << nikonFocuspoints[5] << " ";
                if (focusPointsUsed1 & 64)
                    os << nikonFocuspoints[6] << " ";
                if (focusPointsUsed1 & 128)
                    os << nikonFocuspoints[7] << " ";

                if (focusPointsUsed2 & 1)
                    os << nikonFocuspoints[8] << " ";
                if (focusPointsUsed2 & 2)
                    os << nikonFocuspoints[9] << " ";
                if (focusPointsUsed2 & 4)
                    os << nikonFocuspoints[10] << " ";

                os << "]";
            }
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    //! Quality, tag 0x0003
    extern const TagDetails nikon2Quality[] = {
        { 1, N_("VGA Basic")   },
        { 2, N_("VGA Normal")  },
        { 3, N_("VGA Fine")    },
        { 4, N_("SXGA Basic")  },
        { 5, N_("SXGA Normal") },
        { 6, N_("SXGA Fine")   }
    };

    //! ColorMode, tag 0x0004
    extern const TagDetails nikon2ColorMode[] = {
        { 1, N_("Color")      },
        { 2, N_("Monochrome") }
    };

    //! ImageAdjustment, tag 0x0005
    extern const TagDetails nikon2ImageAdjustment[] = {
        { 0, N_("Normal")    },
        { 1, N_("Bright+")   },
        { 2, N_("Bright-")   },
        { 3, N_("Contrast+") },
        { 4, N_("Contrast-") }
    };

    //! ISOSpeed, tag 0x0006
    extern const TagDetails nikon2IsoSpeed[] = {
        { 0, "80"  },
        { 2, "160" },
        { 4, "320" },
        { 5, "100" }
    };

    //! WhiteBalance, tag 0x0007
    extern const TagDetails nikon2WhiteBalance[] = {
        { 0, N_("Auto")         },
        { 1, N_("Preset")       },
        { 2, N_("Daylight")     },
        { 3, N_("Incandescent") },
        { 4, N_("Fluorescent")  },
        { 5, N_("Cloudy")       },
        { 6, N_("Speedlight")   }
    };

    // Nikon2 MakerNote Tag Info
    const TagInfo Nikon2MakerNote::tagInfo_[] = {
        TagInfo(0x0002, "0x0002", "0x0002",
                N_("Unknown"),
                nikon2IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0003, "Quality", N_("Quality"),
                N_("Image quality setting"),
                nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2Quality)),
        TagInfo(0x0004, "ColorMode", N_("Color Mode"),
                N_("Color mode"),
                nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2ColorMode)),
        TagInfo(0x0005, "ImageAdjustment", N_("Image Adjustment"),
                N_("Image adjustment setting"),
                nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2ImageAdjustment)),
        TagInfo(0x0006, "ISOSpeed", N_("ISO Speed"),
                N_("ISO speed setting"),
                nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2IsoSpeed)),
        TagInfo(0x0007, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2WhiteBalance)),
        TagInfo(0x0008, "Focus", N_("Focus Mode"),
                N_("Focus mode"),
                nikon2IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0009, "0x0009", "0x0009",
                N_("Unknown"),
                nikon2IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "DigitalZoom", N_("Digital Zoom"),
                N_("Digital zoom setting"),
                nikon2IfdId, makerTags, unsignedRational, print0x000a),
        TagInfo(0x000b, "AuxiliaryLens", N_("Auxiliary Lens"),
                N_("Auxiliary lens (adapter)"),
                nikon2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0f00, "0x0f00", "0x0f00",
                N_("Unknown"),
                nikon2IfdId, makerTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon2MnTag)", "(UnknownNikon2MnTag)",
                N_("Unknown Nikon2MakerNote tag"),
                nikon2IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon2MakerNote::tagList()
    {
        return tagInfo_;
    }

    std::ostream& Nikon2MakerNote::print0x000a(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << _("Not used");
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
    const TagInfo Nikon3MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", N_("Version"), N_("Nikon Makernote version"), nikon3IfdId, makerTags, undefined, printExifVersion),
        TagInfo(0x0002, "ISOSpeed", N_("ISO Speed"), N_("ISO speed setting"), nikon3IfdId, makerTags, unsignedShort, print0x0002),
        TagInfo(0x0003, "ColorMode", N_("Color Mode"), N_("Color mode"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0004, "Quality", N_("Quality"), N_("Image quality setting"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "WhiteBalance", N_("White Balance"), N_("White balance"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "Sharpening", N_("Sharpening"), N_("Image sharpening setting"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "Focus", N_("Focus"), N_("Focus mode"), nikon3IfdId, makerTags, asciiString, print0x0007),
        TagInfo(0x0008, "FlashSetting", N_("Flash Setting"), N_("Flash setting"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0009, "FlashDevice", N_("Flash Device"), N_("Flash device"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "0x000a", "0x000a", N_("Unknown"), nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x000b, "WhiteBalanceBias", N_("White Balance Bias"), N_("White balance bias"), nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x000c, "WB_RBLevels", N_("WB RB Levels"), N_("WB RB levels"), nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x000d, "ProgramShift", N_("Program Shift"), N_("Program shift"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x000e, "ExposureDiff", N_("Exposure Difference"), N_("Exposure difference"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x000f, "ISOSelection", N_("ISO Selection"), N_("ISO selection"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0010, "DataDump", N_("Data Dump"), N_("Data dump"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0011, "Preview", N_("Pointer to a preview image"), N_("Offset to an IFD containing a preview image"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0012, "FlashComp", N_("Flash Comp"), N_("Flash compensation setting"), nikon3IfdId, makerTags, undefined, EXV_PRINT_TAG(nikonFlashComp)),
        TagInfo(0x0013, "ISOSettings", N_("ISO Settings"), N_("ISO setting"), nikon3IfdId, makerTags, unsignedShort, print0x0002), // use 0x0002 print fct
        TagInfo(0x0016, "ImageBoundary", N_("Image Boundary"), N_("Image boundary"), nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0017, "0x0017", "0x0017", N_("Unknown"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0018, "FlashBracketComp", N_("Flash Bracket Comp"), N_("Flash bracket compensation applied"), nikon3IfdId, makerTags, undefined, EXV_PRINT_TAG(nikonFlashComp)), // use 0x0012 print fct
        TagInfo(0x0019, "ExposureBracketComp", N_("Exposure Bracket Comp"), N_("AE bracket compensation applied"), nikon3IfdId, makerTags, signedRational, printValue),
        TagInfo(0x001a, "ImageProcessing", N_("Image Processing"), N_("Image processing"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x001b, "CropHiSpeed", N_("Crop High Speed"), N_("Crop high speed"), nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001d, "SerialNumber", N_("Serial Number"), N_("Serial Number"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x001e, "ColorSpace", N_("Color Space"), N_("Color space"), nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonColorSpace)),
        TagInfo(0x001f, "VRInfo", N_("VR Info"), N_("VR info"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0020, "ImageAuthentication", N_("Image Authentication"), N_("Image authentication"), nikon3IfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonOffOn)),
        TagInfo(0x0022, "ActiveDLighting", N_("ActiveD-Lighting"), N_("ActiveD-lighting"), nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonOlnh)),
        TagInfo(0x0023, "PictureControl", N_("Picture Control"), N_(" Picture control"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0024, "WorldTime", N_("World Time"), N_("World time"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0025, "ISOInfo", N_("ISO Info"), N_("ISO info"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x002a, "VignetteControl", N_("Vignette Control"), N_("Vignette control"), nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonOlnh)),
        TagInfo(0x0080, "ImageAdjustment", N_("Image Adjustment"), N_("Image adjustment setting"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0081, "ToneComp", N_("Tone Compensation"), N_("Tone compensation"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0082, "AuxiliaryLens", N_("Auxiliary Lens"), N_("Auxiliary lens (adapter)"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0083, "LensType", N_("Lens Type"), N_("Lens type"), nikon3IfdId, makerTags, unsignedByte, print0x0083),
        TagInfo(0x0084, "Lens", N_("Lens"), N_("Lens"), nikon3IfdId, makerTags, unsignedRational, print0x0084),
        TagInfo(0x0085, "FocusDistance", N_("Focus Distance"), N_("Manual focus distance"), nikon3IfdId, makerTags, unsignedRational, print0x0085),
        TagInfo(0x0086, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom setting"), nikon3IfdId, makerTags, unsignedRational, print0x0086),
        TagInfo(0x0087, "FlashMode", N_("Flash Mode"), N_("Mode of flash used"), nikon3IfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonFlashMode)),
        TagInfo(0x0088, "AFInfo", N_("AF Info"), N_("AF info"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0089, "ShootingMode", N_("Shooting Mode"), N_("Shooting mode"), nikon3IfdId, makerTags, unsignedShort, print0x0089),
        TagInfo(0x008a, "AutoBracketRelease", N_("Auto Bracket Release"), N_("Auto bracket release"), nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonAutoBracketRelease)),
        TagInfo(0x008b, "LensFStops", N_("Lens FStops"), N_("Lens FStops"), nikon3IfdId, makerTags, undefined, print0x008b),
        TagInfo(0x008c, "ContrastCurve", N_("Contrast Curve"), N_("Contrast curve"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x008d, "ColorHue", N_("Color Hue"), N_("Color hue"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x008f, "SceneMode", N_("Scene Mode"), N_("Scene mode"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0090, "LightSource", N_("Light Source"), N_("Light source"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0091, "ShotInfo", "Shot Info", N_("Shot info"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0092, "HueAdjustment", N_("Hue Adjustment"), N_("Hue adjustment"), nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0093, "NEFCompression", N_("NEF Compression"), N_("NEF compression"), nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonNefCompression)),
        TagInfo(0x0094, "Saturation", N_("Saturation"), N_("Saturation"), nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0095, "NoiseReduction", N_("Noise Reduction"), N_("Noise reduction"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0096, "LinearizationTable", N_("Linearization Table"), N_("Linearization table"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0097, "ColorBalance", N_("Color Balance"), N_("Color balance"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0098, "LensData", N_("Lens Data"), N_("Lens data settings"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0099, "RawImageCenter", N_("Raw Image Center"), N_("Raw image center"), nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009a, "SensorPixelSize", N_("Sensor Pixel Size"), N_("Sensor pixel size"), nikon3IfdId, makerTags, unsignedRational, print0x009a),
        TagInfo(0x009b, "0x009b", "0x009b", N_("Unknown"), nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009c, "SceneAssist", N_("Scene Assist"), N_("Scene assist"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x009e, "RetouchHistory", N_("Retouch History"), N_("Retouch history"), nikon3IfdId, makerTags, unsignedShort, print0x009e),
        TagInfo(0x009f, "0x009f", "0x009f", N_("Unknown"), nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x00a0, "SerialNO", N_("Serial NO"), N_("Camera serial number, usually starts with \"NO= \""), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00a2, "ImageDataSize", N_("Image Data Size"), N_("Image data size"), nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a3, "0x00a3", "0x00a3", N_("Unknown"), nikon3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(0x00a5, "ImageCount", N_("Image Count"), N_("Image count"), nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a6, "DeletedImageCount", N_("Deleted Image Count"), N_("Deleted image count"), nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a7, "ShutterCount", N_("Shutter Count"), N_("Number of shots taken by camera"), nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a8, "FlashInfo", "Flash Info", N_("Flash info"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x00a9, "ImageOptimization", N_("Image Optimization"), N_("Image optimization"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00aa, "Saturation", N_("Saturation"), N_("Saturation"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ab, "VariProgram", N_("Program Variation"), N_("Program variation"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ac, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ad, "AFResponse", N_("AF Response"), N_("AF response"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00b0, "MultiExposure", "Multi Exposure", N_("Multi exposure"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x00b1, "HighISONoiseReduction", N_("High ISO Noise Reduction"), N_("High ISO Noise Reduction"), nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonHighISONoiseReduction)),
        TagInfo(0x00b3, "ToningEffect", "Toning Effect", N_("Toning effect"), nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00b7, "AFInfo2", "AF Info 2", N_("AF info 2"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x00b8, "FileInfo", "File Info", N_("File info"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"), N_("PrintIM information"), nikon3IfdId, makerTags, undefined, printValue),
        // TODO: Add Capture Data decoding implementation.
        TagInfo(0x0e01, "CaptureData", N_("Capture Data"), N_("Capture data"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0e09, "CaptureVersion", N_("Capture Version"), N_("Capture version"), nikon3IfdId, makerTags, asciiString, printValue), 
        // TODO: Add Capture Offsets decoding implementation.
        TagInfo(0x0e0e, "CaptureOffsets", N_("Capture Offsets"), N_("Capture offsets"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0e10, "ScanIFD", "Scan IFD", N_("Scan IFD"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0e1d, "ICCProfile", "ICC Profile", N_("ICC profile"), nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0e1e, "CaptureOutput", "Capture Output", N_("Capture output"), nikon3IfdId, makerTags, undefined, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon3MnTag)", "(UnknownNikon3MnTag)", N_("Unknown Nikon3MakerNote tag"), nikon3IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagList()
    {
        return tagInfo_;
    }

    //! YesNo, used for DaylightSavings, tag index 2
    extern const TagDetails nikonYesNo[] = {
        { 0, N_("No")    },
        { 1, N_("Yes")   }
    };

    //! DateDisplayFormat, tag index 3
    extern const TagDetails nikonDateDisplayFormat[] = {
        { 0, N_("Y/M/D") },
        { 1, N_("M/D/Y") },
        { 2, N_("D/M/Y") }
    };

    //! OnOff
    extern const TagDetails nikonOnOff[] = {
        {  1, N_("On")  },
        {  2, N_("Off") }
    };

    // Nikon3 Vibration Reduction Tag Info
    const TagInfo Nikon3MakerNote::tagInfoVr_[] = {
        TagInfo(0, "Version", N_("Version"), N_("Version"), nikonVrIfdId, makerTags, undefined, printExifVersion),
        TagInfo(4, "VibrationReduction", N_("Vibration Reduction"), N_("Vibration reduction"), nikonVrIfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonOnOff)),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonVrTag)", "(UnknownNikonVrTag)", N_("Unknown Nikon Vibration Reduction Tag"), nikonVrIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListVr()
    {
        return tagInfoVr_;
    }

    //! Adjust
    extern const TagDetails nikonAdjust[] = {
        {  0, N_("Default Settings") },
        {  1, N_("Quick Adjust")     },
        {  2, N_("Full Control")     }
    };

    //! FilterEffect
    extern const TagDetails nikonFilterEffect[] = {
        { 0x80, N_("Off")    },
        { 0x81, N_("Yellow") },
        { 0x82, N_("Orange") },
        { 0x83, N_("Red")    },
        { 0x84, N_("Green")  },
        { 0xff, N_("n/a")    }
    };

    //! ToningEffect
    extern const TagDetails nikonToningEffect[] = {
        { 0x80, N_("B&W")         },
        { 0x81, N_("Sepia")       },
        { 0x82, N_("Cyanotype")   },
        { 0x83, N_("Red")         },
        { 0x84, N_("Yellow")      },
        { 0x85, N_("Green")       },
        { 0x86, N_("Blue-green")  },
        { 0x87, N_("Blue")        },
        { 0x88, N_("Purple-blue") },
        { 0x89, N_("Red-purple")  },
        { 0xff, N_("n/a")         }
    };

    // Nikon3 Picture Control Tag Info
    const TagInfo Nikon3MakerNote::tagInfoPc_[] = {
        TagInfo( 0, "Version", N_("Version"), N_("Version"), nikonPcIfdId, makerTags, undefined, printExifVersion),
        TagInfo( 4, "Name", N_("Name"), N_("Name"), nikonPcIfdId, makerTags, asciiString, printValue),
        TagInfo(24, "Base", N_("Base"), N_("Base"), nikonPcIfdId, makerTags, asciiString, printValue),
        TagInfo(48, "Adjust", N_("Adjust"), N_("Adjust"), nikonPcIfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonAdjust)),
        TagInfo(49, "QuickAdjust", N_("Quick Adjust"), N_("Quick adjust"), nikonPcIfdId, makerTags, unsignedByte, printValue),
        TagInfo(50, "Sharpness", N_("Sharpness"), N_("Sharpness"), nikonPcIfdId, makerTags, unsignedByte, printValue),
        TagInfo(51, "Contrast", N_("Contrast"), N_("Contrast"), nikonPcIfdId, makerTags, unsignedByte, printValue),
        TagInfo(52, "Brightness", N_("Brightness"), N_("Brightness"), nikonPcIfdId, makerTags, unsignedByte, printValue),
        TagInfo(53, "Saturation", N_("Saturation"), N_("Saturation"), nikonPcIfdId, makerTags, unsignedByte, printValue),
        TagInfo(54, "HueAdjustment", N_("Hue Adjustment"), N_("Hue adjustment"), nikonPcIfdId, makerTags, unsignedByte, printValue),
        TagInfo(55, "FilterEffect", N_("Filter Effect"), N_("Filter effect"), nikonPcIfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonFilterEffect)),
        TagInfo(56, "ToningEffect", N_("Toning Effect"), N_("Toning effect"), nikonPcIfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonToningEffect)),
        TagInfo(57, "ToningSaturation", N_("Toning Saturation"), N_("Toning saturation"), nikonPcIfdId, makerTags, unsignedByte, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonPcTag)", "(UnknownNikonPcTag)", N_("Unknown Nikon Picture Control Tag"), nikonPcIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListPc()
    {
        return tagInfoPc_;
    }

    // Nikon3 World Time Tag Info
    const TagInfo Nikon3MakerNote::tagInfoWt_[] = {
        TagInfo(0, "Timezone", N_("Timezone"), N_("Timezone"), nikonWtIfdId, makerTags, signedShort, printValue),
        TagInfo(2, "DaylightSavings", N_("Daylight Savings"), N_("Daylight savings"), nikonWtIfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonYesNo)),
        TagInfo(3, "DateDisplayFormat", N_("Date Display Format"), N_("Date display format"), nikonWtIfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonDateDisplayFormat)),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonWtTag)", "(UnknownNikonWtTag)", N_("Unknown Nikon World Time Tag"), nikonWtIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListWt()
    {
        return tagInfoWt_;
    }

    //! ISOExpansion, tag index 4 and 10
    extern const TagDetails nikonIsoExpansion[] = {
        { 0x000, N_("Off")    },
        { 0x101, N_("Hi 0.3") },
        { 0x102, N_("Hi 0.5") },
        { 0x103, N_("Hi 0.7") },
        { 0x104, N_("Hi 1.0") },
        { 0x105, N_("Hi 1.3") },
        { 0x106, N_("Hi 1.5") },
        { 0x107, N_("Hi 1.7") },
        { 0x108, N_("Hi 2.0") },
        { 0x201, N_("Lo 0.3") },
        { 0x202, N_("Lo 0.5") },
        { 0x203, N_("Lo 0.7") },
        { 0x204, N_("Lo 1.0") }
    };

    // Nikon3 ISO Info Tag Info
    const TagInfo Nikon3MakerNote::tagInfoIi_[] = {
        TagInfo( 0, "ISO", N_("ISO"), N_("ISO"), nikonIiIfdId, makerTags, unsignedByte, printIiIso),
        TagInfo( 4, "ISOExpansion", N_("ISO Expansion"), N_("ISO expansion"), nikonIiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonIsoExpansion)),
        TagInfo( 6, "ISO2", N_("ISO 2"), N_("ISO 2"), nikonIiIfdId, makerTags, unsignedByte, printIiIso),
        TagInfo(10, "ISOExpansion2", N_("ISO Expansion 2"), N_("ISO expansion 2"), nikonIiIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonIsoExpansion)),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonIiTag)", "(UnknownNikonIiTag)", N_("Unknown Nikon Iso Info Tag"), nikonIiIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListIi()
    {
        return tagInfoIi_;
    }

    //! AfAreaMode
    extern const TagDetails nikonAfAreaMode[] = {
        { 0, N_("Single Area")                   },
        { 1, N_("Dynamic Area")                  },
        { 2, N_("Dynamic Area, Closest Subject") },
        { 3, N_("Group Dynamic")                 },
        { 4, N_("Single Area (wide)")            },
        { 5, N_("Dynamic Area (wide)")           }
    };

    //! AfPoint
    extern const TagDetails nikonAfPoint[] = {
        { 0, N_("Center")      },
        { 1, N_("Top")         },
        { 2, N_("Bottom")      },
        { 3, N_("Mid-left")    },
        { 4, N_("Mid-right")   },
        { 5, N_("Upper-left")  },
        { 6, N_("Upper-right") },
        { 7, N_("Lower-left")  },
        { 8, N_("Lower-right") },
        { 9, N_("Far Left")    },
        { 10, N_("Far Right")  }
    };

    //! AfPointsInFocus
    extern const TagDetailsBitmask nikonAfPointsInFocus[] = {
        { 0x0001, N_("Center")        },
        { 0x0002, N_("Top")           },
        { 0x0004, N_("Bottom")        },
        { 0x0008, N_("Mid-left")      },
        { 0x0010, N_("Mid-right")     },
        { 0x0020, N_("Upper-left")    },
        { 0x0040, N_("Upper-right")   },
        { 0x0080, N_("Lower-left")    },
        { 0x0100, N_("Lower-right")   },
        { 0x0200, N_("Far Left")      },
        { 0x0400, N_("Far Right")     }
    };

    // Nikon3 Auto Focus Tag Info
    const TagInfo Nikon3MakerNote::tagInfoAf_[] = {
        TagInfo( 0, "AFAreaMode", N_("AF Area Mode"), N_("AF area mode"), nikonAfIfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonAfAreaMode)),
        TagInfo( 1, "AFPoint", N_("AF Point"), N_("AF point"), nikonAfIfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonAfPoint)),
        TagInfo( 2, "AFPointsInFocus", N_("AF Points In Focus"), N_("AF points in focus"), nikonAfIfdId, makerTags, unsignedShort, printAfPointsInFocus),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonAfTag)", "(UnknownNikonAfTag)", N_("Unknown Nikon Auto Focus Tag"), nikonAfIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListAf()
    {
        return tagInfoAf_;
    }

    // Nikon3 Shot Info D80 Tag Info
    const TagInfo Nikon3MakerNote::tagInfoSi1_[] = {
        TagInfo(   0, "Version", N_("Version"), N_("Version"), nikonSi1IfdId, makerTags, unsignedByte, printExifVersion),
        TagInfo( 586, "ShutterCount", N_("Shutter Count"), N_("Shutter count"), nikonSi1IfdId, makerTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonSi1Tag)", "(UnknownNikonSi1Tag)", N_("Unknown Nikon Shot Info D80 Tag"), nikonSi1IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListSi1()
    {
        return tagInfoSi1_;
    }

    // Nikon3 Shot Info D40 Tag Info
    const TagInfo Nikon3MakerNote::tagInfoSi2_[] = {
        TagInfo(   0, "Version", N_("Version"), N_("Version"), nikonSi2IfdId, makerTags, unsignedByte, printExifVersion),
        TagInfo( 582, "ShutterCount", N_("Shutter Count"), N_("Shutter count"), nikonSi2IfdId, makerTags, unsignedLong, printValue),
        TagInfo( 738, "FlashLevel", N_("Flash Level"), N_("Flash level"), nikonSi2IfdId, makerTags, unsignedByte, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonSi2Tag)", "(UnknownNikonSi2Tag)", N_("Unknown Nikon Shot Info D40 Tag"), nikonSi2IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListSi2()
    {
        return tagInfoSi2_;
    }

    //! AfFineTuneAdj D300 (a)
    extern const TagDetails nikonAfFineTuneAdj1[] = {
        { 0x0000, N_("0")   },
        { 0x003a, N_("+1")  },
        { 0x003b, N_("+2")  },
        { 0x003c, N_("+4")  },
        { 0x003d, N_("+8")  },
        { 0x003e, N_("+16") },
        { 0x00c2, N_("-16") },
        { 0x00c3, N_("-8")  },
        { 0x00c4, N_("-4")  },
        { 0x00c5, N_("-2")  },
        { 0x00c6, N_("-1")  },
        { 0x103e, N_("+17") },
        { 0x10c2, N_("-17") },
        { 0x203d, N_("+9")  },
        { 0x203e, N_("+18") },
        { 0x20c2, N_("-18") },
        { 0x20c3, N_("-9")  },
        { 0x303e, N_("+19") },
        { 0x30c2, N_("-19") },
        { 0x403c, N_("+5")  },
        { 0x403d, N_("+10") },
        { 0x403e, N_("+20") },
        { 0x40c2, N_("-20") },
        { 0x40c3, N_("-10") },
        { 0x40c4, N_("-5")  },
        { 0x603d, N_("+11") },
        { 0x60c3, N_("-11") },
        { 0x803b, N_("+3")  },
        { 0x803c, N_("+6")  },
        { 0x803d, N_("+12") },
        { 0x80c3, N_("-12") },
        { 0x80c4, N_("-6")  },
        { 0x80c5, N_("-3")  },
        { 0xa03d, N_("+13") },
        { 0xa0c3, N_("-13") },
        { 0xc03c, N_("+7")  },
        { 0xc03d, N_("+14") },
        { 0xc0c3, N_("-14") },
        { 0xc0c4, N_("-7")  },
        { 0xe03d, N_("+15") },
        { 0xe0c3, N_("-15") }
    };

    // Nikon3 Shot Info D300 (a) Tag Info
    const TagInfo Nikon3MakerNote::tagInfoSi3_[] = {
        TagInfo(   0, "Version", N_("Version"), N_("Version"), nikonSi3IfdId, makerTags, unsignedByte, printExifVersion),
        TagInfo( 604, "ISO", N_("ISO"), N_("ISO"), nikonSi3IfdId, makerTags, unsignedByte, printIiIso),
        TagInfo( 633, "ShutterCount", N_("Shutter Count"), N_("Shutter count"), nikonSi3IfdId, makerTags, unsignedLong, printValue),
        TagInfo( 721, "AFFineTuneAdj", N_("AF Fine Tune Adj"), N_("AF fine tune adj"), nikonSi3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonAfFineTuneAdj1)),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonSi3Tag)", "(UnknownNikonSi3Tag)", N_("Unknown Nikon Shot Info D300 (a) Tag"), nikonSi3IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListSi3()
    {
        return tagInfoSi3_;
    }

    //! AfFineTuneAdj D300 (b)
    extern const TagDetails nikonAfFineTuneAdj2[] = {
        { 0x0000, N_("0")   },
        { 0x043e, N_("+13") },
        { 0x04c2, N_("-13") },
        { 0x183d, N_("+7")  },
        { 0x183e, N_("+14") },
        { 0x18c2, N_("-14") },
        { 0x18c3, N_("-7")  },
        { 0x2c3e, N_("+15") },
        { 0x2cc2, N_("-15") },
        { 0x403a, N_("+1")  },
        { 0x403b, N_("+2")  },
        { 0x403c, N_("+4")  },
        { 0x403d, N_("+8")  },
        { 0x403e, N_("+16") },
        { 0x40c2, N_("-16") },
        { 0x40c3, N_("-8")  },
        { 0x40c4, N_("-4")  },
        { 0x40c5, N_("-2")  },
        { 0x40c6, N_("-1")  },
        { 0x543e, N_("+17") },
        { 0x54c2, N_("-17") },
        { 0x683d, N_("+9")  },
        { 0x683e, N_("+18") },
        { 0x68c2, N_("-18") },
        { 0x68c3, N_("-9")  },
        { 0x7c3e, N_("+19") },
        { 0x7cc2, N_("-19") },
        { 0x903c, N_("+5")  },
        { 0x903d, N_("+10") },
        { 0x903e, N_("+20") },
        { 0x90c2, N_("-20") },
        { 0x90c3, N_("-10") },
        { 0x90c4, N_("-5")  },
        { 0xb83d, N_("+11") },
        { 0xb8c3, N_("-11") },
        { 0xe03b, N_("+3")  },
        { 0xe03c, N_("+6")  },
        { 0xe03d, N_("+12") },
        { 0xe0c3, N_("-12") },
        { 0xe0c4, N_("-6")  },
        { 0xe0c5, N_("-3")  }
    };

    // Nikon3 Shot Info D300 (b) Tag Info
    const TagInfo Nikon3MakerNote::tagInfoSi4_[] = {
        TagInfo(   0, "Version", N_("Version"), N_("Version"), nikonSi4IfdId, makerTags, unsignedByte, printExifVersion),
        TagInfo( 613, "ISO", N_("ISO"), N_("ISO"), nikonSi4IfdId, makerTags, unsignedByte, printIiIso),
        TagInfo( 644, "ShutterCount", N_("Shutter Count"), N_("Shutter count"), nikonSi4IfdId, makerTags, unsignedLong, printValue),
        TagInfo( 732, "AFFineTuneAdj", N_("AF Fine Tune Adj"), N_("AF fine tune adj"), nikonSi4IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonAfFineTuneAdj2)),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonSi4Tag)", "(UnknownNikonSi4Tag)", N_("Unknown Nikon Shot Info D300 (b) Tag"), nikonSi4IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListSi4()
    {
        return tagInfoSi4_;
    }

    //! VibrationReduction
    extern const TagDetails nikonOffOn2[] = {
        { 0, N_("Off")    },
        { 1, N_("On (1)") },
        { 2, N_("On (2)") },
        { 3, N_("On (3)") }
    };

    //! VibrationReduction2
    extern const TagDetails nikonOffOn3[] = {
        { 0x0, N_("n/a") },
        { 0xc, N_("Off") },
        { 0xf, N_("On")  }
    };

    // Nikon3 Shot Info Tag Info
    const TagInfo Nikon3MakerNote::tagInfoSi5_[] = {
        TagInfo(   0, "Version", N_("Version"), N_("Version"), nikonSi5IfdId, makerTags, unsignedByte, printExifVersion),
        TagInfo( 106, "ShutterCount1", N_("Shutter Count 1"), N_("Shutter count 1"), nikonSi5IfdId, makerTags, unsignedLong, printValue),
        TagInfo( 110, "DeletedImageCount", N_("Deleted Image Count"), N_("Deleted image count"), nikonSi5IfdId, makerTags, unsignedLong, printValue),
        TagInfo( 117, "VibrationReduction", N_("Vibration Reduction"), N_("Vibration reduction"), nikonSi5IfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonOffOn2)),
        TagInfo( 130, "VibrationReduction1", N_(""), N_(""), nikonSi5IfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonOffOn)),
        TagInfo( 343, "ShutterCount2", N_("Shutter Count 2"), N_("Shutter count 2"), nikonSi5IfdId, makerTags, undefined, printValue),
        TagInfo( 430, "VibrationReduction2", N_("Vibration Reduction 2"), N_("Vibration reduction 2"), nikonSi5IfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonOffOn3)),
        TagInfo( 598, "ISO", N_("ISO"), N_("ISO"), nikonSi5IfdId, makerTags, unsignedByte, printIiIso),
        TagInfo( 630, "ShutterCount", N_("Shutter Count"), N_("Shutter count"), nikonSi5IfdId, makerTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonSi5Tag)", "(UnknownNikonSi5Tag)", N_("Unknown Nikon Shot Info Tag"), nikonSi5IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListSi5()
    {
        return tagInfoSi5_;
    }

    // Nikon3 Color Balance 1 Tag Info
    const TagInfo Nikon3MakerNote::tagInfoCb1_[] = {
        TagInfo( 0, "Version", N_("Version"), N_("Version"), nikonCb1IfdId, makerTags, undefined, printExifVersion),
        TagInfo(36, "WB_RBGGLevels", N_("WB RBGG Levels"), N_("WB RBGG levels"), nikonCb1IfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonCb1Tag)", "(UnknownNikonCb1Tag)", N_("Unknown Nikon Color Balance 1 Tag"), nikonCb1IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListCb1()
    {
        return tagInfoCb1_;
    }

    // Nikon3 Color Balance 2 Tag Info
    const TagInfo Nikon3MakerNote::tagInfoCb2_[] = {
        TagInfo( 0, "Version", N_("Version"), N_("Version"), nikonCb2IfdId, makerTags, undefined, printExifVersion),
        TagInfo( 5, "WB_RGGBLevels", N_("WB RGGB Levels"), N_("WB RGGB levels"), nikonCb2IfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonCb2Tag)", "(UnknownNikonCb2Tag)", N_("Unknown Nikon Color Balance 2 Tag"), nikonCb2IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListCb2()
    {
        return tagInfoCb2_;
    }

    // Nikon3 Color Balance 2a Tag Info
    const TagInfo Nikon3MakerNote::tagInfoCb2a_[] = {
        TagInfo( 0, "Version", N_("Version"), N_("Version"), nikonCb2aIfdId, makerTags, undefined, printExifVersion),
        TagInfo( 9, "WB_RGGBLevels", N_("WB RGGB Levels"), N_("WB RGGB levels"), nikonCb2aIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonCb2aTag)", "(UnknownNikonCb2aTag)", N_("Unknown Nikon Color Balance 2a Tag"), nikonCb2aIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListCb2a()
    {
        return tagInfoCb2a_;
    }

    // Nikon3 Color Balance 2b Tag Info
    const TagInfo Nikon3MakerNote::tagInfoCb2b_[] = {
        TagInfo(  0, "Version", N_("Version"), N_("Version"), nikonCb2bIfdId, makerTags, undefined, printExifVersion),
        TagInfo(145, "WB_RGGBLevels", N_("WB RGGB Levels"), N_("WB RGGB levels"), nikonCb2bIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonCb2bTag)", "(UnknownNikonCb2bTag)", N_("Unknown Nikon Color Balance 2b Tag"), nikonCb2bIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListCb2b()
    {
        return tagInfoCb2b_;
    }

    // Nikon3 Color Balance 3 Tag Info
    const TagInfo Nikon3MakerNote::tagInfoCb3_[] = {
        TagInfo( 0, "Version", N_("Version"), N_("Version"), nikonCb3IfdId, makerTags, undefined, printExifVersion),
        TagInfo(10, "WB_RGBGLevels", N_("WB RGBG Levels"), N_("WB RGBG levels"), nikonCb3IfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonCb3Tag)", "(UnknownNikonCb3Tag)", N_("Unknown Nikon Color Balance 3 Tag"), nikonCb3IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListCb3()
    {
        return tagInfoCb3_;
    }

    // Nikon3 Color Balance 4 Tag Info
    const TagInfo Nikon3MakerNote::tagInfoCb4_[] = {
        TagInfo(  0, "Version", N_("Version"), N_("Version"), nikonCb4IfdId, makerTags, undefined, printExifVersion),
        TagInfo(147, "WB_GRBGLevels", N_("WB GRBG Levels"), N_("WB GRBG levels"), nikonCb4IfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonCb4Tag)", "(UnknownNikonCb4Tag)", N_("Unknown Nikon Color Balance 4 Tag"), nikonCb4IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListCb4()
    {
        return tagInfoCb4_;
    }

    // Nikon3 Lens Data 1 Tag Info
    const TagInfo Nikon3MakerNote::tagInfoLd1_[] = {
        TagInfo( 0, "Version", N_("Version"), N_("Version"), nikonLd1IfdId, makerTags, undefined, printExifVersion),
        TagInfo( 6, "LensIDNumber", N_("Lens ID Number"), N_("Lens ID number"), nikonLd1IfdId, makerTags, unsignedByte, printLensId1),
        TagInfo( 7, "LensFStops", N_("Lens F-Stops"), N_("Lens F-stops"), nikonLd1IfdId, makerTags, unsignedByte, printValue),
        TagInfo( 8, "MinFocalLength", N_("Min Focal Length"), N_("Min focal length"), nikonLd1IfdId, makerTags, unsignedByte, printValue),
        TagInfo( 9, "MaxFocalLength", N_("Max Focal Length"), N_("Max focal length"), nikonLd1IfdId, makerTags, unsignedByte, printValue),
        TagInfo(10, "MaxApertureAtMinFocal", N_("Max Aperture At Min Focal"), N_("Max aperture at min focal"), nikonLd1IfdId, makerTags, unsignedByte, printValue),
        TagInfo(11, "MaxApertureAtMaxFocal", N_("Max Aperture At Max Focal"), N_("Max aperture at max focal"), nikonLd1IfdId, makerTags, unsignedByte, printValue),
        TagInfo(12, "MCUVersion", N_("MCU Version"), N_("MCU version"), nikonLd1IfdId, makerTags, unsignedByte, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonLd1Tag)", "(UnknownNikonLd1Tag)", N_("Unknown Nikon Lens Data 1 Tag"), nikonLd1IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListLd1()
    {
        return tagInfoLd1_;
    }

    // Nikon3 Lens Data 2 Tag Info
    const TagInfo Nikon3MakerNote::tagInfoLd2_[] = {
        TagInfo( 0, "Version", N_("Version"), N_("Version"), nikonLd2IfdId, makerTags, undefined, printExifVersion),
        TagInfo( 4, "ExitPupilPosition", N_("Exit Pupil Position"), N_("Exit pupil position"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo( 5, "AFAperture", N_("AF Aperture"), N_("AF aperture"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo( 8, "FocusPosition", N_("Focus Position"), N_("Focus position"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo( 9, "FocusDistance", N_("Focus Distance"), N_("Focus distance"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo(10, "FocalLength", N_("Focal Length"), N_("Focal length"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo(11, "LensIDNumber", N_("Lens ID Number"), N_("Lens ID number"), nikonLd2IfdId, makerTags, unsignedByte, printLensId2),
        TagInfo(12, "LensFStops", N_("Lens F-Stops"), N_("Lens F-stops"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo(13, "MinFocalLength", N_("Min Focal Length"), N_("Min focal length"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo(14, "MaxFocalLength", N_("Max Focal Length"), N_("Max focal length"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo(15, "MaxApertureAtMinFocal", N_("Max Aperture At Min Focal"), N_("Max aperture at min focal"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo(16, "MaxApertureAtMaxFocal", N_("Max Aperture At Max Focal"), N_("Max aperture at max focal"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo(17, "MCUVersion", N_("MCU Version"), N_("MCU version"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        TagInfo(18, "EffectiveMaxAperture", N_("Effective Max Aperture"), N_("Effective max aperture"), nikonLd2IfdId, makerTags, unsignedByte, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonLd2Tag)", "(UnknownNikonLd2Tag)", N_("Unknown Nikon Lens Data 2 Tag"), nikonLd2IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListLd2()
    {
        return tagInfoLd2_;
    }

    // Nikon3 Lens Data 3 Tag Info
    const TagInfo Nikon3MakerNote::tagInfoLd3_[] = {
        TagInfo( 0, "Version", N_("Version"), N_("Version"), nikonLd3IfdId, makerTags, undefined, printExifVersion),
        TagInfo( 4, "ExitPupilPosition", N_("Exit Pupil Position"), N_("Exit pupil position"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo( 5, "AFAperture", N_("AF Aperture"), N_("AF aperture"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo( 8, "FocusPosition", N_("Focus Position"), N_("Focus position"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(10, "FocusDistance", N_("Focus Distance"), N_("Focus distance"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(11, "FocalLength", N_("Focal Length"), N_("Focal length"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(12, "LensIDNumber", N_("Lens ID Number"), N_("Lens ID number"), nikonLd3IfdId, makerTags, unsignedByte, printLensId3),
        TagInfo(13, "LensFStops", N_("Lens F-Stops"), N_("Lens F-stops"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(14, "MinFocalLength", N_("Min Focal Length"), N_("Min focal length"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(15, "MaxFocalLength", N_("Max Focal Length"), N_("Max focal length"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(16, "MaxApertureAtMinFocal", N_("Max Aperture At Min Focal"), N_("Max aperture at min focal length"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(17, "MaxApertureAtMaxFocal", N_("Max Aperture At Max Focal"), N_("Max aperture at max focal length"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(18, "MCUVersion", N_("MCU Version"), N_("MCU version"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(19, "EffectiveMaxAperture", N_("Effective Max Aperture"), N_("Effective max aperture"), nikonLd3IfdId, makerTags, unsignedByte, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikonLd3Tag)", "(UnknownNikonLd3Tag)", N_("Unknown Nikon Lens Data 3 Tag"), nikonLd3IfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* Nikon3MakerNote::tagListLd3()
    {
        return tagInfoLd3_;
    }

    std::ostream& Nikon3MakerNote::printIiIso(std::ostream& os,
                                              const Value& value,
                                              const ExifData*)
    {
        double v = 100 * exp((value.toLong() / 12.0 - 5) * log(2.0));
        return os << static_cast<int>(v + 0.5);
    }

    std::ostream& Nikon3MakerNote::print0x0002(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        if (value.count() > 1) {
            os << value.toLong(1);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0007(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        std::string focus = value.toString();
        if      (focus == "AF-C  ") os << _("Continuous autofocus");
        else if (focus == "AF-S  ") os << _("Single autofocus");
        else                      os << "(" << value << ")";
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0083(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        long lensType = value.toLong();

        bool valid=false;
        if (lensType & 1)
        {
            os << "MF ";
            valid=true;
        }
        if (lensType & 2)
        {
            os << "D ";
            valid=true;
        }
        if (lensType & 4)
        {
            os << "G ";
            valid=true;
        }
        if (lensType & 8)
        {
            os << "VR";
            valid=true;
        }

        if (!valid)
            os << "(" << lensType << ")";

        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0084(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        if (   value.count() != 4
            || value.toRational(0).second == 0
            || value.toRational(1).second == 0) {
            os << "(" << value << ")";
            return os;
        }
        long len1 = value.toLong(0);
        long len2 = value.toLong(1);

        Rational fno1 = value.toRational(2);
        Rational fno2 = value.toRational(3);
        os << len1;
        if (len2 != len1) {
            os << "-" << len2;
        }
        os << "mm ";
        std::ostringstream oss;
        oss.copyfmt(os);
        os << "F" << std::setprecision(2)
           << static_cast<float>(fno1.first) / fno1.second;
        if (fno2 != fno1) {
            os << "-" << std::setprecision(2)
               << static_cast<float>(fno2.first) / fno2.second;
        }
        os.copyfmt(oss);
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0085(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << _("Unknown");
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

    std::ostream& Nikon3MakerNote::print0x0086(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << _("Not used");
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

    std::ostream& Nikon3MakerNote::print0x0088(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        if (value.size() != 4) { // Size is 4 even for those who map this way...
            os << "(" << value << ")";
        }
        else {
            // Mapping by Roger Larsson
            unsigned focusmetering = value.toLong(0);
            unsigned focuspoint = value.toLong(1);
            unsigned focusused = (value.toLong(2) << 8) + value.toLong(3);
            enum {standard, wide} combination = standard;
            const unsigned focuspoints =   sizeof(nikonFocuspoints)
                                         / sizeof(nikonFocuspoints[0]);

            if (focusmetering == 0 && focuspoint == 0 && focusused == 0) {
                // Special case, in Manual focus and with Nikon compacts
                // this indicates that the field has no meaning.
                // But when acually in "Single area, Center" this can mean
                // that focus was not found (try this in AF-C mode)
                // TODO: handle the meaningful case (interacts with other fields)
                os << "N/A";
                return os;
            }

            switch (focusmetering) {
            case 0x00: os << _("Single area");          break; // D70, D200
            case 0x01: os << _("Dynamic area");         break; // D70, D200
            case 0x02: os << _("Closest subject");      break; // D70, D200
            case 0x03: os << _("Group dynamic-AF");     break; // D200
            case 0x04: os << _("Single area (wide)");   combination = wide; break; // D200
            case 0x05: os << _("Dynamic area (wide)");  combination = wide; break; // D200
            default: os << "(" << focusmetering << ")"; break;
            }

            char sep = ';';
            if (focusmetering != 0x02) { //  No user selected point for Closest subject
                os << sep << ' ';

                // What focuspoint did the user select?
                if (focuspoint < focuspoints) {
                    os << nikonFocuspoints[focuspoint];
                    // TODO: os << position[fokuspoint][combination]
                }
                else
                    os << "(" << focuspoint << ")";

                sep = ',';
            }

            // What fokuspoints(!) did the camera use? add if differs
            if (focusused == 0)
                os << sep << " " << _("none");
            else if (focusused != 1U<<focuspoint) {
                // selected point was not the actually used one
                // (Roger Larsson: my interpretation, verify)
                os << sep;
                for (unsigned fpid=0; fpid<focuspoints; fpid++)
                    if (focusused & 1<<fpid)
                        os << ' ' << nikonFocuspoints[fpid];
            }

            os << " " << _("used");
        }

        return os;
    }

    std::ostream& Nikon3MakerNote::printAfPointsInFocus(std::ostream& os,
                                                        const Value& value,
                                                        const ExifData* metadata)
    {
        if (value.typeId() != unsignedShort) return os << "(" << value << ")";

        bool dModel = false;
        if (metadata) {
            ExifData::const_iterator pos = metadata->findKey(ExifKey("Exif.Image.Model"));
            if (pos != metadata->end() && pos->count() != 0) {
                std::string model = pos->toString();
                if (model.find("NIKON D") != std::string::npos) {
                    dModel = true;
                }
            }
        }

        uint16_t val = value.toLong();
        if (dModel) val = (val >> 8) | ((val & 0x00ff) << 8);

        if (val == 0x07ff) return os << _("All 11 Points");

        UShortValue v;
        v.value_.push_back(val);
        return EXV_PRINT_TAG_BITMASK(nikonAfPointsInFocus)(os, v, 0);
    }

    std::ostream& Nikon3MakerNote::print0x0089(std::ostream& os,
                                               const Value& value,
                                               const ExifData* metadata)
    {
        if (value.count() != 1 || value.typeId() != unsignedShort) {
            return os << "(" << value << ")";
        }
        long l = value.toLong(0);
        if (l == 0) return os << _("Single-frame");
        if (!(l & 0x87)) os << _("Single-frame") << ", ";
        bool d70 = false;
        if (metadata) {
            ExifKey key("Exif.Image.Model");
            ExifData::const_iterator pos = metadata->findKey(key);
            if (pos != metadata->end() && pos->count() != 0) {
                std::string model = pos->toString();
                if (model.find("D70") != std::string::npos) {
                    d70 = true;
                }
            }
        }
        if (d70) {
            EXV_PRINT_TAG_BITMASK(nikonShootingModeD70)(os, value, 0);
        }
        else {
            EXV_PRINT_TAG_BITMASK(nikonShootingMode)(os, value, 0);
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x008b(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        // Decoded by Robert Rottmerhusen <email@rottmerhusen.com>
        if (   value.size() != 4
            || value.typeId() != undefined) {
            return os << "(" << value << ")";
        }
        float a = value.toFloat(0);
        long  b = value.toLong(1);
        long  c = value.toLong(2);
        if (c == 0) return os << "(" << value << ")";
        return os << a * b / c;
    }

    std::ostream& Nikon3MakerNote::printLensId1(std::ostream& os,
                                                const Value& value,
                                                const ExifData* metadata)
    {
        return printLensId(os, value, metadata, "NikonLd1");
    }

    std::ostream& Nikon3MakerNote::printLensId2(std::ostream& os,
                                                const Value& value,
                                                const ExifData* metadata)
    {
        return printLensId(os, value, metadata, "NikonLd2");
    }

    std::ostream& Nikon3MakerNote::printLensId3(std::ostream& os,
                                                const Value& value,
                                                const ExifData* metadata)
    {
        return printLensId(os, value, metadata, "NikonLd3");
    }

    std::ostream& Nikon3MakerNote::printLensId(std::ostream& os,
                                               const Value& value,
                                               const ExifData* metadata,
                                               const std::string& group)
    {
#ifdef EXV_HAVE_LENSDATA
//------------------------------------------------------------------------------
// List of AF F-Mount lenses - Version 4.1.361.01                    2009-11-10
//------------------------------------------------------------------------------
#define FMLVERSION "4.1.361.01"
#define FMLDATE "2009-11-10"
//------------------------------------------------------------------------------
//
//
// created by Robert Rottmerhusen 2005 - 2009
// http://www.rottmerhusen.com (lens_id@rottmerhusen.com)
//
// for contributor info and more visit my online list:
// http://www.rottmerhusen.com/objektives/lensid/thirdparty.html
//
// three misidentified lenses (ID like the a Nikkor):
//    "Sigma" "105mm F2.8 EX DG Macro";
//    "Tokina" "AT-X 235 AF PRO / AF 20-35mm f/2.8";
//    "Tokina" "AT-X 124 AF PRO DX II / 12-24mm f/4";
//
// free use in non-commercial, GPL or open source software only!
// please contact me for adding lenses or use in commercial software.
//
// product number/order code not complete
//
//"data from TAG 0x98" "ltyp" " "TC" "MID" "manuf" "PN" "lens name from manuf";
//
//------------------------------------------------------------------------------
// Nikkor lenses by their LensID
//------------------------------------------------------------------------------
//
static const struct {unsigned char lid,stps,focs,focl,aps,apl,lfw, ltype, tcinfo, mid; const char *manuf, *lnumber, *lensname;}
fmountlens[] = {
{0x01,0x58,0x50,0x50,0x14,0x14,0x02,0x00,0x00,0x00, "Nikon", "JAA00901", "AF Nikkor 50mm f/1.8"},
{0x02,0x42,0x44,0x5C,0x2A,0x34,0x02,0x00,0x00,0x00, "Nikon", "JAA72701", "AF Zoom-Nikkor 35-70mm f/3.3-4.5"},
{0x02,0x42,0x44,0x5C,0x2A,0x34,0x08,0x00,0x00,0x00, "Nikon", "JAA72701", "AF Zoom-Nikkor 35-70mm f/3.3-4.5"},
{0x03,0x48,0x5C,0x81,0x30,0x30,0x02,0x00,0x00,0x00, "Nikon", "JAA72801", "AF Zoom-Nikkor 70-210mm f/4"},
{0x04,0x48,0x3C,0x3C,0x24,0x24,0x03,0x00,0x00,0x00, "Nikon", "JAA12001", "AF Nikkor 28mm f/2.8"},
{0x05,0x54,0x50,0x50,0x0C,0x0C,0x04,0x00,0x00,0x00, "Nikon", "JAA01001", "AF Nikkor 50mm f/1.4"},
{0x06,0x54,0x53,0x53,0x24,0x24,0x06,0x00,0x00,0x00, "Nikon", "JAA62101", "AF Micro-Nikkor 55mm f/2.8"},
{0x07,0x40,0x3C,0x62,0x2C,0x34,0x03,0x00,0x00,0x00, "Nikon", "JAA72901", "AF Zoom-Nikkor 28-85mm f/3.5-4.5"},
{0x08,0x40,0x44,0x6A,0x2C,0x34,0x04,0x00,0x00,0x00, "Nikon", "JAA73001", "AF Zoom-Nikkor 35-105mm f/3.5-4.5"},
{0x09,0x48,0x37,0x37,0x24,0x24,0x04,0x00,0x00,0x00, "Nikon", "JAA12101", "AF Nikkor 24mm f/2.8"},
{0x0A,0x48,0x8E,0x8E,0x24,0x24,0x03,0x00,0x00,0x00, "Nikon", "JAA322AB", "AF Nikkor 300mm f/2.8 IF-ED"},
{0x0B,0x48,0x7C,0x7C,0x24,0x24,0x05,0x00,0x00,0x00, "Nikon", "JAA32101", "AF Nikkor 180mm f/2.8 IF-ED"},
//0C
{0x0D,0x40,0x44,0x72,0x2C,0x34,0x07,0x00,0x00,0x00, "Nikon", "JAA73101", "AF Zoom-Nikkor 35-135mm f/3.5-4.5"},
{0x0E,0x48,0x5C,0x81,0x30,0x30,0x05,0x00,0x00,0x00, "Nikon", "", "AF Zoom-Nikkor 70-210mm f/4"},
{0x0F,0x58,0x50,0x50,0x14,0x14,0x05,0x00,0x00,0x00, "Nikon", "JAA009AD", "AF Nikkor 50mm f/1.8 N"},
{0x10,0x48,0x8E,0x8E,0x30,0x30,0x08,0x00,0x00,0x00, "Nikon", "JAA32301", "AF Nikkor 300mm f/4 IF-ED"},
{0x11,0x48,0x44,0x5C,0x24,0x24,0x08,0x00,0x00,0x00, "Nikon", "JAA73301", "AF Zoom-Nikkor 35-70mm f/2.8"},
{0x12,0x48,0x5C,0x81,0x30,0x3C,0x09,0x00,0x00,0x00, "Nikon", "JAA73201", "AF Nikkor 70-210mm f/4-5.6"},
{0x13,0x42,0x37,0x50,0x2A,0x34,0x0B,0x00,0x00,0x00, "Nikon", "JAA73401", "AF Zoom-Nikkor 24-50mm f/3.3-4.5"},
{0x14,0x48,0x60,0x80,0x24,0x24,0x0B,0x00,0x00,0x00, "Nikon", "JAA73501", "AF Zoom-Nikkor 80-200mm f/2.8 ED"},
{0x15,0x4C,0x62,0x62,0x14,0x14,0x0C,0x00,0x00,0x00, "Nikon", "JAA32401", "AF Nikkor 85mm f/1.8"},
//16
{0x17,0x3C,0xA0,0xA0,0x30,0x30,0x0F,0x00,0x00,0x00, "Nikon", "JAA518AA", "Nikkor 500mm f/4 P ED IF"},
{0x17,0x3C,0xA0,0xA0,0x30,0x30,0x11,0x00,0x00,0x00, "Nikon", "JAA518AA", "Nikkor 500mm f/4 P ED IF"},
{0x18,0x40,0x44,0x72,0x2C,0x34,0x0E,0x00,0x00,0x00, "Nikon", "JAA736AA", "AF Zoom-Nikkor 35-135mm f/3.5-4.5 N"},
//19
{0x1A,0x54,0x44,0x44,0x18,0x18,0x11,0x00,0x00,0x00, "Nikon", "JAA12201", "AF Nikkor 35mm f/2"},
{0x1B,0x44,0x5E,0x8E,0x34,0x3C,0x10,0x00,0x00,0x00, "Nikon", "JAA738AA", "AF Zoom-Nikkor 75-300mm f/4.5-5.6"},
{0x1C,0x48,0x30,0x30,0x24,0x24,0x12,0x00,0x00,0x00, "Nikon", "JAA12301", "AF Nikkor 20mm f/2.8"},
{0x1D,0x42,0x44,0x5C,0x2A,0x34,0x12,0x00,0x00,0x00, "Nikon", "", "AF Zoom-Nikkor 35-70mm f/3.3-4.5 N"},
{0x1E,0x54,0x56,0x56,0x24,0x24,0x13,0x00,0x00,0x00, "Nikon", "JAA62201", "AF Micro-Nikkor 60mm f/2.8"},
{0x1F,0x54,0x6A,0x6A,0x24,0x24,0x14,0x00,0x00,0x00, "Nikon", "JAA62301", "AF Micro-Nikkor 105mm f/2.8"},
{0x20,0x48,0x60,0x80,0x24,0x24,0x15,0x00,0x00,0x00, "Nikon", "", "AF Zoom-Nikkor 80-200mm f/2.8 ED"},
{0x21,0x40,0x3C,0x5C,0x2C,0x34,0x16,0x00,0x00,0x00, "Nikon", "", "AF Zoom-Nikkor 28-70mm f/3.5-4.5"},
{0x22,0x48,0x72,0x72,0x18,0x18,0x16,0x00,0x00,0x00, "Nikon", "JAA32501", "AF DC-Nikkor 135mm f/2"},
{0x23,0x30,0xBE,0xCA,0x3C,0x48,0x17,0x00,0x00,0x00, "Nikon", "", "Zoom-Nikkor 1200-1700mm f/5.6-8 P ED IF"},
// - D - lenses from here
{0x24,0x48,0x60,0x80,0x24,0x24,0x1A,0x02,0x00,0x00, "Nikon", "JAA742DA", "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
{0x25,0x48,0x44,0x5c,0x24,0x24,0x1B,0x02,0x00,0x00, "Nikon", "JAA743DA", "AF Zoom-Nikkor 35-70mm f/2.8D"},
{0x25,0x48,0x44,0x5c,0x24,0x24,0x52,0x02,0x00,0x00, "Nikon", "JAA743DA", "AF Zoom-Nikkor 35-70mm f/2.8D"},
//26
{0x27,0x48,0x8E,0x8E,0x24,0x24,0x1D,0x02,0x08,0x00, "Nikon", "JAA326DA", "AF-I Nikkor 300mm f/2.8D IF-ED"},
{0x27,0x48,0x8E,0x8E,0x24,0x24,0xF1,0x02,0x28,0x00, "Nikon", "JAA326DA", "AF-I Nikkor 300mm f/2.8D IF-ED + TC-14E"},
{0x27,0x48,0x8E,0x8E,0x24,0x24,0xE1,0x02,0x28,0x00, "Nikon", "JAA326DA", "AF-I Nikkor 300mm f/2.8D IF-ED + TC-17E"},
{0x27,0x48,0x8E,0x8E,0x24,0x24,0xF2,0x02,0x28,0x00, "Nikon", "JAA326DA", "AF-I Nikkor 300mm f/2.8D IF-ED + TC-20E"},
{0x28,0x3C,0xA6,0xA6,0x30,0x30,0x1D,0x02,0x08,0x00, "Nikon", "JAA519DA", "AF-I Nikkor 600mm f/4D IF-ED"},
{0x28,0x3C,0xA6,0xA6,0x30,0x30,0xF1,0x02,0x28,0x00, "Nikon", "JAA519DA", "AF-I Nikkor 600mm f/4D IF-ED + TC-14E"},
{0x28,0x3C,0xA6,0xA6,0x30,0x30,0xE1,0x02,0x28,0x00, "Nikon", "JAA519DA", "AF-I Nikkor 600mm f/4D IF-ED + TC-17E"},
{0x28,0x3C,0xA6,0xA6,0x30,0x30,0xF2,0x02,0x28,0x00, "Nikon", "JAA519DA", "AF-I Nikkor 600mm f/4D IF-ED + TC-20E"},
//29
{0x2A,0x54,0x3C,0x3C,0x0C,0x0C,0x26,0x02,0x00,0x00, "Nikon", "JAA124DA", "AF Nikkor 28mm f/1.4D"},
{0x2B,0x3C,0x44,0x60,0x30,0x3C,0x1F,0x02,0x00,0x00, "Nikon", "", "AF Zoom-Nikkor 35-80mm f/4-5.6D"},
{0x2C,0x48,0x6A,0x6A,0x18,0x18,0x27,0x02,0x00,0x00, "Nikon", "JAA327DA", "AF DC-Nikkor 105mm f/2D"},
{0x2D,0x48,0x80,0x80,0x30,0x30,0x21,0x02,0x00,0x00, "Nikon", "JAA624DA", "AF Micro-Nikkor 200mm f/4D IF-ED"},
{0x2E,0x48,0x5C,0x82,0x30,0x3C,0x28,0x02,0x00,0x00, "Nikon", "JAA747DA", "AF Nikkor 70-210mm f/4-5.6D"},
{0x2F,0x48,0x30,0x44,0x24,0x24,0x29,0x02,0x00,0x00, "Nikon", "JAA746DA", "AF Zoom-Nikkor 20-35mm f/2.8D IF"},
{0x30,0x48,0x98,0x98,0x24,0x24,0x24,0x02,0x08,0x00, "Nikon", "JAA520DA", "AF-I Nikkor 400mm f/2.8D IF-ED"},
{0x30,0x48,0x98,0x98,0x24,0x24,0xF1,0x02,0x28,0x00, "Nikon", "JAA520DA", "AF-I Nikkor 400mm f/2.8D IF-ED + TC-14E"},
{0x30,0x48,0x98,0x98,0x24,0x24,0xE1,0x02,0x28,0x00, "Nikon", "JAA520DA", "AF-I Nikkor 400mm f/2.8D IF-ED + TC-17E"},
{0x30,0x48,0x98,0x98,0x24,0x24,0xF2,0x02,0x28,0x00, "Nikon", "JAA520DA", "AF-I Nikkor 400mm f/2.8D IF-ED + TC-20E"},
{0x31,0x54,0x56,0x56,0x24,0x24,0x25,0x02,0x00,0x00, "Nikon", "JAA625DA", "AF Micro-Nikkor 60mm f/2.8D"},
{0x32,0x54,0x6A,0x6A,0x24,0x24,0x35,0x02,0x00,0x00, "Nikon", "JAA627DA", "AF Micro-Nikkor 105mm f/2.8D"},
{0x33,0x48,0x2D,0x2D,0x24,0x24,0x31,0x02,0x00,0x00, "Nikon", "JAA126DA", "AF Nikkor 18mm f/2.8D"},
{0x34,0x48,0x29,0x29,0x24,0x24,0x32,0x02,0x00,0x00, "Nikon", "JAA626DA", "AF Fisheye Nikkor 16mm f/2.8D"},
{0x35,0x3C,0xA0,0xA0,0x30,0x30,0x33,0x02,0x08,0x00, "Nikon", "JAA521DA", "AF-I Nikkor 500mm f/4D IF-ED"},
{0x35,0x3C,0xA0,0xA0,0x30,0x30,0xF1,0x02,0x28,0x00, "Nikon", "JAA521DA", "AF-I Nikkor 500mm f/4D IF-ED + TC-14E"},
{0x35,0x3C,0xA0,0xA0,0x30,0x30,0xE1,0x02,0x28,0x00, "Nikon", "JAA521DA", "AF-I Nikkor 500mm f/4D IF-ED + TC-17E"},
{0x35,0x3C,0xA0,0xA0,0x30,0x30,0xF2,0x02,0x28,0x00, "Nikon", "JAA521DA", "AF-I Nikkor 500mm f/4D IF-ED + TC-20E"},
{0x36,0x48,0x37,0x37,0x24,0x24,0x34,0x02,0x00,0x00, "Nikon", "JAA125DA", "AF Nikkor 24mm f/2.8D"},
{0x37,0x48,0x30,0x30,0x24,0x24,0x36,0x02,0x00,0x00, "Nikon", "JAA127DA", "AF Nikkor 20mm f/2.8D"},
{0x38,0x4C,0x62,0x62,0x14,0x14,0x37,0x02,0x00,0x00, "Nikon", "JAA328DA", "AF Nikkor 85mm f/1.8D"},
//39                38
{0x3A,0x40,0x3C,0x5C,0x2C,0x34,0x39,0x02,0x00,0x00, "Nikon", "JAA744DA", "AF Zoom-Nikkor 28-70mm f/3.5-4.5D"},
{0x3B,0x48,0x44,0x5C,0x24,0x24,0x3A,0x02,0x00,0x00, "Nikon", "JAA743DA", "AF Zoom-Nikkor 35-70mm f/2.8D N"},
{0x3C,0x48,0x60,0x80,0x24,0x24,0x3B,0x02,0x00,0x00, "Nikon", "", "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
{0x3D,0x3C,0x44,0x60,0x30,0x3C,0x3E,0x02,0x00,0x00, "Nikon", "", "AF Zoom-Nikkor 35-80mm f/4-5.6D"},
{0x3E,0x48,0x3C,0x3C,0x24,0x24,0x3D,0x02,0x00,0x00, "Nikon", "JAA128DA", "AF Nikkor 28mm f/2.8D"},
{0x3F,0x40,0x44,0x6A,0x2C,0x34,0x45,0x02,0x00,0x00, "Nikon", "JAA748DA", "AF Zoom-Nikkor 35-105mm f/3.5-4.5D"},
//40
{0x41,0x48,0x7c,0x7c,0x24,0x24,0x43,0x02,0x00,0x00, "Nikon", "JAA330DA", "AF Nikkor 180mm f/2.8D IF-ED"},
{0x42,0x54,0x44,0x44,0x18,0x18,0x44,0x02,0x00,0x00, "Nikon", "JAA129DA", "AF Nikkor 35mm f/2D"},
{0x43,0x54,0x50,0x50,0x0C,0x0C,0x46,0x02,0x00,0x00, "Nikon", "JAA011DB", "AF Nikkor 50mm f/1.4D"},
{0x44,0x44,0x60,0x80,0x34,0x3C,0x47,0x02,0x00,0x00, "Nikon", "JAA753DB", "AF Zoom-Nikkor 80-200mm f/4.5-5.6D"},
{0x45,0x40,0x3C,0x60,0x2C,0x3C,0x48,0x02,0x00,0x00, "Nikon", "JAA752DA", "AF Zoom-Nikkor 28-80mm f/3.5-5.6D"},
{0x46,0x3C,0x44,0x60,0x30,0x3C,0x49,0x02,0x00,0x00, "Nikon", "JAA754DA", "AF Zoom-Nikkor 35-80mm f/4-5.6D N"},
{0x47,0x42,0x37,0x50,0x2A,0x34,0x4A,0x02,0x00,0x00, "Nikon", "JAA756DA", "AF Zoom-Nikkor 24-50mm f/3.3-4.5D"},
{0x48,0x48,0x8E,0x8E,0x24,0x24,0x4B,0x02,0x08,0x00, "Nikon", "JAA333DA", "AF-S Nikkor 300mm f/2.8D IF-ED"},
{0x48,0x48,0x8E,0x8E,0x24,0x24,0xF1,0x02,0x28,0x00, "Nikon", "JAA333DA", "AF-S Nikkor 300mm f/2.8D IF-ED + TC-14E"},
{0x48,0x48,0x8E,0x8E,0x24,0x24,0xE1,0x02,0x28,0x00, "Nikon", "JAA333DA", "AF-S Nikkor 300mm f/2.8D IF-ED + TC-17E"},
{0x48,0x48,0x8E,0x8E,0x24,0x24,0xF2,0x02,0x28,0x00, "Nikon", "JAA333DA", "AF-S Nikkor 300mm f/2.8D IF-ED + TC-20E"},
{0x49,0x3C,0xA6,0xA6,0x30,0x30,0x4C,0x02,0x08,0x00, "Nikon", "JAA522DA", "AF-S Nikkor 600mm f/4D IF-ED"},
{0x49,0x3C,0xA6,0xA6,0x30,0x30,0xF1,0x02,0x28,0x00, "Nikon", "JAA522DA", "AF-S Nikkor 600mm f/4D IF-ED + TC-14E"},
{0x49,0x3C,0xA6,0xA6,0x30,0x30,0xE1,0x02,0x28,0x00, "Nikon", "JAA522DA", "AF-S Nikkor 600mm f/4D IF-ED + TC-17E"},
{0x49,0x3C,0xA6,0xA6,0x30,0x30,0xF2,0x02,0x28,0x00, "Nikon", "JAA522DA", "AF-S Nikkor 600mm f/4D IF-ED + TC-20E"},
{0x4A,0x54,0x62,0x62,0x0C,0x0C,0x4D,0x02,0x00,0x00, "Nikon", "JAA332DA", "AF Nikkor 85mm f/1.4D IF"},
{0x4B,0x3C,0xA0,0xA0,0x30,0x30,0x4E,0x02,0x08,0x00, "Nikon", "JAA523DA", "AF-S Nikkor 500mm f/4D IF-ED"},
{0x4B,0x3C,0xA0,0xA0,0x30,0x30,0xF1,0x02,0x28,0x00, "Nikon", "JAA523DA", "AF-S Nikkor 500mm f/4D IF-ED + TC-14E"},
{0x4B,0x3C,0xA0,0xA0,0x30,0x30,0xE1,0x02,0x28,0x00, "Nikon", "JAA523DA", "AF-S Nikkor 500mm f/4D IF-ED + TC-17E"},
{0x4B,0x3C,0xA0,0xA0,0x30,0x30,0xF2,0x02,0x28,0x00, "Nikon", "JAA523DA", "AF-S Nikkor 500mm f/4D IF-ED + TC-20E"},
{0x4C,0x40,0x37,0x6E,0x2C,0x3C,0x4F,0x02,0x00,0x00, "Nikon", "JAA757DA", "AF Zoom-Nikkor 24-120mm f/3.5-5.6D IF"},
{0x4D,0x40,0x3C,0x80,0x2C,0x3C,0x62,0x02,0x00,0x00, "Nikon", "JAA758DA", "AF Zoom-Nikkor 28-200mm f/3.5-5.6D IF"},
{0x4E,0x48,0x72,0x72,0x18,0x18,0x51,0x02,0x00,0x00, "Nikon", "JAA329DA", "AF DC-Nikkor 135mm f/2D"},
{0x4F,0x40,0x37,0x5C,0x2C,0x3C,0x53,0x06,0x00,0x00, "Nikon", "JBA701AA", "IX-Nikkor 24-70mm f/3.5-5.6"},
{0x50,0x48,0x56,0x7C,0x30,0x3C,0x54,0x06,0x00,0x00, "Nikon", "JBA702AA", "IX-Nikkor 60-180mm f/4-5.6"},
//                                             "JBA703AC" "IX-Nikkor 20-60 mm f/3.5-5.6";
//51
//52
{0x53,0x48,0x60,0x80,0x24,0x24,0x57,0x02,0x00,0x00, "Nikon", "JAA762DA", "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
{0x53,0x48,0x60,0x80,0x24,0x24,0x60,0x02,0x00,0x00, "Nikon", "JAA762DA", "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
{0x54,0x44,0x5C,0x7C,0x34,0x3C,0x58,0x02,0x00,0x00, "Nikon", "JAA763DA", "AF Zoom-Micro Nikkor 70-180mm f/4.5-5.6D ED"},
//55
{0x56,0x48,0x5C,0x8E,0x30,0x3C,0x5A,0x02,0x00,0x00, "Nikon", "JAA764DA", "AF Zoom-Nikkor 70-300mm f/4-5.6D ED"},
//57
//58
{0x59,0x48,0x98,0x98,0x24,0x24,0x5D,0x02,0x08,0x00, "Nikon", "JAA524DA", "AF-S Nikkor 400mm f/2.8D IF-ED"},
{0x59,0x48,0x98,0x98,0x24,0x24,0xF1,0x02,0x08,0x00, "Nikon", "JAA524DA", "AF-S Nikkor 400mm f/2.8D IF-ED + TC-14E"},
{0x59,0x48,0x98,0x98,0x24,0x24,0xE1,0x02,0x08,0x00, "Nikon", "JAA524DA", "AF-S Nikkor 400mm f/2.8D IF-ED + TC-17E"},
{0x59,0x48,0x98,0x98,0x24,0x24,0xF2,0x02,0x08,0x00, "Nikon", "JAA524DA", "AF-S Nikkor 400mm f/2.8D IF-ED + TC-20E"},
{0x5A,0x3C,0x3E,0x56,0x30,0x3C,0x5E,0x06,0x00,0x00, "Nikon", "JBA704AA", "IX-Nikkor 30-60mm f/4-5.6"},
{0x5B,0x44,0x56,0x7C,0x34,0x3C,0x5F,0x06,0x00,0x00, "Nikon", "JBA705AA", "IX-Nikkor 60-180mm f/4.5-5.6"},
//                                             "JBA706AC" "IX-Nikkor 20-60 mm f/3.5-5.6N";
//5C
{0x5D,0x48,0x3C,0x5C,0x24,0x24,0x63,0x02,0x05,0x00, "Nikon", "JAA767DA", "AF-S Zoom-Nikkor 28-70mm f/2.8D IF-ED"},
{0x5E,0x48,0x60,0x80,0x24,0x24,0x64,0x02,0x05,0x00, "Nikon", "JAA765DA", "AF-S Zoom-Nikkor 80-200mm f/2.8D IF-ED"},
{0x5F,0x40,0x3C,0x6A,0x2C,0x34,0x65,0x02,0x00,0x00, "Nikon", "JAA766DA", "AF Zoom-Nikkor 28-105mm f/3.5-4.5D IF"},
{0x60,0x40,0x3C,0x60,0x2C,0x3C,0x66,0x02,0x00,0x00, "Nikon", "JAA769DA", "AF Zoom-Nikkor 28-80mm f/3.5-5.6D"},
{0x61,0x44,0x5E,0x86,0x34,0x3C,0x67,0x02,0x00,0x00, "Nikon", "", "AF Zoom-Nikkor 75-240mm f/4.5-5.6D"},
//62                69
{0x63,0x48,0x2B,0x44,0x24,0x24,0x68,0x02,0x05,0x00, "Nikon", "JAA770DA", "AF-S Nikkor 17-35mm f/2.8D IF-ED"},
{0x64,0x00,0x62,0x62,0x24,0x24,0x6A,0x02,0x00,0x00, "Nikon", "JAA628DA", "PC Micro-Nikkor 85mm f/2.8D"},
{0x65,0x44,0x60,0x98,0x34,0x3C,0x6B,0x0A,0x00,0x00, "Nikon", "JAA771DA", "AF VR Zoom-Nikkor 80-400mm f/4.5-5.6D ED"},
{0x66,0x40,0x2D,0x44,0x2C,0x34,0x6C,0x02,0x00,0x00, "Nikon", "JAA772DA", "AF Zoom-Nikkor 18-35mm f/3.5-4.5D IF-ED"},
{0x67,0x48,0x37,0x62,0x24,0x30,0x6D,0x02,0x00,0x00, "Nikon", "JAA774DA", "AF Zoom-Nikkor 24-85mm f/2.8-4D IF"},
{0x68,0x42,0x3C,0x60,0x2A,0x3C,0x6E,0x06,0x00,0x00, "Nikon", "JAA777DA", "AF Zoom-Nikkor 28-80mm f/3.3-5.6G"},
{0x69,0x48,0x5C,0x8E,0x30,0x3C,0x6F,0x06,0x00,0x00, "Nikon", "JAA776DA", "AF Zoom-Nikkor 70-300mm f/4-5.6G"},
{0x6A,0x48,0x8E,0x8E,0x30,0x30,0x70,0x02,0x00,0x00, "Nikon", "JAA334DA", "AF-S Nikkor 300mm f/4D IF-ED"},
{0x6B,0x48,0x24,0x24,0x24,0x24,0x71,0x02,0x00,0x00, "Nikon", "JAA130DA", "AF Nikkor ED 14mm f/2.8D"},
//6C                72
{0x6D,0x48,0x8E,0x8E,0x24,0x24,0x73,0x02,0x04,0x00, "Nikon", "JAA335DA", "AF-S Nikkor 300mm f/2.8D IF-ED II"},
{0x6E,0x48,0x98,0x98,0x24,0x24,0x74,0x02,0x04,0x00, "Nikon", "JAA525DA", "AF-S Nikkor 400mm f/2.8D IF-ED II"},
{0x6F,0x3C,0xA0,0xA0,0x30,0x30,0x75,0x02,0x04,0x00, "Nikon", "JAA526DA", "AF-S Nikkor 500mm f/4D IF-ED II"},
{0x70,0x3C,0xA6,0xA6,0x30,0x30,0x76,0x02,0x04,0x00, "Nikon", "JAA527DA", "AF-S Nikkor 600mm f/4D IF-ED II"},
//71
{0x72,0x48,0x4C,0x4C,0x24,0x24,0x77,0x00,0x00,0x00, "Nikon", "JAA012AA", "Nikkor 45mm f/2.8 P"},
//73
{0x74,0x40,0x37,0x62,0x2C,0x34,0x78,0x06,0x05,0x00, "Nikon", "JAA780DA", "AF-S Zoom-Nikkor 24-85mm f/3.5-4.5G IF-ED"},
{0x75,0x40,0x3C,0x68,0x2C,0x3C,0x79,0x06,0x00,0x00, "Nikon", "JAA778DA", "AF Zoom-Nikkor 28-100mm f/3.5-5.6G"},
{0x76,0x58,0x50,0x50,0x14,0x14,0x7A,0x02,0x00,0x00, "Nikon", "JAA013DA", "AF Nikkor 50mm f/1.8D"},
{0x77,0x48,0x5C,0x80,0x24,0x24,0x7B,0x0E,0x04,0x00, "Nikon", "JAA781DA", "AF-S VR Zoom-Nikkor 70-200mm f/2.8G IF-ED"},
{0x78,0x40,0x37,0x6E,0x2C,0x3C,0x7C,0x0E,0x05,0x00, "Nikon", "JAA782DA", "AF-S VR Zoom-Nikkor 24-120mm f/3.5-5.6G IF-ED"},
{0x79,0x40,0x3C,0x80,0x2C,0x3C,0x7F,0x06,0x00,0x00, "Nikon", "JAA783DA", "AF Zoom-Nikkor 28-200mm f/3.5-5.6G IF-ED"},
{0x7A,0x3C,0x1F,0x37,0x30,0x30,0x7E,0x06,0x05,0x00, "Nikon", "JAA784DA", "AF-S DX Zoom-Nikkor 12-24mm f/4G IF-ED"},
{0x7B,0x48,0x80,0x98,0x30,0x30,0x80,0x0E,0x04,0x00, "Nikon", "JAA787DA", "AF-S VR Zoom-Nikkor 200-400mm f/4G IF-ED"},
//7C                81
{0x7D,0x48,0x2B,0x53,0x24,0x24,0x82,0x06,0x01,0x00, "Nikon", "JAA788DA", "AF-S DX Zoom-Nikkor 17-55mm f/2.8G IF-ED"},
//7E                83
{0x7F,0x40,0x2D,0x5C,0x2C,0x34,0x84,0x06,0x01,0x00, "Nikon", "JAA790DA", "AF-S DX Zoom-Nikkor 18-70mm f/3.5-4.5G IF-ED"},
{0x80,0x48,0x1A,0x1A,0x24,0x24,0x85,0x06,0x00,0x00, "Nikon", "JAA629DA", "AF DX Fisheye-Nikkor 10.5mm f/2.8G ED"},
{0x81,0x54,0x80,0x80,0x18,0x18,0x86,0x0E,0x03,0x00, "Nikon", "JAA336DA", "AF-S VR Nikkor 200mm f/2G IF-ED"},
{0x82,0x48,0x8E,0x8E,0x24,0x24,0x87,0x0E,0x03,0x00, "Nikon", "JAA337DA", "AF-S VR Nikkor 300mm f/2.8G IF-ED"},
//83                
//84                
//85                 
//86
//87
//88                
{0x89,0x3C,0x53,0x80,0x30,0x3C,0x8B,0x06,0x00,0x00, "Nikon", "JAA793DA", "AF-S DX Zoom-Nikkor 55-200mm f/4-5.6G ED"},
{0x8A,0x54,0x6A,0x6A,0x24,0x24,0x8C,0x0E,0x03,0x00, "Nikon", "JAA630DA", "AF-S VR Micro-Nikkor 105mm f/2.8G IF-ED"},
{0x8B,0x40,0x2D,0x80,0x2C,0x3C,0x8D,0x0E,0x00,0x00, "Nikon", "JAA794DA", "AF-S DX VR Zoom-Nikkor 18-200mm f/3.5-5.6G IF-ED"},
{0x8B,0x40,0x2D,0x80,0x2C,0x3C,0xFD,0x0E,0x00,0x00, "Nikon", "JAA813DA", "AF-S DX VR Zoom-Nikkor 18-200mm f/3.5-5.6G IF-ED (II)"},
{0x8C,0x40,0x2D,0x53,0x2C,0x3C,0x8E,0x06,0x00,0x00, "Nikon", "JAA792DA", "AF-S DX Zoom-Nikkor 18-55mm f/3.5-5.6G ED"},
{0x8D,0x44,0x5C,0x8E,0x34,0x3C,0x8F,0x0E,0x00,0x00, "Nikon", "JAA795DA", "AF-S VR Zoom-Nikkor 70-300mm f/4.5-5.6G IF-ED"},
//8E                90
{0x8F,0x40,0x2D,0x72,0x2C,0x3C,0x91,0x06,0x00,0x00, "Nikon", "JAA796DA", "AF-S DX Zoom-Nikkor 18-135mm f/3.5-5.6G IF-ED"},
{0x90,0x3B,0x53,0x80,0x30,0x3C,0x92,0x0E,0x00,0x00, "Nikon", "JAA798DA", "AF-S DX VR Zoom-Nikkor 55-200mm f/4-5.6G IF-ED"},
//91                93
{0x92,0x48,0x24,0x37,0x24,0x24,0x94,0x06,0x01,0x00, "Nikon", "JAA801DA", "AF-S Zoom-Nikkor 14-24mm f/2.8G ED"},
{0x93,0x48,0x37,0x5C,0x24,0x24,0x95,0x06,0x01,0x00, "Nikon", "JAA802DA", "AF-S Zoom-Nikkor 24-70mm f/2.8G ED"},
{0x94,0x40,0x2D,0x53,0x2C,0x3C,0x96,0x06,0x00,0x00, "Nikon", "JAA797DA", "AF-S DX Zoom-Nikkor 18-55mm f/3.5-5.6G ED II"},
{0x95,0x4C,0x37,0x37,0x2C,0x2C,0x97,0x02,0x00,0x00, "Nikon", "JAA631DA", "PC-E Nikkor 24mm f/3.5D ED"},
{0x95,0x00,0x37,0x37,0x2C,0x2C,0x97,0x06,0x00,0x00, "Nikon", "JAA631DA", "PC-E Nikkor 24mm f/3.5D ED"},
{0x96,0x48,0x98,0x98,0x24,0x24,0x98,0x0E,0x02,0x00, "Nikon", "JAA528DA", "AF-S VR Nikkor 400mm f/2.8G ED"},
{0x97,0x3C,0xA0,0xA0,0x30,0x30,0x99,0x0E,0x02,0x00, "Nikon", "JAA529DA", "AF-S VR Nikkor 500mm f/4G ED"},
{0x98,0x3C,0xA6,0xA6,0x30,0x30,0x9A,0x0E,0x02,0x00, "Nikon", "JAA530DA", "AF-S VR Nikkor 600mm f/4G ED"},
{0x99,0x40,0x29,0x62,0x2C,0x3C,0x9B,0x0E,0x00,0x00, "Nikon", "JAA800DA", "AF-S DX VR Zoom-Nikkor 16-85mm f/3.5-5.6G ED"},
{0x9A,0x40,0x2D,0x53,0x2C,0x3C,0x9C,0x0E,0x00,0x00, "Nikon", "JAA803DA", "AF-S DX VR Zoom-Nikkor 18-55mm f/3.5-5.6G"},
{0x9B,0x54,0x4C,0x4C,0x24,0x24,0x9D,0x02,0x00,0x00, "Nikon", "JAA633DA", "PC-E Micro Nikkor 45mm f/2.8D ED"},
{0x9B,0x00,0x4C,0x4C,0x24,0x24,0x9D,0x06,0x00,0x00, "Nikon", "JAA633DA", "PC-E Micro Nikkor 45mm f/2.8D ED"},
{0x9C,0x54,0x56,0x56,0x24,0x24,0x9E,0x06,0x00,0x00, "Nikon", "JAA632DA", "AF-S Micro Nikkor 60mm f/2.8G ED"},
{0x9D,0x54,0x62,0x62,0x24,0x24,0x9F,0x02,0x00,0x00, "Nikon", "JAA634DA", "PC-E Micro Nikkor 85mm f/2.8D"},
{0x9D,0x00,0x62,0x62,0x24,0x24,0x9F,0x06,0x00,0x00, "Nikon", "JAA634DA", "PC-E Micro Nikkor 85mm f/2.8D"},
{0x9E,0x40,0x2D,0x6A,0x2C,0x3C,0xA0,0x0E,0x00,0x00, "Nikon", "JAA805DA", "AF-S DX VR Zoom-Nikkor 18-105mm f/3.5-5.6G ED"},
{0x9F,0x58,0x44,0x44,0x14,0x14,0xA1,0x06,0x00,0x00, "Nikon", "JAA132DA", "AF-S DX Nikkor 35mm f/1.8G"},
{0xA0,0x54,0x50,0x50,0x0C,0x0C,0xA2,0x06,0x00,0x00, "Nikon", "JAA014DA", "AF-S Nikkor 50mm f/1.4G"},
{0xA1,0x40,0x18,0x37,0x2C,0x34,0xA3,0x06,0x00,0x00, "Nikon", "JAA804DA", "AF-S DX Nikkor 10-24mm f/3.5-4.5G ED"},
{0xA2,0x48,0x5C,0x80,0x24,0x24,0xA4,0x0E,0x00,0x00, "Nikon", "JAA807DA", "AF-S Nikkor 70-200mm f/2.8G ED VR II"},
//
//
{0x01,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00, "Nikon", "JAA90701", "TC-16A"},
{0x01,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00, "Nikon", "JAA90701", "TC-16A"},
{0x00,0x00,0x00,0x00,0x00,0x00,0xF1,0x0C,0x20,0x00, "Nikon", "JAA908DA", "TC-14E"},
{0x00,0x00,0x00,0x00,0x00,0x00,0xF2,0x18,0x20,0x00, "Nikon", "JAA909DA", "TC-20E"},
{0x00,0x00,0x00,0x00,0x00,0x00,0xF1,0x0C,0x20,0x00, "Nikon", "JAA910DA", "TC-14E II"},
{0x00,0x00,0x00,0x00,0x00,0x00,0xF2,0x18,0x20,0x00, "Nikon", "JAA911DA", "TC-20E II"},
{0x00,0x00,0x00,0x00,0x00,0x00,0xE1,0x12,0x20,0x00, "Nikon", "JAA912DA", "TC-17E II"},
//
//------------------------------------------------------------------------------
// Sigma lenses by focal length, first fixed then zoom lenses
//------------------------------------------------------------------------------
//
{0xFE,0x47,0x00,0x00,0x24,0x24,0x4B,0x06,0x00,0x05, "Sigma", "486556", "4.5mm F2.8 EX DC HSM Circular Fisheye"},
{0x26,0x48,0x11,0x11,0x30,0x30,0x1C,0x02,0x00,0x05, "Sigma", "483", "8mm F4 EX Circular Fisheye"},
{0x79,0x40,0x11,0x11,0x2C,0x2C,0x1C,0x06,0x00,0x05, "Sigma", "485597", "8mm F3.5 EX Circular Fisheye"},
{0xDC,0x48,0x19,0x19,0x24,0x24,0x4B,0x06,0x00,0x05, "Sigma", "477554", "10mm F2.8 EX DC HSM Fisheye"},
{0x02,0x3F,0x24,0x24,0x2C,0x2C,0x02,0x00,0x00,0x05, "Sigma", "468", "14mm F3.5"},
{0x48,0x48,0x24,0x24,0x24,0x24,0x4B,0x02,0x00,0x05, "Sigma", "", "14mm F2.8 EX Aspherical HSM"},
{0x26,0x48,0x27,0x27,0x24,0x24,0x1C,0x02,0x00,0x05, "Sigma", "476441", "15mm F2.8 EX Diagonal Fisheye"},
//                                     "Sigma" "410"    "18mm F3.5";
{0x26,0x58,0x31,0x31,0x14,0x14,0x1C,0x02,0x00,0x05, "Sigma", "411442", "20mm F1.8 EX DG Aspherical RF"},
{0x26,0x58,0x37,0x37,0x14,0x14,0x1C,0x02,0x00,0x05, "Sigma", "432447", "24mm F1.8 EX DG Aspherical Macro"},
{0xE1,0x58,0x37,0x37,0x14,0x14,0x1C,0x02,0x00,0x05, "Sigma", "432447", "24mm F1.8 EX DG Aspherical Macro"},
{0x02,0x46,0x37,0x37,0x25,0x25,0x02,0x00,0x00,0x05, "Sigma", "438", "24mm F2.8 Super Wide II Macro"},
{0x26,0x58,0x3C,0x3C,0x14,0x14,0x1C,0x02,0x00,0x05, "Sigma", "440442", "28mm F1.8 EX DG Aspherical Macro"},
{0x48,0x54,0x3E,0x3E,0x0C,0x0C,0x4B,0x06,0x00,0x05, "Sigma", "477554", "30mm F1.4 EX DC HSM"},
{0xF8,0x54,0x3E,0x3E,0x0C,0x0C,0x4B,0x06,0x00,0x05, "Sigma", "477554", "30mm F1.4 EX DC HSM"},
{0xDE,0x54,0x50,0x50,0x0C,0x0C,0x4B,0x06,0x00,0x05, "Sigma", "310554", "50mm F1.4 EX DG HSM"},
{0x32,0x54,0x50,0x50,0x24,0x24,0x35,0x02,0x00,0x05, "Sigma", "346447", "Macro 50mm F2.8 EX DG"},
{0x79,0x48,0x5C,0x5C,0x24,0x24,0x1C,0x06,0x00,0x05, "Sigma", "270599", "Macro 70mm F2.8 EX DG"},
{0x02,0x48,0x65,0x65,0x24,0x24,0x02,0x00,0x00,0x05, "Sigma", "", "90mm F2.8 Macro"},
{0x32,0x54,0x6A,0x6A,0x24,0x24,0x35,0x02,0x00,0x05, "Sigma", "256", "Macro 105mm F2.8 EX DG"},
{0xE5,0x54,0x6A,0x6A,0x24,0x24,0x35,0x02,0x00,0x05, "Sigma", "257446", "Macro 105mm F2.8 EX DG"},
{0x48,0x48,0x76,0x76,0x24,0x24,0x4B,0x06,0x04,0x05, "Sigma", "104559", "APO Macro 150mm F2.8 EX DG HSM"},
{0xF5,0x48,0x76,0x76,0x24,0x24,0x4B,0x06,0x04,0x05, "Sigma", "104559", "APO Macro 150mm F2.8 EX DG HSM"},
{0x48,0x4C,0x7C,0x7C,0x2C,0x2C,0x4B,0x02,0x00,0x05, "Sigma", "", "180mm F3.5 EX DG Macro"},
{0x48,0x4C,0x7D,0x7D,0x2C,0x2C,0x4B,0x02,0x00,0x05, "Sigma", "105556", "APO Macro 180mm F3.5 EX DG HSM"},
{0x48,0x54,0x8E,0x8E,0x24,0x24,0x4B,0x02,0x00,0x05, "Sigma", "", "APO 300mm F2.8 EX DG HSM"},
{0xFB,0x54,0x8E,0x8E,0x24,0x24,0x4B,0x02,0x00,0x05, "Sigma", "195557", "APO 300mm F2.8 EX DG HSM"},
{0x26,0x48,0x8E,0x8E,0x30,0x30,0x1C,0x02,0x00,0x05, "Sigma", "", "APO Tele Macro 300mm F4"},
{0x02,0x2F,0x98,0x98,0x3D,0x3D,0x02,0x00,0x00,0x05, "Sigma", "", "400mm F5.6 APO"},
{0x02,0x37,0xA0,0xA0,0x34,0x34,0x02,0x00,0x00,0x05, "Sigma", "", "APO 500mm F4.5"},
{0x48,0x44,0xA0,0xA0,0x34,0x34,0x4B,0x02,0x00,0x05, "Sigma", "", "APO 500mm F4.5 EX HSM"},
//                                     "Sigma" ""       "500mm F7.2 APO";
//                                     "Sigma" ""       "800mm F5.6 APO";
{0x48,0x3C,0xB0,0xB0,0x3C,0x3C,0x4B,0x02,0x00,0x05, "Sigma", "", "APO 800mm F5.6 EX HSM"},
//                                     "Sigma" "152550" "APO 800mm F5.6 EX DG HSM";
//
{0xA1,0x41,0x19,0x31,0x2C,0x2C,0x4B,0x06,0x00,0x05, "Sigma", "", "10-20mm F3.5 EX DC HSM"},
{0x48,0x3C,0x19,0x31,0x30,0x3C,0x4B,0x06,0x00,0x05, "Sigma", "201555", "10-20mm F4-5.6 EX DC HSM"},
{0xF9,0x3C,0x19,0x31,0x30,0x3C,0x4B,0x06,0x00,0x05, "Sigma", "201555", "10-20mm F4-5.6 EX DC HSM"},
{0x48,0x38,0x1F,0x37,0x34,0x3C,0x4B,0x06,0x00,0x05, "Sigma", "200558", "12-24mm F4.5-5.6 EX DG Aspherical HSM"},
{0xF0,0x38,0x1F,0x37,0x34,0x3C,0x4B,0x06,0x00,0x05, "Sigma", "200558", "12-24mm F4.5-5.6 EX DG Aspherical HSM"},
{0x26,0x40,0x27,0x3F,0x2C,0x34,0x1C,0x02,0x00,0x05, "Sigma", "", "15-30mm F3.5-4.5 EX DG Aspherical DF"},
{0x48,0x48,0x2B,0x44,0x24,0x30,0x4B,0x06,0x00,0x05, "Sigma", "", "17-35mm F2.8-4 EX DG  Aspherical HSM"},
{0x26,0x54,0x2B,0x44,0x24,0x30,0x1C,0x02,0x00,0x05, "Sigma", "", "17-35mm F2.8-4 EX Aspherical"},
{0x7A,0x47,0x2B,0x5C,0x24,0x34,0x4B,0x06,0x00,0x05, "Sigma", "689599", "17-70mm F2.8-4.5 DC Macro Asp. IF HSM"},
{0x7A,0x48,0x2B,0x5C,0x24,0x34,0x4B,0x06,0x00,0x05, "Sigma", "689599", "17-70mm F2.8-4.5 DC Macro Asp. IF HSM"},
{0x7F,0x48,0x2B,0x5C,0x24,0x34,0x1C,0x06,0x00,0x05, "Sigma", "", "17-70mm F2.8-4.5 DC Macro Asp. IF"},
{0x26,0x40,0x2D,0x44,0x2B,0x34,0x1C,0x02,0x00,0x05, "Sigma", "", "18-35 F3.5-4.5 Aspherical"},
{0x26,0x48,0x2D,0x50,0x24,0x24,0x1C,0x06,0x00,0x05, "Sigma", "", "18-50mm F2.8 EX DC"},
{0x7F,0x48,0x2D,0x50,0x24,0x24,0x1C,0x06,0x00,0x05, "Sigma", "", "18-50mm F2.8 EX DC Macro"},
{0x7A,0x48,0x2D,0x50,0x24,0x24,0x4B,0x06,0x00,0x05, "Sigma", "582593", "18-50mm F2.8 EX DC Macro"},
{0x26,0x40,0x2D,0x50,0x2C,0x3C,0x1C,0x06,0x00,0x05, "Sigma", "", "18-50mm F3.5-5.6 DC"},
{0x7A,0x40,0x2D,0x50,0x2C,0x3C,0x4B,0x06,0x00,0x05, "Sigma", "551551", "18-50mm F3.5-5.6 DC HSM"},
{0x26,0x40,0x2D,0x70,0x2B,0x3C,0x1C,0x06,0x00,0x05, "Sigma", "", "18-125mm F3.5-5.6 DC"},
{0xCD,0x3D,0x2D,0x70,0x2E,0x3C,0x4B,0x0E,0x00,0x05, "Sigma", "853556", "18-125mm F3.8-5.6 DC OS HSM"},
{0x26,0x40,0x2D,0x80,0x2C,0x40,0x1C,0x06,0x00,0x05, "Sigma", "777555", "18-200mm F3.5-6.3 DC"},
{0xED,0x40,0x2D,0x80,0x2C,0x40,0x4B,0x0E,0x00,0x05, "Sigma", "888558", "18-200mm F3.5-6.3 DC OS HSM"},
{0xA5,0x40,0x2D,0x88,0x2C,0x40,0x4B,0x0E,0x00,0x05, "Sigma", "", "18-250mm F3.5-6.3 DC OS HSM"},
{0x26,0x48,0x31,0x49,0x24,0x24,0x1C,0x02,0x00,0x05, "Sigma", "", "20-40mm F2.8"},
{0x26,0x48,0x37,0x56,0x24,0x24,0x1C,0x02,0x00,0x05, "Sigma", "547448", "24-60mm F2.8 EX DG"},
{0xB6,0x48,0x37,0x56,0x24,0x24,0x1C,0x02,0x00,0x05, "Sigma", "547448", "24-60mm F2.8 EX DG"},
{0xA6,0x48,0x37,0x5C,0x24,0x24,0x4B,0x06,0x00,0x05, "Sigma", "", "24-70mm F2.8 IF EX DG HSM"},
{0x26,0x54,0x37,0x5C,0x24,0x24,0x1C,0x02,0x00,0x05, "Sigma", "", "24-70mm F2.8 EX DG Macro"},
{0x67,0x54,0x37,0x5C,0x24,0x24,0x1C,0x02,0x00,0x05, "Sigma", "548445", "24-70mm F2.8 EX DG Macro"},
{0xE9,0x54,0x37,0x5C,0x24,0x24,0x1C,0x02,0x00,0x05, "Sigma", "548445", "24-70mm F2.8 EX DG Macro"},
{0x26,0x40,0x37,0x5C,0x2C,0x3C,0x1C,0x02,0x00,0x05, "Sigma", "", "24-70mm F3.5-5.6 Aspherical HF"},
{0x26,0x54,0x37,0x73,0x24,0x34,0x1C,0x02,0x00,0x05, "Sigma", "", "24-135mm F2.8-4.5"},
{0x02,0x46,0x3C,0x5C,0x25,0x25,0x02,0x00,0x00,0x05, "Sigma", "", "28-70mm F2.8"},
{0x26,0x54,0x3C,0x5C,0x24,0x24,0x1C,0x02,0x00,0x05, "Sigma", "", "28-70mm F2.8 EX"},
{0x26,0x48,0x3C,0x5C,0x24,0x24,0x1C,0x06,0x00,0x05, "Sigma", "549442", "28-70mm F2.8 EX DG"},
{0x26,0x48,0x3C,0x5C,0x24,0x30,0x1C,0x02,0x00,0x05, "Sigma", "634445", "28-70mm F2.8-4 DG"},
{0x02,0x3F,0x3C,0x5C,0x2D,0x35,0x02,0x00,0x00,0x05, "Sigma", "", "28-70mm F3.5-4.5 UC"},
{0x26,0x40,0x3C,0x60,0x2C,0x3C,0x1C,0x02,0x00,0x05, "Sigma", "", "28-80mm F3.5-5.6 Mini Zoom Macro II Aspherical"},
{0x26,0x40,0x3C,0x65,0x2C,0x3C,0x1C,0x02,0x00,0x05, "Sigma", "", "28-90mm F3.5-5.6 Macro"},
{0x26,0x48,0x3C,0x6A,0x24,0x30,0x1C,0x02,0x00,0x05, "Sigma", "", "28-105mm F2.8-4 Aspherical"},
{0x26,0x3E,0x3C,0x6A,0x2E,0x3C,0x1C,0x02,0x00,0x05, "Sigma", "", "28-105mm F3.8-5.6 UC-III Aspherical IF"},
{0x26,0x40,0x3C,0x80,0x2C,0x3C,0x1C,0x02,0x00,0x05, "Sigma", "", "28-200mm F3.5-5.6 Compact Aspherical Hyperzoom Macro"},
{0x26,0x40,0x3C,0x80,0x2B,0x3C,0x1C,0x02,0x00,0x05, "Sigma", "", "28-200mm F3.5-5.6 Compact Aspherical Hyperzoom Macro"},
{0x26,0x3D,0x3C,0x80,0x2F,0x3D,0x1C,0x02,0x00,0x05, "Sigma", "", "28-300mm F3.8-5.6 Aspherical"},
{0x26,0x41,0x3C,0x8E,0x2C,0x40,0x1C,0x02,0x00,0x05, "Sigma", "795443", "28-300mm F3.5-6.3 DG Macro"},
{0x26,0x40,0x3C,0x8E,0x2C,0x40,0x1C,0x02,0x00,0x05, "Sigma", "", "28-300mm F3.5-6.3 Macro"},
{0x02,0x3B,0x44,0x61,0x30,0x3D,0x02,0x00,0x00,0x05, "Sigma", "", "35-80mm F4-5.6"},
{0x02,0x40,0x44,0x73,0x2B,0x36,0x02,0x00,0x00,0x05, "Sigma", "", "35-135mm F3.5-4.5 a"},
{0x7A,0x47,0x50,0x76,0x24,0x24,0x4B,0x06,0x00,0x05, "Sigma", "", "50-150mm F2.8 EX APO DC HSM"},
{0xFD,0x47,0x50,0x76,0x24,0x24,0x4B,0x06,0x00,0x05, "Sigma", "691554", "50-150mm F2.8 EX APO DC HSM II"},
{0x48,0x3C,0x50,0xA0,0x30,0x40,0x4B,0x02,0x04,0x05, "Sigma", "736552", "APO 50-500mm F4-6.3 EX HSM"},
{0x26,0x3C,0x54,0x80,0x30,0x3C,0x1C,0x06,0x00,0x05, "Sigma", "", "55-200mm F4-5.6 DC"},
{0x7A,0x3B,0x53,0x80,0x30,0x3C,0x4B,0x06,0x00,0x05, "Sigma", "", "55-200mm F4-5.6 DC HSM"},
{0x48,0x54,0x5C,0x80,0x24,0x24,0x4B,0x02,0x00,0x05, "Sigma", "", "70-200mm F2.8 EX APO IF HSM"},
{0x7A,0x48,0x5C,0x80,0x24,0x24,0x4B,0x06,0x00,0x05, "Sigma", "", "70-200mm F2.8 EX APO DG Macro HSM II"},
{0xEE,0x48,0x5C,0x80,0x24,0x24,0x4B,0x06,0x00,0x05, "Sigma", "579555", "70-200mm F2.8 EX APO DG Macro HSM II"},
{0x02,0x46,0x5C,0x82,0x25,0x25,0x02,0x00,0x00,0x05, "Sigma", "", "70-210mm F2.8 APO"},
{0x26,0x3C,0x5C,0x82,0x30,0x3C,0x1C,0x02,0x00,0x05, "Sigma", "", "70-210mm F4-5.6 UC-II"},
{0x26,0x3C,0x5C,0x8E,0x30,0x3C,0x1C,0x02,0x00,0x05, "Sigma", "", "70-300mm F4-5.6 DG Macro"},
{0x56,0x3C,0x5C,0x8E,0x30,0x3C,0x1C,0x02,0x00,0x05, "Sigma", "", "70-300mm F4-5.6 APO Macro Super II"},
{0xE0,0x3C,0x5C,0x8E,0x30,0x3C,0x4B,0x06,0x00,0x05, "Sigma", "", "70-300mm F4-5.6 APO DG Macro HSM"},
{0x02,0x37,0x5E,0x8E,0x35,0x3D,0x02,0x00,0x00,0x05, "Sigma", "", "75-300mm F4.5-5.6 APO"},
{0x02,0x3A,0x5E,0x8E,0x32,0x3D,0x02,0x00,0x00,0x05, "Sigma", "", "75-300mm F4.0-5.6"},
{0x77,0x44,0x61,0x98,0x34,0x3C,0x7B,0x0E,0x04,0x05, "Sigma", "", "80-400mm f4.5-5.6 EX OS"},
{0x48,0x48,0x68,0x8E,0x30,0x30,0x4B,0x02,0x04,0x05, "Sigma", "", "APO 100-300mm F4 EX IF HSM"},
{0x48,0x54,0x6F,0x8E,0x24,0x24,0x4B,0x02,0x04,0x05, "Sigma", "", "APO 120-300mm F2.8 EX DG HSM"},
{0x7A,0x54,0x6E,0x8E,0x24,0x24,0x4B,0x02,0x04,0x05, "Sigma", "", "APO 120-300mm F2.8 EX DG HSM"},
{0xCF,0x38,0x6E,0x98,0x34,0x3C,0x4B,0x0E,0x00,0x05, "Sigma", "", "APO 120-400mm F4.5-5.6 DG OS HSM"},
{0x26,0x44,0x73,0x98,0x34,0x3C,0x1C,0x02,0x00,0x05, "Sigma", "", "135-400mm F4.5-5.6 APO Aspherical"},
{0xCE,0x34,0x76,0xA0,0x38,0x40,0x4B,0x0E,0x00,0x05, "Sigma", "", "APO 150-500mm F5-6.3 DG OS HSM"},
{0x26,0x40,0x7B,0xA0,0x34,0x40,0x1C,0x02,0x00,0x05, "Sigma", "", "APO 170-500mm F5-6.3 Aspherical RF"},
{0x48,0x3C,0x8E,0xB0,0x3C,0x3C,0x4B,0x02,0x00,0x05, "Sigma", "", "APO 300-800 F5.6 EX DG HSM"},
//
{0x00,0x00,0x00,0x00,0x00,0x00,0xF1,0x0C,0x10,0x05, "Sigma", "", "APO Tele Converter 1.4x EX DG"},
{0x00,0x00,0x00,0x00,0x00,0x00,0xF2,0x18,0x10,0x05, "Sigma", "", "APO Tele Converter 2x EX DG"},
//
//------------------------------------------------------------------------------
// Tamron lenses by focal length, first fixed then zoom lenses
//------------------------------------------------------------------------------
//
//                                     "Tamron" "69E"    "SP AF 14mm F/2.8 Aspherical (IF)"
{0xF4,0x54,0x56,0x56,0x18,0x18,0x84,0x06,0x00,0x02, "Tamron", "G005", "SP AF 60mm F/2 Di II LD (IF) Macro 1:1"},
{0x1E,0x5D,0x64,0x64,0x20,0x20,0x13,0x00,0x00,0x02, "Tamron", "52E", "SP AF 90mm F/2.5"},
//                                     "Tamron" "152E"   "SP AF 90mm F/2.5";
//                                     "Tamron" "72E"    "SP AF 90mm F/2.8 Macro 1:1";
{0x32,0x53,0x64,0x64,0x24,0x24,0x35,0x02,0x00,0x02, "Tamron", "272E", "SP AF 90mm F/2.8 Di Macro 1:1"},
{0xF8,0x55,0x64,0x64,0x24,0x24,0x84,0x06,0x00,0x02, "Tamron", "272NII", "SP AF 90mm F/2.8 Di Macro 1:1"},
{0x00,0x4C,0x7C,0x7C,0x2C,0x2C,0x00,0x02,0x00,0x02, "Tamron", "B01", "SP AF 180mm F/3.5 Di Model"},
//                                     "Tamron" "60E"    "SP AF 300mm F/2.8 LD-IF";
//                                     "Tamron" "360E"   "SP AF 300mm F/2.8 LD-IF";
//
{0xF6,0x3F,0x18,0x37,0x2C,0x34,0x84,0x06,0x00,0x02, "Tamron", "B001", "SP AF 10-24mm F/3.5-4.5 Di II LD Aspherical (IF)"},
{0x00,0x36,0x1C,0x2D,0x34,0x3C,0x00,0x06,0x00,0x02, "Tamron", "A13", "SP AF 11-18mm F/4.5-5.6 Di II LD Aspherical (IF)"},
{0x07,0x46,0x2B,0x44,0x24,0x30,0x03,0x02,0x00,0x02, "Tamron", "A05", "SP AF 17-35mm F/2.8-4 Di LD Aspherical (IF)"},
{0x00,0x53,0x2B,0x50,0x24,0x24,0x00,0x06,0x00,0x02, "Tamron", "A16", "SP AF 17-50mm F/2.8 XR Di II LD Aspherical (IF)"},
{0x00,0x54,0x2B,0x50,0x24,0x24,0x00,0x06,0x00,0x02, "Tamron", "A16NII", "SP AF 17-50mm F/2.8 XR Di II LD Aspherical (IF)"},
{0xF3,0x54,0x2B,0x50,0x24,0x24,0x84,0x0E,0x00,0x02, "Tamron", "B005", "SP AF 17-50mm F/2.8 XR Di II VC LD Aspherical (IF)"},
{0x00,0x3F,0x2D,0x80,0x2B,0x40,0x00,0x06,0x00,0x02, "Tamron", "A14", "AF 18-200mm F/3.5-6.3 XR Di II LD Aspherical (IF)"},
{0x00,0x3F,0x2D,0x80,0x2C,0x40,0x00,0x06,0x00,0x02, "Tamron", "A14", "AF 18-200mm F/3.5-6.3 XR Di II LD Aspherical (IF) Macro"},
{0x00,0x40,0x2D,0x80,0x2C,0x40,0x00,0x06,0x00,0x02, "Tamron", "A14NII", "AF 18-200mm F/3.5-6.3 XR Di II LD Aspherical (IF) Macro"},
{0x00,0x40,0x2D,0x88,0x2C,0x40,0x62,0x06,0x00,0x02, "Tamron", "A18", "AF 18-250mm F/3.5-6.3 Di II LD Aspherical (IF) Macro"},
{0x00,0x40,0x2D,0x88,0x2C,0x40,0x00,0x06,0x00,0x02, "Tamron", "A18NII", "AF 18-250mm F/3.5-6.3 Di II LD Aspherical (IF) Macro "},
{0xF5,0x40,0x2C,0x8A,0x2C,0x40,0x40,0x0E,0x00,0x02, "Tamron", "B003", "AF 18-270mm F/3.5-6.3 Di II VC LD Aspherical (IF) Macro"},
{0x07,0x40,0x2F,0x44,0x2C,0x34,0x03,0x02,0x00,0x02, "Tamron", "A10", "AF 19-35mm F/3.5-4.5"},
{0x07,0x40,0x30,0x45,0x2D,0x35,0x03,0x02,0x00,0x02, "Tamron", "A10", "AF 19-35mm F/3.5-4.5"},
{0x00,0x49,0x30,0x48,0x22,0x2B,0x00,0x02,0x00,0x02, "Tamron", "166D", "SP AF 20-40mm F/2.7-3.5"},
{0x0E,0x4A,0x31,0x48,0x23,0x2D,0x0E,0x02,0x00,0x02, "Tamron", "166D", "SP AF 20-40mm F/2.7-3.5"},
//                                     "Tamron" "266D"   "SP AF 20-40mm F/2.7-3.5 Aspherical-IF";
//                                     "Tamron" "73D"    "AF 24-70mm F/3.3-5.6 Aspherical";
{0x45,0x41,0x37,0x72,0x2C,0x3C,0x48,0x02,0x00,0x02, "Tamron", "190D", "SP AF 24-135mm F/3.5-5.6 AD Aspherical (IF) Macro"},
//                                     "Tamron" "159D"   "AF 28-70mm F/3.5-4.5";
//                                     "Tamron" "259D"   "AF 28-70mm F/3.5-4.5";
{0x33,0x54,0x3C,0x5E,0x24,0x24,0x62,0x02,0x00,0x02, "Tamron", "A09", "SP AF 28-75mm F/2.8 XR Di LD Aspherical (IF) Macro"},
{0xFA,0x54,0x3C,0x5E,0x24,0x24,0x84,0x06,0x00,0x02, "Tamron", "A09NII", "SP AF 28-75mm F/2.8 XR Di LD Aspherical (IF) Macro"},
{0x10,0x3D,0x3C,0x60,0x2C,0x3C,0xD2,0x02,0x00,0x02, "Tamron", "177D", "AF 28-80mm F/3.5-5.6 Aspherical"},
{0x45,0x3D,0x3C,0x60,0x2C,0x3C,0x48,0x02,0x00,0x02, "Tamron", "177D", "AF 28-80mm F/3.5-5.6 Aspherical"},
{0x00,0x48,0x3C,0x6A,0x24,0x24,0x00,0x02,0x00,0x02, "Tamron", "176D", "SP AF 28-105mm F/2.8 LD Aspherical IF"},
//                                     "Tamron" "276D"   "SP AF 28-105mm F/2.8 LD Aspherical IF";
//                                     "Tamron" "179D"   "AF 28-105mm F4.0-5.6 IF";
//                                     "Tamron" "285D"   "AF 28-300mm F/3.8-6.3 LD Aspherical IF Silver";
//                                     "Tamron" "271D"   "AF 28-200mm F/3.8-5.6 LD Aspherical IF Super Silver";
//                                     "Tamron" "471D"   "AF 28-200mm F/3.8-5.6 Aspherical IF Super2 Silver";
{0x0B,0x3E,0x3D,0x7F,0x2F,0x3D,0x0E,0x00,0x00,0x02, "Tamron", "71D", "AF 28-200mm F/3.8-5.6"},
{0x0B,0x3E,0x3D,0x7F,0x2F,0x3D,0x0E,0x02,0x00,0x02, "Tamron", "171D", "AF 28-200mm F/3.8-5.6D"},
{0x12,0x3D,0x3C,0x80,0x2E,0x3C,0xDF,0x02,0x00,0x02, "Tamron", "271D", "AF 28-200mm F/3.8-5.6 AF Aspherical LD (IF)"},
{0x4D,0x41,0x3C,0x8E,0x2B,0x40,0x62,0x02,0x00,0x02, "Tamron", "A061", "AF 28-300mm F/3.5-6.3 XR Di LD Aspherical (IF)"},
{0x4D,0x41,0x3C,0x8E,0x2C,0x40,0x62,0x02,0x00,0x02, "Tamron", "185D", "AF 28-300mm F/3.5-6.3 XR LD Aspherical (IF)"},
{0xF9,0x40,0x3C,0x8E,0x2C,0x40,0x40,0x0E,0x00,0x02, "Tamron", "A20", "AF 28-300mm F/3.5-6.3 XR Di VC LD Aspherical (IF) Macro"},
//                                     "Tamron" "63D"    "AF 35-90mm F/4-5.6";
//                                     "Tamron" "65D"    "SP AF 35-105mm F/2.8 Aspherical";
//                                     "Tamron" ""       "AF 35-135mm F/3.5-4.5";
{0x00,0x47,0x53,0x80,0x30,0x3C,0x00,0x06,0x00,0x02, "Tamron", "A15", "AF 55-200mm F/4-5.6 Di II LD"},
{0xF7,0x53,0x5C,0x80,0x24,0x24,0x84,0x06,0x00,0x02, "Tamron", "A001", "SP AF 70-200mm F/2.8 Di LD (IF) Macro"},
//                                     "Tamron" "67D"    "SP AF 70-210mm f/2.8 LD";
//                                     "Tamron" ""       "AF 70-210mm F/3.5-4.5";
//                                     "Tamron" "158D"   "AF 70-210mm F/4-5.6";
//                                     "Tamron" "258D"   "AF 70-210mm F/4-5.6";
//                                     "Tamron" "67D"    "SP AF 70-210mm F/2.8 LD";
//                                     "Tamron" "172D"   "AF 70-300mm F/4-5.6";
//                                     "Tamron" "472D"   "AF 70-300mm F/4-5.6 LD";
{0x69,0x48,0x5C,0x8E,0x30,0x3C,0x6F,0x02,0x00,0x02, "Tamron", "772D", "AF 70-300mm F/4-5.6 LD Macro 1:2"},
{0x00,0x48,0x5C,0x8E,0x30,0x3C,0x00,0x06,0x00,0x02, "Tamron", "A17", "AF 70-300mm F/4-5.6 Di LD Macro 1:2"},
//                                     "Tamron" "872D"   "AF 75-300mm F/4-5.6 LD";
//                                     "Tamron" "278D"   "AF 80-210mm F/4.5-5.6";
//                                     "Tamron" "62D"    "AF 90-300mm F/4.5-5.6";
//                                     "Tamron" "186D"   "AF 100-300mm F/5-6.3";
{0x20,0x3C,0x80,0x98,0x3D,0x3D,0x1E,0x02,0x00,0x02, "Tamron", "75D", "AF 200-400mm F/5.6 LD IF"},
{0x00,0x3E,0x80,0xA0,0x38,0x3F,0x00,0x02,0x00,0x02, "Tamron", "A08", "SP AF 200-500mm F/5-6.3 Di LD (IF)"},
{0x00,0x3F,0x80,0xA0,0x38,0x3F,0x00,0x02,0x00,0x02, "Tamron", "A08", "SP AF 200-500mm F/5-6.3 Di"},
//
//------------------------------------------------------------------------------
// Tokina Lenses by focal length, first fixed then zoom lenses
//------------------------------------------------------------------------------
//
{0x00,0x40,0x2B,0x2B,0x2C,0x2C,0x00,0x02,0x00,0x03, "Tokina", "", "AT-X 17 AF PRO (AF 17mm f/3.5)"},
{0x00,0x47,0x44,0x44,0x24,0x24,0x00,0x06,0x00,0x03, "Tokina", "T303503", "AT-X M35 PRO DX (AF 35mm f/2.8 Macro)"},
{0x00,0x54,0x68,0x68,0x24,0x24,0x00,0x02,0x00,0x03, "Tokina", "T310003N", "AT-X M100 PRO D (AF 100mm f/2.8 Macro)"},
//                                     "Tokina" ""          "AT-X 304 AF (AF 300mm f/4.0)";
{0x00,0x54,0x8E,0x8E,0x24,0x24,0x00,0x02,0x00,0x03, "Tokina", "", "AT-X 300 AF PRO (AF 300mm f/2.8)"},
//                                     "Tokina" ""          "AT-X 400 AF SD (AF 400mm f/5.6)";
//
{0x00,0x40,0x18,0x2B,0x2C,0x34,0x00,0x06,0x00,0x03, "Tokina", "T4101703", "AT-X 107 DX Fisheye (AF 10-17mm f/3.5-4.5)"},
{0x00,0x48,0x1C,0x29,0x24,0x24,0x00,0x06,0x00,0x03, "Tokina", "T4111603", "AT-X 116 PRO DX (AF 11-16mm f/2.8)"},
{0x00,0x3C,0x1F,0x37,0x30,0x30,0x00,0x06,0x00,0x03, "Tokina", "T4122403", "AT-X 124 AF PRO DX (AF 12-24mm f/4)"},
{0x7A,0x3C,0x1F,0x37,0x30,0x30,0x7E,0x06,0x00,0x03, "Tokina", "T4122423", "AT-X 124 AF PRO DX II (AF 12-24mm f/4)"},
{0x00,0x48,0x29,0x50,0x24,0x24,0x00,0x06,0x00,0x03, "Tokina", "", "AT-X 165 PRO DX (AF 16-50mm f/2.8)"},
{0x00,0x40,0x2A,0x72,0x2C,0x3C,0x00,0x06,0x00,0x03, "Tokina", "", "AT-X 16.5-135 DX (AF 16.5-135mm F3.5-5.6)"},
//                                     "Tokina" ""          "AF 193 (AF 19-35mm f/3.5-4.5)"; 
{0x2F,0x48,0x30,0x44,0x24,0x24,0x29,0x02,0x00,0x03, "Tokina", "", "AT-X 235 AF PRO (AF 20-35mm f/2.8)"},
//                                     "Tokina" ""          "AF 235 (AF 20-35mm f/3.5-4.5)";
{0x2F,0x40,0x30,0x44,0x2C,0x34,0x29,0x02,0x00,0x03, "Tokina", "", "AF 235 II (AF 20-35mm f/3.5-4.5)"},
//                                     "Tokina" ""          "AT-X 240 AF (AF 24-40mm f/2.8)";
//                                     "Tokina" ""          "AT-X 242 AF (AF 24-200mm f/3.5-4.5)";
{0x25,0x48,0x3C,0x5C,0x24,0x24,0x1B,0x02,0x00,0x03, "Tokina", "", "AT-X 270 AF PRO II (AF 28-70mm f/2.6-2.8)"},
//25 48 3C 5C 24 24 1B" "02" "00" "03" "Tokina" ""          "AT-X 287 AF PRO SV (AF 28-70mm f/2.8)";
{0x07,0x48,0x3C,0x5C,0x24,0x24,0x03,0x00,0x00,0x03, "Tokina", "", "AT-X 287 AF (AF 28-70mm f/2.8)"},
{0x07,0x47,0x3C,0x5C,0x25,0x35,0x03,0x00,0x00,0x03, "Tokina", "", "AF 287 SD (AF 28-70mm f/2.8-4.5)"},
//                                     "Tokina" ""          "AF 270 II (AF 28-70mm f/3.5-4.5)";
{0x00,0x48,0x3C,0x60,0x24,0x24,0x00,0x02,0x00,0x03, "Tokina", "", "AT-X 280 AF PRO (AF 28-80mm f/2.8)"},
//                                     "Tokina" ""          "AF 280 II EMZ (AF 28-80mm f/3.5-5.6)";
//                                     "Tokina" ""          "AF 205 (AF 28-105mm f/3.5-4.5);
//                                     "Tokina" ""          "AF 282 EMZ II (AF 28-210mm f/4.2-6.5)";
//                                     "Tokina" ""          "AF 370 II (AF 35-70mm f/3.5-4.6)";
//                                     "Tokina" ""          "AF 353 (AF 35-300mm f/4.5-6.7);
{0x00,0x48,0x50,0x72,0x24,0x24,0x00,0x06,0x00,0x03, "Tokina", "", "AT-X 535 PRO DX (AF 50-135mm f/2.8)"},
//                                     "Tokina" ""          "AF 210 II SD (AF 70-210mm f/4.0-5.6)";
//                                     "Tokina" ""          "AF-730 II (AF 75-300mm f/4.5-5.6)";
//                                     "Tokina" ""          "AT-X 828 AF (AF 80-200mm f/2.8)";
{0x14,0x54,0x60,0x80,0x24,0x24,0x0B,0x00,0x00,0x03, "Tokina", "", "AT-X 828 AF PRO (AF 80-200mm f/2.8)"},
//                                     "Tokina" ""          "AT-X 840 AF (AF 80-400mm f/4.5-5.6)";
{0x24,0x44,0x60,0x98,0x34,0x3C,0x1A,0x02,0x00,0x03, "Tokina", "", "AT-X 840 AF-II (AF 80-400mm f/4.5-5.6)"},
{0x00,0x44,0x60,0x98,0x34,0x3C,0x00,0x02,0x00,0x03, "Tokina", "", "AT-X 840 AF D (AF 80-400mm f/4.5-5.6)"},
{0x14,0x48,0x68,0x8E,0x30,0x30,0x0B,0x00,0x00,0x03, "Tokina", "", "AT-X 340 AF (AF 100-300mm f/4)"},
//                                     "Tokina" ""          "AT-X 340 AF-II (AF 100-300mm f/4)";
//                                     "Tokina" ""          "AF 130 EMZ II (AF 100-300mm f/5.6-6.7)";
//                                     "Tokina" ""          "AF 140 EMZ (AF 100-400mm f/4.5-6.3)";
//
//------------------------------------------------------------------------------
// Lenses from various other brands
//------------------------------------------------------------------------------
//
{0x06,0x3F,0x68,0x68,0x2C,0x2C,0x06,0x00,0x00,0x04, "Cosina", "", "AF 100mm F3.5 Macro"},
{0x07,0x36,0x3D,0x5F,0x2C,0x3C,0x03,0x00,0x00,0x04, "Cosina", "", "AF Zoom 28-80mm F3.5-5.6 MC Macro"},
{0x07,0x46,0x3D,0x6A,0x25,0x2F,0x03,0x00,0x00,0x04, "Cosina", "", "AF Zoom 28-105mm F2.8-3.8 MC"},
//                                     "Cosina" "" "AF Zoom 28-210mm F3.5-5.6"; 
//                                     "Cosina" "" "AF Zoom 28-210mm F4.2-6.5 Aspherical IF";
//                                     "Cosina" "" "AF Zoom 28-300mm F4.0-6.3"; 
//                                     "Cosina" "" "AF Zoom 70-210mm F2.8-4.0"; 
{0x12,0x36,0x5C,0x81,0x35,0x3D,0x09,0x00,0x00,0x04, "Cosina", "", "AF Zoom 70-210mm F4.5-5.6 MC Macro"},
{0x12,0x39,0x5C,0x8E,0x34,0x3D,0x08,0x02,0x00,0x04, "Cosina", "", "AF Zoom 70-300mm F4.5-5.6 MC Macro"},
{0x12,0x3B,0x68,0x8D,0x3D,0x43,0x09,0x02,0x00,0x04, "Cosina", "", "AF Zoom 100-300mm F5.6-6.7 MC Macro"},
//                                     "Cosina" "" "AF Zoom 100-400mm F5.6-6.7 MC";
//
{0x00,0x40,0x31,0x31,0x2C,0x2C,0x00,0x00,0x00,0x04, "Voigtlander", "BA295AN", "Color Skopar 20mm F3.5 SLII Aspherical"},
{0x00,0x54,0x48,0x48,0x18,0x18,0x00,0x00,0x00,0x04, "Voigtlander", "BA229DN", "Ultron 40mm F2 SLII Aspherical"},
{0x00,0x54,0x55,0x55,0x0C,0x0C,0x00,0x00,0x00,0x04, "Voigtlander", "BA239BN", "Nokton 58mm F1.4 SLII"},
//
{0x00,0x54,0x56,0x56,0x30,0x30,0x00,0x00,0x00,0x0C, "Coastal Optical Systems", "", "60mm 1:4 UV-VIS-IR Macro Apo"},
//
{0x02,0x40,0x44,0x5C,0x2C,0x34,0x02,0x00,0x00,0x15, "Exakta", "", "AF 35-70mm 1:3.5-4.5 MC"},
{0x07,0x3E,0x30,0x43,0x2D,0x35,0x03,0x00,0x00,0x14, "Soligor", "", "AF Zoom 19-35mm 1:3.5-4.5 MC"},
{0x03,0x43,0x5C,0x81,0x35,0x35,0x02,0x00,0x00,0x13, "Soligor", "", "AF C/D Zoom UMCS 70-210mm 1:4.5"},
{0x12,0x4A,0x5C,0x81,0x31,0x3D,0x09,0x00,0x00,0x13, "Soligor", "", "AF C/D Auto Zoom+Macro 70-210mm 1:4-5.6 UMCS"},
//
{0x00,0x00,0x00,0x00,0x00,0x00,0xF1,0x0C,0x10,0x08, "Kenko", "KE-MCP1DGN", "Teleplus PRO 300 DG 1.4x"},
{0x00,0x00,0x00,0x00,0x00,0x00,0xF2,0x18,0x10,0x08, "Kenko", "KE-MCP2DGN", "Teleplus PRO 300 DG 2.0x"},
//
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00, "Manual Lens", "", "No CPU"},
//
//------------------------------------------------------------------------------
//
// Lenses, that were upgraded with custom CPU
//
{0x00,0x47,0x10,0x10,0x24,0x24,0x00,0x00,0x00,0x00, "Nikon", "JAA604AC", "Fisheye Nikkor 8mm f/2.8 AiS"},
{0x00,0x54,0x44,0x44,0x0C,0x0C,0x00,0x00,0x00,0x00, "Nikon", "JAA115AD", "Nikkor 35mm f/1.4 AiS"},
{0x00,0x48,0x50,0x50,0x18,0x18,0x00,0x00,0x00,0x00, "Nikon", "", "Nikkor H 50mm f/2"},
{0x00,0x48,0x68,0x68,0x24,0x24,0x00,0x00,0x00,0x00, "Nikon", "JAA304AA", "Series E 100mm f/2.8"},
{0x00,0x4C,0x6A,0x6A,0x20,0x20,0x00,0x00,0x00,0x00, "Nikon", "JAA305AA", "Nikkor 105mm f/2.5 AiS"},
{0x00,0x48,0x80,0x80,0x30,0x30,0x00,0x00,0x00,0x00, "Nikon", "JAA313AA", "Nikkor 200mm f/4 AiS"},
//
{0,0,0,0,0,0,0,0,0,0, NULL, NULL, NULL}
};

        if (metadata == 0) return os << value;

        byte raw[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

        static const char* tags[] = {
            "LensIDNumber",
            "LensFStops",
            "MinFocalLength",
            "MaxFocalLength",
            "MaxApertureAtMinFocal",
            "MaxApertureAtMaxFocal",
            "MCUVersion"
        };

        const std::string pre = std::string("Exif.") + group + std::string(".");
        for (unsigned int i = 0; i < 7; ++i) {
            ExifKey key(pre + std::string(tags[i]));
            ExifData::const_iterator md = metadata->findKey(key);
            if (md == metadata->end() || md->typeId() != unsignedByte || md->count() == 0) {
                return os << value;
            }
            raw[i] = static_cast<byte>(md->toLong());
        }

        ExifData::const_iterator md = metadata->findKey(ExifKey("Exif.Nikon3.LensType"));
        if (md == metadata->end() || md->typeId() != unsignedByte || md->count() == 0) {
            return os << value;
        }
        raw[7] = static_cast<byte>(md->toLong());

        for (int i = 0; fmountlens[i].lensname != NULL; ++i) {
            if (   raw[0] == fmountlens[i].lid
                && raw[1] == fmountlens[i].stps
                && raw[2] == fmountlens[i].focs
                && raw[3] == fmountlens[i].focl
                && raw[4] == fmountlens[i].aps
                && raw[5] == fmountlens[i].apl
                && raw[6] == fmountlens[i].lfw
                && raw[7] == fmountlens[i].ltype) {
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

    std::ostream& Nikon3MakerNote::print0x009a(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        if (value.count() != 2 || value.typeId() != unsignedRational) {
            return os << value;
        }
        float f1 = value.toFloat(0);
        float f2 = value.toFloat(1);
        return os << f1 << " x " << f2 << " um";
    }

    std::ostream& Nikon3MakerNote::print0x009e(std::ostream& os,
                                               const Value& value,
                                               const ExifData*)
    {
        if (value.count() != 10 || value.typeId() != unsignedShort) {
            return os << value;
        }
        std::string s;
        bool trim = true;
        for (int i = 9; i >= 0; --i) {
            long l = value.toLong(i);
            if (i > 0 && l == 0 && trim) continue;
            if (l != 0) trim = false;
            std::string d = s.empty() ? "" : "; ";
            const TagDetails* td = find(nikonRetouchHistory, l);
            if (td) {
                s = std::string(exvGettext(td->label_)) + d + s;
            }
            else {
                s = std::string(_("Unknown")) + std::string(" (") + toString(l) + std::string(")") + d + s;
            }
        }
        return os << s;
    }
}                                       // namespace Exiv2
