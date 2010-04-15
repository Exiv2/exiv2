// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2010 Andreas Huggel <ahuggel@gmx.net>
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
  File:      sonymn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   18-Apr-05, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "minoltasonyvalues.hpp"
#include "sonymn.hpp"
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

    // -- Standard Sony Makernotes tags ---------------------------------------------------------------

    //! Lookup table to translate Sony image quality values to readable labels
    extern const TagDetails sonyImageQuality[] = {
        { 0, N_("Raw")                   },
        { 1, N_("Super Fine")            },
        { 2, N_("Fine")                  },
        { 3, N_("Standard")              },
        { 4, N_("Economy")               },
        { 5, N_("Extra Fine")            },
        { 6, N_("Raw + JPEG")            },
        { 7, N_("Compressed Raw")        },
        { 8, N_("Compressed Raw + JPEG") }
    };

    //! Lookup table to translate Sony teleconverter model values to readable labels
    extern const TagDetails sonyTeleconverterModel[] = {
        { 0x00, N_("None")                     },
        { 0x48, N_("Minolta AF 2x APO (D)")    },
        { 0x50, N_("Minolta AF 2x APO II")     },
        { 0x88, N_("Minolta AF 1.4x APO (D)")  },
        { 0x90, N_("Minolta AF 1.4x APO II")   }
    };

    //! Lookup table to translate Sony Std camera settings white balance values to readable labels
    extern const TagDetails sonyWhiteBalanceStd[] = {
        { 0x00,  N_("Auto")                           },
        { 0x01,  N_("Color Temperature/Color Filter") },
        { 0x10,  N_("Daylight")                       },
        { 0x20,  N_("Cloudy")                         },
        { 0x30,  N_("Shade")                          },
        { 0x40,  N_("Tungsten")                       },
        { 0x50,  N_("Flash")                          },
        { 0x60,  N_("Fluorescent")                    },
        { 0x70,  N_("Custom")                         }
    };

    //! Lookup table to translate Sony Auto HDR values to readable labels
    extern const TagDetails sonyAutoHDR[] = {
        { 0x00000, N_("Off") },
        { 0x10001, N_("On")  }
    };

    //! Lookup table to translate Sony model ID values to readable labels
    extern const TagDetails sonyModelId[] = {
        { 2,   "DSC-R1"    },
        { 256, "DSLR-A100" },
        { 257, "DSLR-A900" },
        { 258, "DSLR-A700" },
        { 259, "DSLR-A200" },
        { 260, "DSLR-A350" },
        { 261, "DSLR-A300" },
        { 263, "DSLR-A380" },
        { 264, "DSLR-A330" },
        { 265, "DSLR-A230" },
        { 269, "DSLR-A850" },
        { 273, "DSLR-A550" },
        { 274, "DSLR-A500" },
        { 275, "DSLR-A450" }
    };

    //! Lookup table to translate Sony scene mode values to readable labels
    extern const TagDetails sonySceneMode[] = {
        { 0,  N_("Standard")            },
        { 1,  N_("Portrait")            },
        { 2,  N_("Text")                },
        { 3,  N_("Night Scene")         },
        { 4,  N_("Sunset")              },
        { 5,  N_("Sports")              },
        { 6,  N_("Landscape")           },
        { 7,  N_("Night Portrait")      },
        { 8,  N_("Macro")               },
        { 9,  N_("Super Macro")         },
        { 16, N_("Auto")                },
        { 17, N_("Night View/Portrait") }
    };

    //! Lookup table to translate Sony zone matching values
    extern const TagDetails sonyZoneMatching[] = {
        { 0, N_("ISO Setting Used") },
        { 1, N_("High Key") },
        { 2, N_("Low Key")  }
    };

    //! Lookup table to translate Sony dynamic range optimizer values
    extern const TagDetails sonyDynamicRangeOptimizer[] = {
        { 0,  N_("Off")           },
        { 1,  N_("Standard ")     },
        { 2,  N_("Advanced Auto") },
        { 3,  N_("Auto")          },
        { 8,  N_("Advanced Lv1")  },
        { 9,  N_("Advanced Lv2")  },
        { 10, N_("Advanced Lv3")  },
        { 11, N_("Advanced Lv4")  },
        { 12, N_("Advanced Lv5")  }
    };

    //! Lookup table to translate Sony exposure mode values to readable labels
    extern const TagDetails sonyExposureMode[] = {
        { 0,  N_("Auto")                     },
        { 1,  N_("Portrait")                 },
        { 2,  N_("Beach")                    },
        { 4,  N_("Snow")                     },
        { 5,  N_("Landscape ")               },
        { 6,  N_("Program")                  },
        { 7,  N_("Aperture priority")        },
        { 8,  N_("Shutter priority")         },
        { 9,  N_("Night Scene / Twilight")   },
        { 10, N_("Hi-Speed Shutter")         },
        { 11, N_("Twilight Portrait")        },
        { 12, N_("Soft Snap")                },
        { 13, N_("Fireworks")                },
        { 15, N_("Manual")                   },
        { 18, N_("High Sensitivity")         },
        { 20, N_("Advanced Sports Shooting") },
        { 29, N_("Underwater")               },
        { 33, N_("Gourmet")                  },
        { 34, N_("Panorama")                 },
        { 35, N_("Handheld Twilight")        },
        { 36, N_("Anti Motion Blur")         },
        { 37, N_("Pet")                      },
        { 38, N_("Backlight Correction HDR") }
    };

    //! Lookup table to translate Sony Quality values to readable labels
    extern const TagDetails sonyQuality[] = {
        { 0, N_("Normal") },
        { 1, N_("Fine")   }
    };

    // Sony MakerNote Tag Info
    const TagInfo SonyMakerNote::tagInfo_[] = {

        TagInfo(0x0102, "Quality", N_("Image Quality"),
                N_("Image quality"),
                sonyIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(sonyImageQuality)),
        TagInfo(0x0104, "FlashExposureComp", N_("Flash Exposure Compensation"),
                N_("Flash exposure compensation in EV"),
                sonyIfdId, makerTags, signedRational, printValue),
        TagInfo(0x0105, "Teleconverter", N_("Teleconverter Model"),
                N_("Teleconverter Model"),
                sonyIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(sonyTeleconverterModel)),
        TagInfo(0x0112, "WhiteBalanceFineTune", N_("White Balance Fine Tune"),
                N_("White Balance Fine Tune Value"),
                sonyIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0114, "CameraSettings", N_("Camera Settings"),
                N_("Camera Settings"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x0115, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                sonyIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(sonyWhiteBalanceStd)),
        TagInfo(0x0116, "0x0116", "0x0116",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"),
                N_("PrintIM information"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x2000, "0x2000", "0x2000",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x2001, "PreviewImage", N_("Preview Image"),
                N_("Preview Image"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x2002, "0x2002", "0x2002",
                N_("Unknown"),
                sonyIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x2003, "0x2003", "0x2003",
                N_("Unknown"),
                sonyIfdId, makerTags, asciiString, printValue),
        TagInfo(0x2004, "0x2004", "0x2004",
                N_("Unknown"),
                sonyIfdId, makerTags, signedLong, printValue),
        TagInfo(0x2005, "0x2005", "0x2005",
                N_("Unknown"),
                sonyIfdId, makerTags, signedLong, printValue),
        TagInfo(0x2006, "0x2006", "0x2006",
                N_("Unknown"),
                sonyIfdId, makerTags, signedLong, printValue),
        TagInfo(0x2007, "0x2007", "0x2007",
                N_("Unknown"),
                sonyIfdId, makerTags, signedLong, printValue),
        TagInfo(0x2008, "0x2008", "0x2008",
                N_("Unknown"),
                sonyIfdId, makerTags, signedLong, printValue),
        TagInfo(0x2009, "0x2009", "0x2009",
                N_("Unknown"),
                sonyIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x200A, "AutoHDR", N_("Auto HDR"),
                N_("Auto High Definition Range"),
                sonyIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(sonyAutoHDR)),
        TagInfo(0x3000, "ShotInfo", N_("Shot Info"),
                N_("Shot Information"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0xb000, "FileFormat", N_("File Format"),
                N_("File Format"),
                sonyIfdId, makerTags, unsignedByte, print0xb000),
        TagInfo(0xb001, "SonyModelID", N_("Sony Model ID"),
                N_("Sony Model ID"),
                sonyIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(sonyModelId)),
        TagInfo(0xb020, "ColorReproduction", N_("Color Reproduction"),
                N_("Color Reproduction"),
                sonyIfdId, makerTags, asciiString, printValue),
        TagInfo(0xb021, "ColorTemperature", N_("Color Temperature"),
                N_("Color Temperature"),
                sonyIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0xb022, "ColorCompensationFilter", N_("Color Compensation Filter"),
                N_("Color Compensation Filter: negative is green, positive is magenta"),
                sonyIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0xb023, "SceneMode", N_("Scene Mode"),
                N_("Scene Mode"),
                sonyIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(sonySceneMode)),
        TagInfo(0xb024, "ZoneMatching", N_("Zone Matching"),
                N_("Zone Matching"),
                sonyIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(sonyZoneMatching)),
        TagInfo(0xb025, "DynamicRangeOptimizer", N_("Dynamic Range Optimizer"),
                N_("Dynamic Range Optimizer"),
                sonyIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(sonyDynamicRangeOptimizer)),
        TagInfo(0xb026, "ImageStabilization", N_("Image Stabilization"),
                N_("Image stabilization"),
                sonyIfdId, makerTags, unsignedLong, printMinoltaSonyBoolValue),
        TagInfo(0xb027, "LensID", N_("Lens ID"),
                N_("Lens identifier"),
                sonyIfdId, makerTags, unsignedLong, printMinoltaSonyLensID),

        // TODO
        TagInfo(0xb028, "MinoltaMakerNote", N_("Minolta MakerNote"),
                N_("Minolta MakerNote"),
                sonyIfdId, makerTags, undefined, printValue),

        TagInfo(0xb029, "ColorMode", N_("Color Mode"),
                N_("Color Mode"),
                sonyIfdId, makerTags, unsignedLong, printMinoltaSonyBoolValue),
        TagInfo(0xb02b, "FullImageSize", N_("Full Image Size"),
                N_("Full Image Size"),
                sonyIfdId, makerTags, unsignedLong, printImageSize),
        TagInfo(0xb02c, "PreviewImageSize", N_("Preview Image Size"),
                N_("Preview Image Size"),
                sonyIfdId, makerTags, unsignedLong, printImageSize),
        TagInfo(0xb040, "Macro", N_("Macro"),
                N_("Macro"),
                sonyIfdId, makerTags, unsignedShort, printMinoltaSonyBoolValue),
        TagInfo(0xb041, "ExposureMode", N_("Exposure Mode"),
                N_("Exposure Mode"),
                sonyIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(sonyExposureMode)),
        TagInfo(0xb047, "Quality", N_("Quality"),
                N_("Quality"),
                sonyIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(sonyQuality)),

        // TODO
        TagInfo(0xb04b, "AntiBlur", N_("Anti-Blur"),
                N_("Anti-Blur"),
                sonyIfdId, makerTags, unsignedShort, printValue),

        TagInfo(0xb04e, "LongExposureNoiseReduction", N_("Long Exposure Noise Reduction"),
                N_("Long Exposure Noise Reduction"),
                sonyIfdId, makerTags, unsignedShort, printMinoltaSonyBoolValue),

        // TODO
        TagInfo(0xb04f, "DynamicRangeOptimizer", N_("Dynamic Range Optimizer"),
                N_("Dynamic Range Optimizer"),
                sonyIfdId, makerTags, unsignedShort, printValue),

        // TODO
        TagInfo(0xb052, "IntelligentAuto", N_("Intelligent Auto"),
                N_("Intelligent Auto"),
                sonyIfdId, makerTags, unsignedShort, printValue),

        // TODO
        TagInfo(0xb054, "WhiteBalance", N_("White Balance"),
                N_("White Balance"),
                sonyIfdId, makerTags, unsignedShort, printValue),

        // --------------------------------------------------------------------------------

        // Old sony tags

        TagInfo(0x9001, "0x9001", "0x9001",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9002, "0x9002", "0x9002",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9003, "0x9003", "0x9003",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9004, "0x9004", "0x9004",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9005, "0x9005", "0x9005",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9006, "0x9006", "0x9006",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9007, "0x9007", "0x9007",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        TagInfo(0x9008, "0x9008", "0x9008",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownSonyMakerNoteTag)", "(UnknownSonyMakerNoteTag)",
                N_("Unknown SonyMakerNote tag"),
                sonyIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* SonyMakerNote::tagList()
    {
        return tagInfo_;
    }

    std::ostream& SonyMakerNote::print0xb000(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 4)
        {
            os << "(" << value << ")";
        }
        else
        {
            std::string val = value.toString(0) + value.toString(1) + value.toString(2) + value.toString(3);
            if      (val == "0002") os << "JPEG";
            else if (val == "1000") os << "SR2";
            else if (val == "2000") os << "ARW 1.0";
            else if (val == "3000") os << "ARW 2.0";
            else if (val == "3100") os << "ARW 2.1";
            else                    os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& SonyMakerNote::printImageSize(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() == 2)
            os << value.toString(0) << " x " << value.toString(1);
        else
            os << "(" << value << ")";

        return os;
    }

}                                       // namespace Exiv2
