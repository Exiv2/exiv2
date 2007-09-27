// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2007 Michal Cihar <michal@cihar.com>
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
  File:      pentaxmn.cpp
  Version:   $Rev$
  Author(s): Michal Cihar <michal@cihar.com>
  Based on fujimn.cpp by:
               Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier.gilles@kdemail.net>
  History:   07-09-27 created
  Credits:   See header file.
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "pentaxmn.hpp"
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
    PentaxMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("PENTAX*", "*", createPentaxMakerNote);
        MakerNoteFactory::registerMakerNote(
            pentaxIfdId, MakerNote::AutoPtr(new PentaxMakerNote));

        ExifTags::registerMakerTagInfo(pentaxIfdId, tagInfo_);
    }
    //! @endcond

    //! ShootingMode, tag 0x0001
    extern const TagDetails pentaxShootingMode[] = {
        {   0, N_("Auto")                      },
        {   1, N_("Night-Scene")               },
        {   2, N_("Manual")                    },
    };

    //! CameraModel, tag 0x0005
    extern const TagDetails pentaxModel[] = {
        {    0x0000d, N_("Optio 330/430") },
        {    0x12926, N_("Optio 230") },
        {    0x12958, N_("Optio 330GS") },
        {    0x12962, N_("Optio 450/550") },
        {    0x1296c, N_("Optio S") },
        {    0x12994, N_("*ist D") },
        {    0x129b2, N_("Optio 33L") },
        {    0x129bc, N_("Optio 33LF") },
        {    0x129c6, N_("Optio 33WR/43WR/555") },
        {    0x129d5, N_("Optio S4") },
        {    0x12a02, N_("Optio MX") },
        {    0x12a0c, N_("Optio S40") },
        {    0x12a16, N_("Optio S4i") },
        {    0x12a34, N_("Optio 30") },
        {    0x12a52, N_("Optio S30") },
        {    0x12a66, N_("Optio 750Z") },
        {    0x12a70, N_("Optio SV") },
        {    0x12a75, N_("Optio SVi") },
        {    0x12a7a, N_("Optio X") },
        {    0x12a8e, N_("Optio S5i") },
        {    0x12a98, N_("Optio S50") },
        {    0x12aa2, N_("*ist DS") },
        {    0x12ab6, N_("Optio MX4") },
        {    0x12ac0, N_("Optio S5n") },
        {    0x12aca, N_("Optio WP") },
        {    0x12afc, N_("Optio S55") },
        {    0x12b10, N_("Optio S5z") },
        {    0x12b1a, N_("*ist DL") },
        {    0x12b24, N_("Optio S60") },
        {    0x12b2e, N_("Optio S45") },
        {    0x12b38, N_("Optio S6") },
        {    0x12b4c, N_("Optio WPi") },
        {    0x12b56, N_("BenQ DC X600") },
        {    0x12b60, N_("*ist DS2") },
        {    0x12b62, N_("Samsung GX-1S") },
        {    0x12b6a, N_("Optio A10") },
        {    0x12b7e, N_("*ist DL2") },
        {    0x12b80, N_("Samsung GX-1L") },
        {    0x12b9c, N_("K100D") },
        {    0x12b9d, N_("K110D") },
        {    0x12ba2, N_("K100D Super") },
        {    0x12bb0, N_("Optio T10") },
        {    0x12be2, N_("Optio W10") },
        {    0x12bf6, N_("Optio M10") },
        {    0x12c1e, N_("K10D") },
        {    0x12c20, N_("Samsung GX10") },
        {    0x12c28, N_("Optio S7") },
        {    0x12c32, N_("Optio M20") },
        {    0x12c3c, N_("Optio W20") },
        {    0x12c46, N_("Optio A20") },
        {    0x12c8c, N_("Optio M30") },
        {    0x12c78, N_("Optio E30") },
        {    0x12c82, N_("Optio T30") },
        {    0x12c96, N_("Optio W30") },
        {    0x12ca0, N_("Optio A30") },
        {    0x12cb4, N_("Optio E40") },
        {    0x12cbe, N_("Optio M40") },
        {    0x12cdc, N_("Optio S10") },
    };

    //! Quality, tag 0x0008
    extern const TagDetails pentaxQuality[] = {
        {   0, N_("Good") },
        {   1, N_("Better") },
        {   2, N_("Best") },
        {   3, N_("TIFF") },
        {   4, N_("RAW") },
    };

    //! Size, tag 0x0009
    extern const TagDetails pentaxSize[] = {
        {    0, N_("640x480") },
        {    1, N_("Full") },
        {    2, N_("1024x768") },
        {    3, N_("1280x960") },
        {    4, N_("1600x1200") },
        {    5, N_("2048x1536") },
        {    8, N_("2560x1920 or 2304x1728") },
        {    9, N_("3072x2304") },
        {    10, N_("3264x2448") },
        {    19, N_("320x240") },
        {    20, N_("2288x1712") },
        {    21, N_("2592x1944") },
        {    22, N_("2304x1728 or 2592x1944") },
        {    23, N_("3056x2296") },
        {    25, N_("2816x2212 or 2816x2112") },
        {    27, N_("3648x2736") },
    };

    //! Flash, tag 0x000c
    extern const TagDetails pentaxFlash[] = {
        {    0x000, N_("Auto, Did not fire") },
        {    0x001, N_("Off") },
        {    0x003, N_("Auto, Did not fire, Red-eye reduction") },
        {    0x100, N_("Auto, Fired") },
        {    0x102, N_("On") },
        {    0x103, N_("Auto, Fired, Red-eye reduction") },
        {    0x104, N_("On, Red-eye reduction") },
        {    0x105, N_("On, Wireless") },
        {    0x108, N_("On, Soft") },
        {    0x109, N_("On, Slow-sync") },
        {    0x10a, N_("On, Slow-sync, Red-eye reduction") },
        {    0x10b, N_("On, Trailing-curtain Sync") },
    };

    //! Focus, tag 0x000d
    extern const TagDetails pentaxFocus[] = {
        {    0, N_("Normal") },
        {    1, N_("Macro") },
        {    2, N_("Infinity") },
        {    3, N_("Manual") },
        {    5, N_("Pan Focus") },
        {    16, N_("AF-S") },
        {    17, N_("AF-C") },
    };

    //! AFPoint, tag 0x000e
    extern const TagDetails pentaxAFPoint[] = {
        {    0xffff, N_("Auto") },
        {    0xfffe, N_("Fixed Center") },
        {    1, N_("Upper-left") },
        {    2, N_("Top") },
        {    3, N_("Upper-right") },
        {    4, N_("Left") },
        {    5, N_("Mid-left") },
        {    6, N_("Center") },
        {    7, N_("Mid-right") },
        {    8, N_("Right") },
        {    9, N_("Lower-left") },
        {    10, N_("Bottom") },
        {    11, N_("Lower-right") },
    };

    //! ISO, tag 0x0014
    extern const TagDetails pentaxISO[] = {
        {    3, N_("50") },
        {    4, N_("64") },
        {    5, N_("80") },
        {    6, N_("100") },
        {    7, N_("125") },
        {    8, N_("160") },
        {    9, N_("200") },
        {    10, N_("250") },
        {    11, N_("320") },
        {    12, N_("400") },
        {    13, N_("500") },
        {    14, N_("640") },
        {    15, N_("800") },
        {    16, N_("1000") },
        {    17, N_("1250") },
        {    18, N_("1600") },
        {    21, N_("3200") },
        {    50, N_("50") },
        {    100, N_("100") },
        {    200, N_("200") },
        {    268, N_("200") },
        {    400, N_("400") },
        {    800, N_("800") },
        {    1600, N_("1600") },
        {    3200, N_("3200") },
    };

    //! Generic for Off/On switches
    extern const TagDetails pentaxOffOn[] = {
        {    0, N_("Off") },
        {    1, N_("On") },
    };

    //! Generic for Yes/No switches
    extern const TagDetails pentaxYesNo[] = {
        {    0, N_("No") },
        {    1, N_("Yes") },
    };

    //! MeteringMode, tag 0x0017
    extern const TagDetails pentaxMeteringMode[] = {
        {    0, N_("Multi Segment") },
        {    1, N_("Center Weighted") },
        {    2, N_("Spot") },
    };

    //! WhiteBallance, tag 0x0019
    extern const TagDetails pentaxWhiteBallance[] = {
        {    0, N_("Auto") },
        {    1, N_("Daylight") },
        {    2, N_("Shade") },
        {    3, N_("Fluorescent") },
        {    4, N_("Tungsten") },
        {    5, N_("Manual") },
        {    6, N_("DaylightFluorescent") },
        {    7, N_("DaywhiteFluorescent") },
        {    8, N_("WhiteFluorescent") },
        {    9, N_("Flash") },
        {    10, N_("Cloudy") },
        {    65534, N_("Unknown") },
        {    65535, N_("User Selected") },
    };

    //! WhiteBallance, tag 0x001a
    extern const TagDetails pentaxWhiteBallanceMode[] = {
        {    1, N_("Auto (Daylight)") },
        {    2, N_("Auto (Shade)") },
        {    3, N_("Auto (Flash)") },
        {    4, N_("Auto (Tungsten)") },
        {    7, N_("Auto (DaywhiteFluorescent)") },
        {    8, N_("Auto (WhiteFluorescent)") },
        {    10, N_("Auto (Cloudy)") },
        {    0xffff, N_("User-Selected") },
        {    0xfffe, N_("Preset (Fireworks?)") },
    };

    //! Saturation, tag 0x001f
    extern const TagDetails pentaxSaturation[] = {
        {    0, N_("Low") },
        {    1, N_("Normal") },
        {    2, N_("High") },
        {    3, N_("Med Low") },
        {    4, N_("Med High") },
        {    5, N_("Very Low") },
        {    6, N_("Very High") },
    };

    //! Contrast, tag 0x0020
    extern const TagDetails pentaxContrast[] = {
        {    0, N_("Low") },
        {    1, N_("Normal") },
        {    2, N_("High") },
        {    3, N_("Med Low") },
        {    4, N_("Med High") },
        {    5, N_("Very Low") },
        {    6, N_("Very High") },
    };

    //! Sharpness, tag 0x0021
    extern const TagDetails pentaxSharpness[] = {
        {    0, N_("Soft") },
        {    1, N_("Normal") },
        {    2, N_("Hard") },
        {    3, N_("Med Soft") },
        {    4, N_("Med Hard") },
        {    5, N_("Very Soft") },
        {    6, N_("Very Hard") },
    };

    //! Location, tag 0x0022
    extern const TagDetails pentaxLocation[] = {
        {    0, N_("Home town") },
        {    1, N_("Destination") },
    };

    //! City names, tags 0x0023 and 0x0024
    extern const TagDetails pentaxCities[] = {
        {    0, N_("Pago Pago") },
        {    1, N_("Honolulu") },
        {    2, N_("Anchorage") },
        {    3, N_("Vancouver") },
        {    4, N_("San Fransisco") },
        {    5, N_("Los Angeles") },
        {    6, N_("Calgary") },
        {    7, N_("Denver") },
        {    8, N_("Mexico City") },
        {    9, N_("Chicago") },
        {    10, N_("Miami") },
        {    11, N_("Toronto") },
        {    12, N_("New York") },
        {    13, N_("Santiago") },
        {    14, N_("Caracus") },
        {    15, N_("Halifax") },
        {    16, N_("Buenos Aires") },
        {    17, N_("Sao Paulo") },
        {    18, N_("Rio de Janeiro") },
        {    19, N_("Madrid") },
        {    20, N_("London") },
        {    21, N_("Paris") },
        {    22, N_("Milan") },
        {    23, N_("Rome") },
        {    24, N_("Berlin") },
        {    25, N_("Johannesburg") },
        {    26, N_("Istanbul") },
        {    27, N_("Cairo") },
        {    28, N_("Jerusalem") },
        {    29, N_("Moscow") },
        {    30, N_("Jeddah") },
        {    31, N_("Tehran") },
        {    32, N_("Dubai") },
        {    33, N_("Karachi") },
        {    34, N_("Kabul") },
        {    35, N_("Male") },
        {    36, N_("Delhi") },
        {    37, N_("Colombo") },
        {    38, N_("Kathmandu") },
        {    39, N_("Dacca") },
        {    40, N_("Yangon") },
        {    41, N_("Bangkok") },
        {    42, N_("Kuala Lumpur") },
        {    43, N_("Vientiane") },
        {    44, N_("Singapore") },
        {    45, N_("Phnom Penh") },
        {    46, N_("Ho Chi Minh") },
        {    47, N_("Jakarta") },
        {    48, N_("Hong Kong") },
        {    49, N_("Perth") },
        {    50, N_("Beijing") },
        {    51, N_("Shanghai") },
        {    52, N_("Manila") },
        {    53, N_("Taipei") },
        {    54, N_("Seoul") },
        {    55, N_("Adelaide") },
        {    56, N_("Tokyo") },
        {    57, N_("Guam") },
        {    58, N_("Sydney") },
        {    59, N_("Noumea") },
        {    60, N_("Wellington") },
        {    61, N_("Auckland") },
        {    62, N_("Lima") },
        {    63, N_("Dakar") },
        {    64, N_("Algiers") },
        {    65, N_("Helsinki") },
        {    66, N_("Athens") },
        {    67, N_("Nairobi") },
        {    68, N_("Amsterdam") },
        {    69, N_("Stockholm") },
        {    70, N_("Lisbon") },
    };

    //! ColorSpace, tag 0x0037
    extern const TagDetails pentaxColorSpace[] = {
        {    0, N_("sRGB") },
        {    1, N_("Adobe RGB") },
    };

    //! ImageTone, tag 0x004f
    extern const TagDetails pentaxImageTone[] = {
        {    0, N_("Natural") },
        {    1, N_("Bright") },
    };

    std::ostream& PentaxMakerNote::printPentaxVersion(std::ostream& os, const Value& value)
    {
        std::string val = value.toString();
        size_t i;
        while ((i = val.find(' ')) != std::string::npos && i != val.length() - 1) {
            val.replace(i, 1, ".");
        }
        os << val;
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxResolution(std::ostream& os, const Value& value)
    {
        std::string val = value.toString();
        size_t i;
        while ((i = val.find(' ')) != std::string::npos && i != val.length() - 1) {
            val.replace(i, 1, "x");
        }
        os << val;
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxDate(std::ostream& os, const Value& value)
    {
        /* I choose same format as is used inside EXIF itself */
        os << ((value.toLong(0) << 8) + value.toLong(1));
        os << ":";
        os << std::setw(2) << std::setfill('0') << value.toLong(2);
        os << ":";
        os << std::setw(2) << std::setfill('0') << value.toLong(3);
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxTime(std::ostream& os, const Value& value)
    {
        os << std::setw(2) << std::setfill('0') << value.toLong(0);
        os << ":";
        os << std::setw(2) << std::setfill('0') << value.toLong(1);
        os << ":";
        os << std::setw(2) << std::setfill('0') << value.toLong(2);
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxExposure(std::ostream& os, const Value& value)
    {
        os << static_cast<float>(value.toLong()) / 100 << " ms";
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxFValue(std::ostream& os, const Value& value)
    {
        os << "F" << std::setprecision(2)
           << static_cast<float>(value.toLong()) / 10;
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxFocalLength(std::ostream& os, const Value& value)
    {
        os << std::fixed << std::setprecision(1)
           << static_cast<float>(value.toLong()) / 100
           << " mm";
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxCompensation(std::ostream& os, const Value& value)
    {
        os << std::setprecision(2)
           << (static_cast<float>(value.toLong()) - 50) / 10
           << " EV";
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxTemperature(std::ostream& os, const Value& value)
    {
        os << value.toLong() << " C";
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxFlashCompensation(std::ostream& os, const Value& value)
    {
        os << std::setprecision(2)
           << static_cast<float>(value.toLong()) / 256
           << " EV";
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxBracketing(std::ostream& os, const Value& value)
    {
        long l0 = value.toLong(0);

        if (l0 < 10) {
            os << std::setprecision(2)
               << static_cast<float>(l0) / 3
               << " EV";
        } else {
            os << std::setprecision(2)
               << static_cast<float>(l0) - 9.5
               << " EV";
        }

        if (value.size() == 2) {
            long l1 = value.toLong(1);
            long type;
            long range;
            os << " (";
            if (l1 == 0) {
                os << _("No extended bracketing");
            } else {
                type = l1 >> 8;
                range = l1 & 0xff;
                switch (type) {
                    case 1:
                        os << _("WB-BA");
                        break;
                    case 2:
                        os << _("WB-GM");
                        break;
                    case 3:
                        os << _("Saturation");
                        break;
                    case 4:
                        os << _("Sharpness");
                        break;
                    case 5:
                        os << _("Contrast");
                        break;
                    default:
                        os << _("Unknown ") << type;
                        break;
                }
                os << " " << range;
            }
            os << ")";
        }
        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxImageProcessing(std::ostream& os, const Value& value)
    {
        long l0 = value.toLong(0);
        long l1 = value.toLong(1);
        long l2 = value.toLong(2);
        long l3 = value.toLong(3);

        if (l0 == 0 && l1 == 0 && l2 == 0 && l3 == 0) {
            os << _("Unprocessed");
        } else if (l0 == 0 && l1 == 0 && l2 == 0 && l3 == 4) {
            os << _("Digital Filter");
        } else if (l0 == 2 && l1 == 0 && l2 == 0 && l3 == 0) {
            os << _("Cropped");
        } else if (l0 == 4 && l1 == 0 && l2 == 0 && l3 == 0) {
            os << _("Color Filter");
        } else if (l0 == 16 && l1 == 0 && l2 == 0 && l3 == 0) {
            os << _("Frame Synthesis?");
        } else {
            os << _("Unknown") << " (" << l0 << " " << l1 << " " << l2 << " " << l3 << ")";
        }

        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxPictureMode(std::ostream& os, const Value& value)
    {
        long l0 = value.toLong(0);
        long l1 = value.toLong(1);
        long l2 = value.toLong(2);

        if (l0 == 0 && l1 == 0 && l2 == 0) {
            os << _("Program");
        } else if (l0 == 0 && l1 == 4 && l2 == 0) {
            os << _("Standard");
        } else if (l0 == 0 && l1 == 5 && l2 == 0) {
            os << _("Portrait");
        } else if (l0 == 0 && l1 == 6 && l2 == 0) {
            os << _("Landscape");
        } else if (l0 == 0 && l1 == 7 && l2 == 0) {
            os << _("Macro");
        } else if (l0 == 0 && l1 == 8 && l2 == 0) {
            os << _("Sport");
        } else if (l0 == 0 && l1 == 9 && l2 == 0) {
            os << _("Night Scene Portrait");
        } else if (l0 == 0 && l1 == 10 && l2 == 0) {
            os << _("No Flash");
        /* SCN modes (menu-selected) */
        } else if (l0 == 0 && l1 == 11 && l2 == 0) {
            os << _("Night Scene");
        } else if (l0 == 0 && l1 == 12 && l2 == 0) {
            os << _("Surf & Snow");
        } else if (l0 == 0 && l1 == 13 && l2 == 0) {
            os << _("Text");
        } else if (l0 == 0 && l1 == 14 && l2 == 0) {
            os << _("Sunset");
        } else if (l0 == 0 && l1 == 15 && l2 == 0) {
            os << _("Kids");
        } else if (l0 == 0 && l1 == 16 && l2 == 0) {
            os << _("Pet");
        } else if (l0 == 0 && l1 == 17 && l2 == 0) {
            os << _("Candlelight");
        } else if (l0 == 0 && l1 == 18 && l2 == 0) {
            os << _("Museum");
        /* AUTO PICT modes (auto-selected) */
        } else if (l0 == 1 && l1 == 4 && l2 == 0) {
            os << _("Auto PICT (Standard)");
        } else if (l0 == 1 && l1 == 5 && l2 == 0) {
            os << _("Auto PICT (Portrait)");
        } else if (l0 == 1 && l1 == 6 && l2 == 0) {
            os << _("Auto PICT (Landscape)");
        } else if (l0 == 1 && l1 == 7 && l2 == 0) {
            os << _("Auto PICT (Macro)");
        } else if (l0 == 1 && l1 == 8 && l2 == 0) {
            os << _("Auto PICT (Sport)");
        /* Manual dial modes */
        } else if (l0 == 2 && l1 == 0 && l2 == 0) {
            os << _("Program AE");
        } else if (l0 == 3 && l1 == 0 && l2 == 0) {
            os << _("Green Mode");
        } else if (l0 == 4 && l1 == 0 && l2 == 0) {
            os << _("Shutter Speed Priority");
        } else if (l0 == 5 && l1 == 0 && l2 == 0) {
            os << _("Aperture Priority");
        } else if (l0 == 8 && l1 == 0 && l2 == 0) {
            os << _("Manual");
        } else if (l0 == 9 && l1 == 0 && l2 == 0) {
            os << _("Bulb");
           /* *istD modes */
        } else if (l0 == 2 && l1 == 0 && l2 == 1) {
            os << _("Program AE");
        } else if (l0 == 2 && l1 == 1 && l2 == 1) {
            os << _("Hi-speed Program");
        } else if (l0 == 2 && l1 == 2 && l2 == 1) {
            os << _("DOF Program");
        } else if (l0 == 2 && l1 == 3 && l2 == 1) {
            os << _("MTF Program");
        } else if (l0 == 3 && l1 == 0 && l2 == 1) {
            os << _("Green Mode");
        } else if (l0 == 4 && l1 == 0 && l2 == 1) {
            os << _("Shutter Speed Priority");
        } else if (l0 == 5 && l1 == 0 && l2 == 1) {
            os << _("Aperture Priority");
        } else if (l0 == 6 && l1 == 0 && l2 == 1) {
            os << _("Program Tv Shift");
        } else if (l0 == 7 && l1 == 0 && l2 == 1) {
            os << _("Program Av Shift");
        } else if (l0 == 8 && l1 == 0 && l2 == 1) {
            os << _("Manual");
        } else if (l0 == 9 && l1 == 0 && l2 == 1) {
            os << _("Bulb");
        } else if (l0 == 10 && l1 == 0 && l2 == 1) {
            os << _("Aperture Priority (Off-Auto-Aperture)");
        } else if (l0 == 11 && l1 == 0 && l2 == 1) {
            os << _("Manual (Off-Auto-Aperture)");
        } else if (l0 == 12 && l1 == 0 && l2 == 1) {
            os << _("Bulb (Off-Auto-Aperture)");
        /* K10D modes */
        } else if (l0 == 6 && l1 == 0 && l2 == 0) {
            os << _("Shutter Priority");
        } else if (l0 == 13 && l1 == 0 && l2 == 0) {
            os << _("Shutter & Aperture Priority AE");
        } else if (l0 == 13 && l1 == 0 && l2 == 1) {
            os << _("Shutter & Aperture Priority AE (1)");
        } else if (l0 == 15 && l1 == 0 && l2 == 0) {
            os << _("Sensitivity Priority AE");
        } else if (l0 == 15 && l1 == 0 && l2 == 1) {
            os << _("Sensitivity Priority AE (1)");
        } else if (l0 == 16 && l1 == 0 && l2 == 0) {
            os << _("Flash X-Sync Speed AE");
        } else if (l0 == 16 && l1 == 0 && l2 == 1) {
            os << _("Flash X-Sync Speed AE (1)");
        /* other modes */
        } else if (l0 == 0 && l1 == 0 && l2 == 1) {
            os << _("Program");
        } else {
            os << _("Unknown") << " (" << l0 << " " << l1 << " " << l2 << ")";
        }

        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxDriveMode(std::ostream& os, const Value& value)
    {
        long l0 = value.toLong(0);
        long l1 = value.toLong(1);
        long l2 = value.toLong(2);
        long l3 = value.toLong(3);

        if (l0 == 0 && l1 == 0 && l2 == 0 && l3 == 0) {
            os << _("Single-frame");
        } else if (l0 == 1 && l1 == 0 && l2 == 0 && l3 == 0) {
            os << _("Continuous");
        } else if (l0 == 0 && l1 == 1 && l2 == 0 && l3 == 0) {
            os << _("Self-timer (12 sec)");
        } else if (l0 == 0 && l1 == 2 && l2 == 0 && l3 == 0) {
            os << _("Self-timer (2 sec)");
        } else if (l0 == 0 && l1 == 0 && l2 == 1 && l3 == 0) {
            os << _("Remote Control?");
        } else if (l0 == 0 && l1 == 0 && l2 == 0 && l3 == 1) {
            os << _("Multiple Exposure");
        } else {
            os << _("Unknown") << " (" << l0 << " " << l1 << " " << l2 << " " << l3 << ")";
        }

        return os;
    }

    std::ostream& PentaxMakerNote::printPentaxLensType(std::ostream& os, const Value& value)
    {
        long l0 = value.toLong(0);
        long l1 = value.toLong(1);

        switch(l0) {
            case 0:
                switch (l1) {
                    case 0:
                        os << "M-42 or No Lens";
                        break;
                    default:
                        os << _("Unknown") << " (" << l0 << " " << l1 << ")";
                        break;
                }
            case 1:
                switch (l1) {
                    case 0:
                        os << "K,M Lens";
                        break;
                    default:
                        os << _("Unknown") << " (" << l0 << " " << l1 << ")";
                        break;
                }
            case 2:
                switch (l1) {
                    case 0:
                        os << "A Series Lens";
                        break;
                    default:
                        os << _("Unknown") << " (" << l0 << " " << l1 << ")";
                        break;
                }
            case 3:
                switch (l1) {
                    case 0:
                        os << "SIGMA";
                        break;
                    case 17:
                        os << "smc PENTAX-FA SOFT 85mm F2.8";
                        break;
                    case 18:
                        os << "smc PENTAX-F 1.7X AF ADAPTER";
                        break;
                    case 19:
                        os << "smc PENTAX-F 24-50mm F4";
                        break;
                    case 20:
                        os << "smc PENTAX-F 35-80mm F4-5.6";
                        break;
                    case 21:
                        os << "smc PENTAX-F 80-200mm F4.7-5.6";
                        break;
                    case 22:
                        os << "smc PENTAX-F FISH-EYE 17-28mm F3.5-4.5";
                        break;
                    case 23:
                        os << "smc PENTAX-F 100-300mm F4.5-5.6";
                        break;
                    case 24:
                        os << "smc PENTAX-F 35-135mm F3.5-4.5";
                        break;
                    case 25:
                        os << "smc PENTAX-F 35-105mm F4-5.6 or SIGMA or Tokina";
                        break;
                    case 26:
                        os << "smc PENTAX-F* 250-600mm F5.6 ED[IF]";
                        break;
                    case 27:
                        os << "smc PENTAX-F 28-80mm F3.5-4.5";
                        break;
                    case 28:
                        os << "smc PENTAX-F 35-70mm F3.5-4.5";
                        break;
                    case 29:
                        os << "PENTAX-F 28-80mm F3.5-4.5 or SIGMA AF 18-125mm F3.5-5.6 DC";
                        break;
                    case 30:
                        os << "PENTAX-F 70-200mm F4-5.6";
                        break;
                    case 31:
                        os << "smc PENTAX-F 70-210mm F4-5.6";
                        break;
                    case 32:
                        os << "smc PENTAX-F 50mm F1.4";
                        break;
                    case 33:
                        os << "smc PENTAX-F 50mm F1.7";
                        break;
                    case 34:
                        os << "smc PENTAX-F 135mm F2.8 [IF]";
                        break;
                    case 35:
                        os << "smc PENTAX-F 28mm F2.8";
                        break;
                    case 36:
                        os << "SIGMA 20mm F1.8 EX DG ASPHERICAL RF";
                        break;
                    case 38:
                        os << "smc PENTAX-F* 300mm F4.5 ED[IF]";
                        break;
                    case 39:
                        os << "smc PENTAX-F* 600mm F4 ED[IF]";
                        break;
                    case 40:
                        os << "smc PENTAX-F MACRO 100mm F2.8";
                        break;
                    case 41:
                        os << "smc PENTAX-F MACRO 50mm F2.8 or Sigma 50mm F2,8 MACRO";
                        break;
                    case 44:
                        os << "Tamron 35-90mm F4 AF or various SIGMA models";
                        break;
                    case 46:
                        os << "SIGMA APO 70-200mm F2.8 EX";
                        break;
                    case 50:
                        os << "smc PENTAX-FA 28-70mm F4 AL";
                        break;
                    case 51:
                        os << "SIGMA 28mm F1.8 EX DG ASPHERICAL MACRO";
                        break;
                    case 52:
                        os << "smc PENTAX-FA 28-200mm F3.8-5.6 AL[IF]";
                        break;
                    case 53:
                        os << "smc PENTAX-FA 28-80mm F3.5-5.6 AL";
                        break;
                    case 247:
                        os << "smc PENTAX-DA FISH-EYE 10-17mm F3.5-4.5 ED[IF]";
                        break;
                    case 248:
                        os << "smc PENTAX-DA 12-24mm F4 ED AL[IF]";
                        break;
                    case 250:
                        os << "smc PENTAX-DA 50-200mm F4-5.6 ED";
                        break;
                    case 251:
                        os << "smc PENTAX-DA 40mm F2.8 Limited";
                        break;
                    case 252:
                        os << "smc PENTAX-DA 18-55mm F3.5-5.6 AL";
                        break;
                    case 253:
                        os << "smc PENTAX-DA 14mm F2.8 ED[IF]";
                        break;
                    case 254:
                        os << "smc PENTAX-DA 16-45mm F4 ED AL";
                        break;
                    case 255:
                        os << "SIGMA";
                        break;
                    default:
                        os << _("Unknown") << " (" << l0 << " " << l1 << ")";
                        break;
                }
                break;
            case 4:
                switch (l1) {
                    case 1:
                        os << "smc PENTAX-FA SOFT 28mm F2.8";
                        break;
                    case 2:
                        os << "smc PENTAX-FA 80-320mm F4.5-5.6";
                        break;
                    case 3:
                        os << "smc PENTAX-FA 43mm F1.9 Limited";
                        break;
                    case 6:
                        os << "smc PENTAX-FA 35-80mm F4-5.6";
                        break;
                    case 12:
                        os << "smc PENTAX-FA 50mm F1.4";
                        break;
                    case 15:
                        os << "smc PENTAX-FA 28-105mm F4-5.6 [IF]";
                        break;
                    case 16:
                        os << "TAMRON AF 80-210mm F4-5.6 (178D)";
                        break;
                    case 19:
                        os << "TAMRON SP AF 90mm F2.8 (172E)";
                        break;
                    case 20:
                        os << "smc PENTAX-FA 28-80mm F3.5-5.6";
                        break;
                    case 22:
                        os << "TOKINA 28-80mm F3.5-5.6";
                        break;
                    case 23:
                        os << "smc PENTAX-FA 20-35mm F4 AL";
                        break;
                    case 24:
                        os << "smc PENTAX-FA 77mm F1.8 Limited";
                        break;
                    case 25:
                        os << "TAMRON SP AF 14mm F2.8";
                        break;
                    case 26:
                        os << "smc PENTAX-FA MACRO 100mm F3.5";
                        break;
                    case 27:
                        os << "TAMRON AF28-300mm F/3.5-6.3 LD Aspherical[IF] MACRO (285D)";
                        break;
                    case 28:
                        os << "smc PENTAX-FA 35mm F2 AL";
                        break;
                    case 29:
                        os << "TAMRON AF 28-200mm F/3.8-5.6 LD Super II MACRO (371D)";
                        break;
                    case 34:
                        os << "smc PENTAX-FA 24-90mm F3.5-4.5 AL[IF]";
                        break;
                    case 35:
                        os << "smc PENTAX-FA 100-300mm F4.7-5.8";
                        break;
                    case 36:
                        os << "TAMRON AF70-300mm F/4-5.6 LD MACRO";
                        break;
                    case 37:
                        os << "TAMRON SP AF 24-135mm F3.5-5.6 AD AL (190D)";
                        break;
                    case 38:
                        os << "smc PENTAX-FA 28-105mm F3.2-4.5 AL[IF]";
                        break;
                    case 39:
                        os << "smc PENTAX-FA 31mm F1.8AL Limited";
                        break;
                    case 41:
                        os << "TAMRON AF 28-200mm Super Zoom F3.8-5.6 Aspherical XR [IF] MACRO (A03)";
                        break;
                    case 43:
                        os << "smc PENTAX-FA 28-90mm F3.5-5.6";
                        break;
                    case 44:
                        os << "smc PENTAX-FA J 75-300mm F4.5-5.8 AL";
                        break;
                    case 45:
                        os << "TAMRON 28-300mm F3.5-6.3 Ultra zoom XR";
                        break;
                    case 46:
                        os << "smc PENTAX-FA J 28-80mm F3.5-5.6 AL";
                        break;
                    case 47:
                        os << "smc PENTAX-FA J 18-35mm F4-5.6 AL";
                        break;
                    case 49:
                        os << "TAMRON SP AF 28-75mm F2.8 XR Di (A09)";
                        break;
                    case 51:
                        os << "smc PENTAX-D FA 50mm F2.8 MACRO";
                        break;
                    case 52:
                        os << "smc PENTAX-D FA 100mm F2.8 MACRO";
                        break;
                    case 244:
                        os << "smc PENTAX-DA 21mm F3.2 AL Limited";
                        break;
                    case 245:
                        os << "Schneider D-XENON 50-200mm";
                        break;
                    case 246:
                        os << "Schneider D-XENON 18-55mm";
                        break;
                    case 247:
                        os << "smc PENTAX-DA 10-17mm F3.5-4.5 ED [IF] Fisheye zoom";
                        break;
                    case 248:
                        os << "smc PENTAX-DA 12-24mm F4 ED AL [IF]";
                        break;
                    case 249:
                        os << "TAMRON XR DiII 18-200mm F3.5-6.3 (A14)";
                        break;
                    case 250:
                        os << "smc PENTAX-DA 50-200mm F4-5.6 ED";
                        break;
                    case 251:
                        os << "smc PENTAX-DA 40mm F2.8 Limited";
                        break;
                    case 252:
                        os << "smc PENTAX-DA 18-55mm F3.5-5.6 AL";
                        break;
                    case 253:
                        os << "smc PENTAX-DA 14mm F2.8 ED[IF]";
                        break;
                    case 254:
                        os << "smc PENTAX-DA 16-45mm F4 ED AL";
                        break;
                    default:
                        os << _("Unknown") << " (" << l0 << " " << l1 << ")";
                        break;
                }
                break;
            case 5:
                switch (l1) {
                    case 1:
                        os << "smc PENTAX-FA* 24mm F2 AL[IF]";
                        break;
                    case 2:
                        os << "smc PENTAX-FA 28mm F2.8 AL";
                        break;
                    case 3:
                        os << "smc PENTAX-FA 50mm F1.7";
                        break;
                    case 4:
                        os << "smc PENTAX-FA 50mm F1.4";
                        break;
                    case 5:
                        os << "smc PENTAX-FA* 600mm F4 ED[IF]";
                        break;
                    case 6:
                        os << "smc PENTAX-FA* 300mm F4.5 ED[IF]";
                        break;
                    case 7:
                        os << "smc PENTAX-FA 135mm F2.8 [IF]";
                        break;
                    case 8:
                        os << "smc PENTAX-FA MACRO 50mm F2.8";
                        break;
                    case 9:
                        os << "smc PENTAX-FA MACRO 100mm F2.8";
                        break;
                    case 10:
                        os << "smc PENTAX-FA* 85mm F1.4 [IF]";
                        break;
                    case 11:
                        os << "smc PENTAX-FA* 200mm F2.8 ED[IF]";
                        break;
                    case 12:
                        os << "smc PENTAX-FA 28-80mm F3.5-4.7";
                        break;
                    case 13:
                        os << "smc PENTAX-FA 70-200mm F4-5.6";
                        break;
                    case 14:
                        os << "smc PENTAX-FA* 250-600mm F5.6 ED[IF]";
                        break;
                    case 15:
                        os << "smc PENTAX-FA 28-105mm F4-5.6";
                        break;
                    case 16:
                        os << "smc PENTAX-FA 100-300mm F4.5-5.6";
                        break;
                    default:
                        os << _("Unknown") << " (" << l0 << " " << l1 << ")";
                        break;
                }
                break;
            case 6:
                switch (l1) {
                    case 1:
                        os << "smc PENTAX-FA* 85mm F1.4[IF]";
                        break;
                    case 2:
                        os << "smc PENTAX-FA* 200mm F2.8 ED[IF]";
                        break;
                    case 3:
                        os << "smc PENTAX-FA* 300mm F2.8 ED[IF]";
                        break;
                    case 4:
                        os << "smc PENTAX-FA* 28-70mm F2.8 AL";
                        break;
                    case 5:
                        os << "smc PENTAX-FA* 80-200mm F2.8 ED[IF]";
                        break;
                    case 6:
                        os << "smc PENTAX-FA* 28-70mm F2.8 AL";
                        break;
                    case 7:
                        os << "smc PENTAX-FA* 80-200mm F2.8 ED[IF]";
                        break;
                    case 8:
                        os << "smc PENTAX-FA 28-70mm F4AL";
                        break;
                    case 9:
                        os << "smc PENTAX-FA 20mm F2.8";
                        break;
                    case 10:
                        os << "smc PENTAX-FA* 400mm F5.6 ED[IF]";
                        break;
                    case 13:
                        os << "smc PENTAX-FA* 400mm F5.6 ED[IF]";
                        break;
                    case 14:
                        os << "smc PENTAX-FA* MACRO 200mm F4 ED[IF]";
                        break;
                    default:
                        os << _("Unknown") << " (" << l0 << " " << l1 << ")";
                        break;
                }
                break;
            case 7:
                switch (l1) {
                    case 0:
                        os << "smc PENTAX-DA 21mm F3.2 AL Limited";
                        break;
                    case 238:
                        os << "TAMRON AF 18-250mm F3.5-6.3 Di II LD Aspherical [IF] MACRO";
                        break;
                    case 243:
                        os << "smc PENTAX-DA 70mm F2.4 Limited";
                        break;
                    case 244:
                        os << "smc PENTAX-DA 21mm F3.2 AL Limited";
                        break;
                    default:
                        os << _("Unknown") << " (" << l0 << " " << l1 << ")";
                        break;
                }
                break;
            case 8:
                switch (l1) {
                    case 241:
                        os << "smc PENTAX-DA* 50-135mm F2.8 ED [IF] SDM";
                        break;
                    case 242:
                        os << "smc PENTAX-DA* 16-50mm F2.8 ED AL [IF] SDM";
                        break;
                    default:
                        os << _("Unknown") << " (" << l0 << " " << l1 << ")";
                        break;
                }
                break;
            default:
                os << _("Unknown") << " (" << l0 << " " << l1 << ")";
                break;
        }
        return os;
    }

    // Pentax MakerNote Tag Info
    const TagInfo PentaxMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "Version", N_("Version"),
                N_("Pentax Makernote version"),
                pentaxIfdId, makerTags, undefined, printPentaxVersion),
        TagInfo(0x0001, "Mode", N_("Shooting mode"),
                N_("Camera shooting mode"),
                pentaxIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(pentaxShootingMode)),
        TagInfo(0x0002, "PreviewResolution", N_("Resolution of a preview image"),
                N_("Resolution of a preview image"),
                pentaxIfdId, makerTags, undefined, printPentaxResolution),
        TagInfo(0x0003, "PreviewLength", N_("Length of a preview image"),
                N_("Size of an IFD containing a preview image"),
                pentaxIfdId, makerTags, undefined, printValue),
        TagInfo(0x0004, "PreviewOffset", N_("Pointer to a preview image"),
                N_("Offset to an IFD containing a preview image"),
                pentaxIfdId, makerTags, undefined, printValue),
        TagInfo(0x0005, "ModelID", N_("Model identification"),
                N_("Pentax model idenfication"),
                pentaxIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(pentaxModel)),
        TagInfo(0x0006, "Date", N_("Date"),
                N_("Date"),
                pentaxIfdId, makerTags, undefined, printPentaxDate),
        TagInfo(0x0007, "Time", N_("Time"),
                N_("Time"),
                pentaxIfdId, makerTags, undefined, printPentaxTime),
        TagInfo(0x0008, "Quality", N_("Image quality"),
                N_("Image quality settings"),
                pentaxIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(pentaxQuality)),
        TagInfo(0x0009, "Size", N_("Image size"),
                N_("Image size settings"),
                pentaxIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(pentaxSize)),
        /* Some missing ! */
        TagInfo(0x000c, "Flash", N_("Flash mode"),
                N_("Flash mode settings"),
                pentaxIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(pentaxFlash)),
        TagInfo(0x000d, "Focus", N_("Focus mode"),
                N_("Focus mode settings"),
                pentaxIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(pentaxFocus)),
        TagInfo(0x000e, "AFPoint", N_("AF point"),
                N_("Selected AF point"),
                pentaxIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(pentaxAFPoint)),
        /* Some missing ! */
        TagInfo(0x0012, "ExposureTime", N_("Exposure time"),
                N_("Exposure time"),
                pentaxIfdId, makerTags, unsignedLong, printPentaxExposure),
        TagInfo(0x0013, "FNumber", N_("F-Number"),
                N_("F-Number"),
                pentaxIfdId, makerTags, unsignedLong, printPentaxFValue),
        TagInfo(0x0014, "ISO", N_("ISO sensitivity"),
                N_("ISO sensitivity settings"),
                pentaxIfdId, makerTags, unsignedLong, EXV_PRINT_TAG(pentaxISO)),
        /* Some missing ! */
        TagInfo(0x0016, "ExposureCompensation", N_("Exposure compensation"),
                N_("Exposure compensation"),
                pentaxIfdId, makerTags, unsignedLong, printPentaxCompensation),

        /* Some missing ! */
        TagInfo(0x0017, "MeteringMode", N_("MeteringMode"),
                N_("MeteringMode"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxMeteringMode)),
        TagInfo(0x0018, "AutoBracketing", N_("AutoBracketing"),
                N_("AutoBracketing"),
                pentaxIfdId, makerTags, undefined, printPentaxBracketing),
        TagInfo(0x0019, "WhiteBallance", N_("White ballance"),
                N_("White ballance"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxWhiteBallance)),
        TagInfo(0x001a, "WhiteBallanceMode", N_("White ballance mode"),
                N_("White ballance mode"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxWhiteBallanceMode)),
        /* Some missing ! */
        TagInfo(0x001d, "FocalLength", N_("FocalLength"),
                N_("FocalLength"),
                pentaxIfdId, makerTags, undefined, printPentaxFocalLength),
        /* Some missing ! */
        TagInfo(0x001f, "Saturation", N_("Saturation"),
                N_("Saturation"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxSaturation)),
        TagInfo(0x0020, "Contrast", N_("Contrast"),
                N_("Contrast"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxContrast)),
        TagInfo(0x0021, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxSharpness)),
        TagInfo(0x0022, "Location", N_("Location"),
                N_("Location"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxLocation)),
        TagInfo(0x0023, "Hometown", N_("Hometown"),
                N_("Home town"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxCities)),
        TagInfo(0x0024, "Destination", N_("Destination"),
                N_("Destination"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxCities)),
        TagInfo(0x0025, "HometownDST", N_("Hometown DST"),
                N_("Whether day saving time is active in home town"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxYesNo)),
        TagInfo(0x0026, "DestinationDST", N_("Destination DST"),
                N_("Whether day saving time is active in destination"),
                pentaxIfdId, makerTags, undefined, EXV_PRINT_TAG(pentaxYesNo)),
        /* Some missing ! */
        TagInfo(0x0029, "FrameNumber", N_("Frame number"),
                N_("Frame number"),
                pentaxIfdId, makerTags, undefined, printValue),
        /* Some missing ! */
        TagInfo(0x0032, "ImageProcessing", N_("Image processing"),
                N_("Image processing"),
                pentaxIfdId, makerTags, undefined, printPentaxImageProcessing),
        TagInfo(0x0033, "PictureMode", N_("Picture mode"),
                N_("Picture mode"),
                pentaxIfdId, makerTags, undefined, printPentaxPictureMode),
        TagInfo(0x0034, "DriveMode", N_("Drive mode"),
                N_("Drive mode"),
                pentaxIfdId, makerTags, undefined, printPentaxDriveMode),
        /* Some missing ! */
        TagInfo(0x0037, "ColorSpace", N_("Color space"),
                N_("Color space"),
                pentaxIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(pentaxColorSpace)),
        /* Some missing ! */
        TagInfo(0x003f, "LensType", N_("Lens type"),
                N_("Lens type"),
                pentaxIfdId, makerTags, unsignedLong, printPentaxLensType),
        /* Some missing ! */
        TagInfo(0x0041, "DigitalFilter", N_("Digital filter"),
                N_("Digital filter"),
                pentaxIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(pentaxOffOn)),
        /* Some missing ! */
        TagInfo(0x0047, "Temperature", N_("Temperature"),
                N_("Camera temperature"),
                pentaxIfdId, makerTags, signedByte, printPentaxTemperature),
        /* Some missing ! */
        TagInfo(0x0049, "NoiseReduction", N_("Noise reduction"),
                N_("Noise reduction"),
                pentaxIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(pentaxOffOn)),
        /* Some missing ! */
        TagInfo(0x004d, "FlashExposureCompensation", N_("Flash exposure compensation"),
                N_("Flash exposure compensation"),
                pentaxIfdId, makerTags, signedLong, printPentaxFlashCompensation),
        /* Some missing ! */
        TagInfo(0x004f, "ImageTone", N_("Image tone"),
                N_("Image tone"),
                pentaxIfdId, makerTags, unsignedShort, EXV_PRINT_TAG(pentaxImageTone)),
        /* Some missing ! */
        TagInfo(0x005c, "ShakeReduction", N_("Shake reduction"),
                N_("Shake reduction information"),
                pentaxIfdId, makerTags, undefined, printValue),
        TagInfo(0x005d, "ShutterCount", N_("Shutter count"),
                N_("Shutter count"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: This has some encryption by date (see exiftool) */

        /* Many missing ! */
        TagInfo(0x0200, "BlackPoint", N_("Black point"),
                N_("Black point"),
                pentaxIfdId, makerTags, undefined, printValue),
        TagInfo(0x0201, "WhitePoint", N_("White point"),
                N_("White point"),
                pentaxIfdId, makerTags, undefined, printValue),
        /* Some missing ! */
        TagInfo(0x0206, "AEInfo", N_("AEInfo"),
                N_("AEInfo"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0207, "LensInfo", N_("LensInfo"),
                N_("LensInfo"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0208, "FlashInfo", N_("FlashInfo"),
                N_("FlashInfo"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0209, "AEMeteringSegments", N_("AEMeteringSegments"),
                N_("AEMeteringSegments"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x020a, "FlashADump", N_("FlashADump"),
                N_("FlashADump"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x020b, "FlashBDump", N_("FlashBDump"),
                N_("FlashBDump"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        /* Some missing ! */
        TagInfo(0x020d, "WB_RGGBLevelsDaylight", N_("WB_RGGBLevelsDaylight"),
                N_("WB_RGGBLevelsDaylight"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x020e, "WB_RGGBLevelsShade", N_("WB_RGGBLevelsShade"),
                N_("WB_RGGBLevelsShade"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x020f, "WB_RGGBLevelsCloudy", N_("WB_RGGBLevelsCloudy"),
                N_("WB_RGGBLevelsCloudy"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0210, "WB_RGGBLevelsTungsten", N_("WB_RGGBLevelsTungsten"),
                N_("WB_RGGBLevelsTungsten"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0211, "WB_RGGBLevelsFluorescentD", N_("WB_RGGBLevelsFluorescentD"),
                N_("WB_RGGBLevelsFluorescentD"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0212, "WB_RGGBLevelsFluorescentN", N_("WB_RGGBLevelsFluorescentN"),
                N_("WB_RGGBLevelsFluorescentN"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0213, "WB_RGGBLevelsFluorescentW", N_("WB_RGGBLevelsFluorescentW"),
                N_("WB_RGGBLevelsFluorescentW"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0214, "WB_RGGBLevelsFlash", N_("WB_RGGBLevelsFlash"),
                N_("WB_RGGBLevelsFlash"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0215, "CameraInfo", N_("CameraInfo"),
                N_("CameraInfo"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0216, "BatteryInfo", N_("BatteryInfo"),
                N_("BatteryInfo"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x021f, "AFInfo", N_("AFInfo"),
                N_("AFInfo"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */
        TagInfo(0x0206, "Saturation", N_("Saturation"),
                N_("Saturation"),
                pentaxIfdId, makerTags, undefined, printValue), /* TODO: Decoding missing */

        // End of list marker
        TagInfo(0xffff, "(UnknownPentaxMakerNoteTag)", "(UnknownPentaxMakerNoteTag)",
                N_("Unknown PentaxMakerNote tag"),
                pentaxIfdId, makerTags, invalidTypeId, printValue)
    };

    const TagInfo* PentaxMakerNote::tagList()
    {
        return tagInfo_;
    }

    PentaxMakerNote::PentaxMakerNote(bool alloc)
        : IfdMakerNote(pentaxIfdId, alloc)
    {
        byte buf[] = {
            'A', 'O', 'C', 0x00, 'M', 'M'
        };
        readHeader(buf, 6, byteOrder_);
    }

    PentaxMakerNote::PentaxMakerNote(const PentaxMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int PentaxMakerNote::readHeader(const byte* buf,
                                  long len,
                                  ByteOrder /*byteOrder*/)
    {
        if (len < 6) return 1;

        header_.alloc(6);
        std::memcpy(header_.pData_, buf, header_.size_);
        start_ = 6;
        return 0;
    }

    int PentaxMakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the AOC prefix
        if (   header_.size_ < 6
            || std::string(reinterpret_cast<char*>(header_.pData_), 3)
                    != std::string("AOC", 3)) {
            rc = 2;
        }
        return rc;
    }

    PentaxMakerNote::AutoPtr PentaxMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    PentaxMakerNote* PentaxMakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote(new PentaxMakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    PentaxMakerNote::AutoPtr PentaxMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    PentaxMakerNote* PentaxMakerNote::clone_() const
    {
        return new PentaxMakerNote(*this);
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createPentaxMakerNote(bool        alloc,
                                           const byte* /*buf*/,
                                           long        /*len*/,
                                           ByteOrder   /*byteOrder*/,
                                           long        /*offset*/)
    {
        return MakerNote::AutoPtr(new PentaxMakerNote(alloc));
    }

}                                       // namespace Exiv2
