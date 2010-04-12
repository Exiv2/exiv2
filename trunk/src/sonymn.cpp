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
        { 0x88, N_("Minolta AF 1.4x APO (D) ") },
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

        TagInfo(0x2000, "0x2000", "0x2000",
                N_("Unknown"),
                sonyIfdId, makerTags, undefined, printValue),
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

}                                       // namespace Exiv2
