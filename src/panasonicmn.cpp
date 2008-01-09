// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
  File:      panasonicmn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier.gilles@kdemail.net>
  History:   11-Jun-04, ahu: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "panasonicmn.hpp"
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

    //! @cond IGNORE
    PanasonicMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("Panasonic", "*", createPanasonicMakerNote);
        MakerNoteFactory::registerMakerNote(
            panasonicIfdId, MakerNote::AutoPtr(new PanasonicMakerNote));

        ExifTags::registerMakerTagInfo(panasonicIfdId, tagInfo_);
    }
    //! @endcond

    //! Quality, tag 0x0001
    extern const TagDetails panasonicQuality[] = {
        { 2, N_("High")      },
        { 3, N_("Normal")    },
        { 6, N_("Very High") },
        { 7, N_("Raw")       }
    };

    //! WhiteBalance, tag 0x0003
    extern const TagDetails panasonicWhiteBalance[] = {
        {  1, N_("Auto")            },
        {  2, N_("Daylight")        },
        {  3, N_("Cloudy")          },
        {  4, N_("Halogen")         },
        {  5, N_("Manual")          },
        {  8, N_("Flash")           },
        { 10, N_("Black and white") }
    };

    //! FocusMode, tag 0x0007
    extern const TagDetails panasonicFocusMode[] = {
        {  1, N_("Auto")               },
        {  2, N_("Manual")             },
        {  4, N_("Auto, focus button") },
        {  5, N_("Auto, continuous")   }
    };

    //! ImageStabilizer, tag 0x001a
    extern const TagDetails panasonicImageStabilizer[] = {
        {  2, N_("On, Mode 1") },
        {  3, N_("Off")        },
        {  4, N_("On, Mode 2") }
    };

    //! Macro, tag 0x001c
    extern const TagDetails panasonicMacro[] = {
        {   1, N_("On")         },
        {   2, N_("Off")        },
        { 257, N_("Tele-macro") }
    };

    //! ShootingMode, tag 0x001f
    extern const TagDetails panasonicShootingMode[] = {
        {  1, N_("Normal")                  },
        {  2, N_("Portrait")                },
        {  3, N_("Scenery")                 },
        {  4, N_("Sports")                  },
        {  5, N_("Night portrait")          },
        {  6, N_("Program")                 },
        {  7, N_("Aperture priority")       },
        {  8, N_("Shutter-speed priority")  },
        {  9, N_("Macro")                   },
        { 11, N_("Manual")                  },
        { 13, N_("Panning")                 },
        { 18, N_("Fireworks")               },
        { 19, N_("Party")                   },
        { 20, N_("Snow")                    },
        { 21, N_("Night scenery")           },
        { 22, N_("Food")                    }
    };

    //! Audio, tag 0x0020
    extern const TagDetails panasonicAudio[] = {
        { 1, N_("Yes") },
        { 2, N_("No")  }
    };

    //! ColorEffect, tag 0x0028
    extern const TagDetails panasonicColorEffect[] = {
        { 1, N_("Off")             },
        { 2, N_("Warm")            },
        { 3, N_("Cool")            },
        { 4, N_("Black and white") },
        { 5, N_("Sepia")           }
    };

    //! BustMode, tag 0x002a
    extern const TagDetails panasonicBurstMode[] = {
        { 0, N_("Off")              },
        { 1, N_("Low/High quality") },
        { 2, N_("Infinite")         }
    };

    //! Contrast, tag 0x002c
    extern const TagDetails panasonicContrast[] = {
        {   0, N_("Normal")   },
        {   1, N_("Low")      },
        {   2, N_("High")     },
        { 256, N_("Low")      },
        { 272, N_("Standard") },
        { 288, N_("High")     }
    };

    //! NoiseReduction, tag 0x002d
    extern const TagDetails panasonicNoiseReduction[] = {
        { 0, N_("Standard") },
        { 1, N_("Low")      },
        { 2, N_("High")     }
    };

    //! SelfTimer, tag 0x002e
    extern const TagDetails panasonicSelfTimer[] = {
        { 1, N_("Off") },
        { 2, N_("10s") },
        { 3, N_("2s")  }
    };

    //! Rotation, tag 0x0030
    extern const TagDetails panasonicRotation[] = {
        { 1, N_("Horizontal (normal)") },
        { 6, N_("Rotate 90 CW")        },
        { 8, N_("Rotate 270 CW")       }
    };

    //! ColorMode, tag 0x0032
    extern const TagDetails panasonicColorMode[] = {
        { 0, N_("Normal")  },
        { 1, N_("Natural") }
    };

    // Panasonic MakerNote Tag Info
    const TagInfo PanasonicMakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Quality", N_("Quality"),
                N_("Image Quality"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicQuality)),
        TagInfo(0x0002, "FirmwareVersion", N_("Firmware Version"),
                N_("Firmware version"),
                panasonicIfdId, makerTags, undefined, printValue),
        TagInfo(0x0003, "WhiteBalance", N_("White Balance"),
                N_("White balance setting"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicWhiteBalance)),
        TagInfo(0x0004, "0x0004", "0x0004",
                N_("Unknown"),
                panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "FocusMode", N_("Focus Mode"),
                N_("Focus mode"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicFocusMode)),
        TagInfo(0x000f, "SpotMode", N_("Spot Mode"), N_("Spot mode"),
                panasonicIfdId, makerTags, unsignedByte, print0x000f),
        TagInfo(0x001a, "ImageStabilizer", N_("ImageStabilizer"),
                N_("Image stabilizer"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicImageStabilizer)),
        TagInfo(0x001c, "Macro", N_("Macro"),
                N_("Macro mode"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicMacro)),
        TagInfo(0x001f, "ShootingMode", N_("Shooting Mode"),
                N_("Shooting mode"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicShootingMode)),
        TagInfo(0x0020, "Audio", N_("Audio"),
                N_("Audio"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicAudio)),
        TagInfo(0x0021, "DataDump", N_("Data Dump"),
                N_("Data dump"),
                panasonicIfdId, makerTags, undefined, printValue),
        TagInfo(0x0022, "0x0022", "0x0022",
                N_("Unknown"),
                panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0023, "WhiteBalanceBias", N_("White Balance Bias"),
                N_("White balance adjustment"),
                panasonicIfdId, makerTags, unsignedShort, print0x0023),
        TagInfo(0x0024, "FlashBias", N_("FlashBias"),
                N_("Flash bias"),
                panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0025, "SerialNumber", N_("Serial Number"),
                N_("This number is unique, and contains the date of manufacture, "
                   "but is not the same as the number printed on the camera body."),
                panasonicIfdId, makerTags, undefined, printValue),
        TagInfo(0x0026, "0x0026", "0x0026",
                N_("Unknown"),
                panasonicIfdId, makerTags, undefined, printValue),
        TagInfo(0x0027, "0x0027", "0x0027",
                N_("Unknown"),
                panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0028, "ColorEffect", N_("Color Effect"),
                N_("Color effect"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicColorEffect)),
        TagInfo(0x0029, "0x0029", "0x0029",
                N_("Unknown"),
                panasonicIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x002a, "BurstMode", N_("Burst Mode"),
                N_("Burst mode"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicBurstMode)),
        TagInfo(0x002b, "SequenceNumber", N_("Sequence Number"),
                N_("Sequence number"),
                panasonicIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x002c, "Contrast", N_("Contrast"),
                N_("Contrast setting"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicContrast)),
        TagInfo(0x002d, "NoiseReduction", N_("NoiseReduction"),
                N_("Noise reduction"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicNoiseReduction)),
        TagInfo(0x002e, "SelfTimer", N_("Self Timer"),
                N_("Self timer"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicSelfTimer)),
        TagInfo(0x002f, "0x002f", "0x002f",
                N_("Unknown"),
                panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0030, "Rotation", N_("Rotation"),
                N_("Rotation"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicRotation)),
        TagInfo(0x0031, "0x0031", "0x0031",
                N_("Unknown"),
                panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0032, "ColorMode", N_("Color Mode"),
                N_("Color mode"),
                panasonicIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(panasonicColorMode)),
        TagInfo(0x0036, "TravelDay", N_("Travel Day"),
                N_("Travel day"),
                panasonicIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"),
                N_("PrintIM information"),
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x4449, "0x4449", "0x4449",
                N_("Unknown"),
                panasonicIfdId, makerTags, undefined, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownPanasonicMakerNoteTag)", "(UnknownPanasonicMakerNoteTag)",
                N_("Unknown PanasonicMakerNote tag"),
                panasonicIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* PanasonicMakerNote::tagList()
    {
        return tagInfo_;
    }

    PanasonicMakerNote::PanasonicMakerNote(bool alloc)
        : IfdMakerNote(panasonicIfdId, alloc, false)
    {
        byte buf[] = {
            'P', 'a', 'n', 'a', 's', 'o', 'n', 'i', 'c', 0x00, 0x00, 0x00
        };
        readHeader(buf, 12, byteOrder_);
    }

    PanasonicMakerNote::PanasonicMakerNote(const PanasonicMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int PanasonicMakerNote::readHeader(const byte* buf,
                                       long        len,
                                       ByteOrder   /*byteOrder*/)
    {
        if (len < 12) return 1;

        header_.alloc(12);
        std::memcpy(header_.pData_, buf, header_.size_);
        // Adjust the offset of the IFD for the prefix
        start_ = 12;
        return 0;
    }

    int PanasonicMakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the Panasonic prefix
        if (   header_.size_ < 12
            || std::string(reinterpret_cast<char*>(header_.pData_), 9)
               != std::string("Panasonic", 9)) {
            rc = 2;
        }
        return rc;
    }

    PanasonicMakerNote::AutoPtr PanasonicMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    PanasonicMakerNote* PanasonicMakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote(new PanasonicMakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    PanasonicMakerNote::AutoPtr PanasonicMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    PanasonicMakerNote* PanasonicMakerNote::clone_() const
    {
        return new PanasonicMakerNote(*this);
    }

    std::ostream& PanasonicMakerNote::print0x000f(std::ostream& os,
                                                  const Value& value)
    {
        if (value.count() < 2 || value.typeId() != unsignedByte) {
            return os << value;
        }
        long l0 = value.toLong(0);
        if (l0 == 1) os << _("On");
        else if (l0 == 16) os << _("Off");
        else os << value;
        return os;
    } // PanasonicMakerNote::print0x000f

    std::ostream& PanasonicMakerNote::print0x0023(std::ostream& os,
                                                  const Value& value)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1)
           << value.toLong() / 3 << _(" EV");
        os.copyfmt(oss);

        return os;

    } // PanasonicMakerNote::print0x0023

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createPanasonicMakerNote(bool        alloc,
                                                const byte* /*buf*/,
                                                long        /*len*/,
                                                ByteOrder   /*byteOrder*/,
                                                long        /*offset*/)
    {
        return MakerNote::AutoPtr(new PanasonicMakerNote(alloc));
    }

}                                       // namespace Exiv2
