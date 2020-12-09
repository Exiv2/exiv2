// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
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
  File:      canonmn.cpp
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             David Cannings (dc) <david@edeca.net>
             Andi Clemens (ac) <andi.clemens@gmx.net>
 */
// *****************************************************************************
// included header files
#include "types.hpp"
#include "makernote_int.hpp"
#include "canonmn_int.hpp"
#include "tags_int.hpp"
#include "value.hpp"
#include "exif.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <ios>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    //! OffOn, multiple tags
    constexpr TagDetails canonOffOn[] = {
        {  0, N_("Off") },
        {  1, N_("On")  }
    };

    //! Special treatment pretty-print function for non-unique lens ids.
    std::ostream& printCsLensByFocalLengthAndMaxAperture(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata);
    //! Special treatment pretty-print function for non-unique lens ids.
    std::ostream& printCsLensByFocalLength(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata);
    //! Special treatment pretty-print function for non-unique lens ids.
    std::ostream& printCsLensByFocalLengthTC(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata);
    //! Special treatment pretty-print function for non-unique lens ids.
    std::ostream& printCsLensFFFF(std::ostream& os,
                                  const Value& value,
                                  const ExifData* metadata);

    //! ModelId, tag 0x0010
    constexpr TagDetails canonModelId[] = {
        { static_cast<long int>(0x00000811), "EOS M6 Mark II"}, 
        { static_cast<long int>(0x00000804), "Powershot G5 X Mark II"}, 
        { static_cast<long int>(0x00000805), "PowerShot SX70 HS"}, 
        { static_cast<long int>(0x00000808), "PowerShot G7 X Mark III"},               
        { static_cast<long int>(0x00000812), "EOS M200"}, 
        { static_cast<long int>(0x1010000), "PowerShot A30" },
        { static_cast<long int>(0x1040000), "PowerShot S300 / Digital IXUS 300 / IXY Digital 300" },
        { static_cast<long int>(0x1060000), "PowerShot A20" },
        { static_cast<long int>(0x1080000), "PowerShot A10" },
        { static_cast<long int>(0x1090000), "PowerShot S110 / Digital IXUS v / IXY Digital 200" },
        { static_cast<long int>(0x1100000), "PowerShot G2" },
        { static_cast<long int>(0x1110000), "PowerShot S40" },
        { static_cast<long int>(0x1120000), "PowerShot S30" },
        { static_cast<long int>(0x1130000), "PowerShot A40" },
        { static_cast<long int>(0x1140000), "EOS D30" },
        { static_cast<long int>(0x1150000), "PowerShot A100" },
        { static_cast<long int>(0x1160000), "PowerShot S200 / Digital IXUS v2 / IXY Digital 200a" },
        { static_cast<long int>(0x1170000), "PowerShot A200" },
        { static_cast<long int>(0x1180000), "PowerShot S330 / Digital IXUS 330 / IXY Digital 300a" },
        { static_cast<long int>(0x1190000), "PowerShot G3" },
        { static_cast<long int>(0x1210000), "PowerShot S45" },
        { static_cast<long int>(0x1230000), "PowerShot SD100 / Digital IXUS II / IXY Digital 30" },
        { static_cast<long int>(0x1240000), "PowerShot S230 / Digital IXUS v3 / IXY Digital 320" },
        { static_cast<long int>(0x1250000), "PowerShot A70" },
        { static_cast<long int>(0x1260000), "PowerShot A60" },
        { static_cast<long int>(0x1270000), "PowerShot S400 / Digital IXUS 400 / IXY Digital 400" },
        { static_cast<long int>(0x1290000), "PowerShot G5" },
        { static_cast<long int>(0x1300000), "PowerShot A300" },
        { static_cast<long int>(0x1310000), "PowerShot S50" },
        { static_cast<long int>(0x1340000), "PowerShot A80" },
        { static_cast<long int>(0x1350000), "PowerShot SD10 / Digital IXUS i / IXY Digital L" },
        { static_cast<long int>(0x1360000), "PowerShot S1 IS" },
        { static_cast<long int>(0x1370000), "PowerShot Pro1" },
        { static_cast<long int>(0x1380000), "PowerShot S70" },
        { static_cast<long int>(0x1390000), "PowerShot S60" },
        { static_cast<long int>(0x1400000), "PowerShot G6" },
        { static_cast<long int>(0x1410000), "PowerShot S500 / Digital IXUS 500 / IXY Digital 500" },
        { static_cast<long int>(0x1420000), "PowerShot A75" },
        { static_cast<long int>(0x1440000), "PowerShot SD110 / Digital IXUS IIs / IXY Digital 30a" },
        { static_cast<long int>(0x1450000), "PowerShot A400" },
        { static_cast<long int>(0x1470000), "PowerShot A310" },
        { static_cast<long int>(0x1490000), "PowerShot A85" },
        { static_cast<long int>(0x1520000), "PowerShot S410 / Digital IXUS 430 / IXY Digital 450" },
        { static_cast<long int>(0x1530000), "PowerShot A95" },
        { static_cast<long int>(0x1540000), "PowerShot SD300 / Digital IXUS 40 / IXY Digital 50" },
        { static_cast<long int>(0x1550000), "PowerShot SD200 / Digital IXUS 30 / IXY Digital 40" },
        { static_cast<long int>(0x1560000), "PowerShot A520" },
        { static_cast<long int>(0x1570000), "PowerShot A510" },
        { static_cast<long int>(0x1590000), "PowerShot SD20 / Digital IXUS i5 / IXY Digital L2" },
        { static_cast<long int>(0x1640000), "PowerShot S2 IS" },
        { static_cast<long int>(0x1650000), "PowerShot SD430 / Digital IXUS Wireless / IXY Digital Wireless" },
        { static_cast<long int>(0x1660000), "PowerShot SD500 / Digital IXUS 700 / IXY Digital 600" },
        { static_cast<long int>(0x1668000), "EOS D60" },
        { static_cast<long int>(0x1700000), "PowerShot SD30 / Digital IXUS i Zoom / IXY Digital L3" },
        { static_cast<long int>(0x1740000), "PowerShot A430" },
        { static_cast<long int>(0x1750000), "PowerShot A410" },
        { static_cast<long int>(0x1760000), "PowerShot S80" },
        { static_cast<long int>(0x1780000), "PowerShot A620" },
        { static_cast<long int>(0x1790000), "PowerShot A610" },
        { static_cast<long int>(0x1800000), "PowerShot SD630 / Digital IXUS 65 / IXY Digital 80" },
        { static_cast<long int>(0x1810000), "PowerShot SD450 / Digital IXUS 55 / IXY Digital 60" },
        { static_cast<long int>(0x1820000), "PowerShot TX1" },
        { static_cast<long int>(0x1870000), "PowerShot SD400 / Digital IXUS 50 / IXY Digital 55" },
        { static_cast<long int>(0x1880000), "PowerShot A420" },
        { static_cast<long int>(0x1890000), "PowerShot SD900 / Digital IXUS 900 Ti / IXY Digital 1000" },
        { static_cast<long int>(0x1900000), "PowerShot SD550 / Digital IXUS 750 / IXY Digital 700" },
        { static_cast<long int>(0x1920000), "PowerShot A700" },
        { static_cast<long int>(0x1940000), "PowerShot SD700 IS / Digital IXUS 800 IS / IXY Digital 800 IS" },
        { static_cast<long int>(0x1950000), "PowerShot S3 IS" },
        { static_cast<long int>(0x1960000), "PowerShot A540" },
        { static_cast<long int>(0x1970000), "PowerShot SD600 / Digital IXUS 60 / IXY Digital 70" },
        { static_cast<long int>(0x1980000), "PowerShot G7" },
        { static_cast<long int>(0x1990000), "PowerShot A530" },
        { static_cast<long int>(0x2000000), "PowerShot SD800 IS / Digital IXUS 850 IS / IXY Digital 900 IS" },
        { static_cast<long int>(0x2010000), "PowerShot SD40 / Digital IXUS i7 / IXY Digital L4" },
        { static_cast<long int>(0x2020000), "PowerShot A710 IS" },
        { static_cast<long int>(0x2030000), "PowerShot A640" },
        { static_cast<long int>(0x2040000), "PowerShot A630" },
        { static_cast<long int>(0x2090000), "PowerShot S5 IS" },
        { static_cast<long int>(0x2100000), "PowerShot A460" },
        { static_cast<long int>(0x2120000), "PowerShot SD850 IS / Digital IXUS 950 IS / IXY Digital 810 IS" },
        { static_cast<long int>(0x2130000), "PowerShot A570 IS" },
        { static_cast<long int>(0x2140000), "PowerShot A560" },
        { static_cast<long int>(0x2150000), "PowerShot SD750 / Digital IXUS 75 / IXY Digital 90" },
        { static_cast<long int>(0x2160000), "PowerShot SD1000 / Digital IXUS 70 / IXY Digital 10" },
        { static_cast<long int>(0x2180000), "PowerShot A550" },
        { static_cast<long int>(0x2190000), "PowerShot A450" },
        { static_cast<long int>(0x2230000), "PowerShot G9" },
        { static_cast<long int>(0x2240000), "PowerShot A650 IS" },
        { static_cast<long int>(0x2260000), "PowerShot A720 IS" },
        { static_cast<long int>(0x2290000), "PowerShot SX100 IS" },
        { static_cast<long int>(0x2300000), "PowerShot SD950 IS / Digital IXUS 960 IS / IXY Digital 2000 IS" },
        { static_cast<long int>(0x2310000), "PowerShot SD870 IS / Digital IXUS 860 IS / IXY Digital 910 IS" },
        { static_cast<long int>(0x2320000), "PowerShot SD890 IS / Digital IXUS 970 IS / IXY Digital 820 IS" },
        { static_cast<long int>(0x2360000), "PowerShot SD790 IS / Digital IXUS 90 IS / IXY Digital 95 IS" },
        { static_cast<long int>(0x2370000), "PowerShot SD770 IS / Digital IXUS 85 IS / IXY Digital 25 IS" },
        { static_cast<long int>(0x2380000), "PowerShot A590 IS" },
        { static_cast<long int>(0x2390000), "PowerShot A580" },
        { static_cast<long int>(0x2420000), "PowerShot A470" },
        { static_cast<long int>(0x2430000), "PowerShot SD1100 IS / Digital IXUS 80 IS / IXY Digital 20 IS" },
        { static_cast<long int>(0x2460000), "PowerShot SX1 IS" },
        { static_cast<long int>(0x2470000), "PowerShot SX10 IS" },
        { static_cast<long int>(0x2480000), "PowerShot A1000 IS" },
        { static_cast<long int>(0x2490000), "PowerShot G10" },
        { static_cast<long int>(0x2510000), "PowerShot A2000 IS" },
        { static_cast<long int>(0x2520000), "PowerShot SX110 IS" },
        { static_cast<long int>(0x2530000), "PowerShot SD990 IS / Digital IXUS 980 IS / IXY Digital 3000 IS" },
        { static_cast<long int>(0x2540000), "PowerShot SD880 IS / Digital IXUS 870 IS / IXY Digital 920 IS" },
        { static_cast<long int>(0x2550000), "PowerShot E1" },
        { static_cast<long int>(0x2560000), "PowerShot D10" },
        { static_cast<long int>(0x2570000), "PowerShot SD960 IS / Digital IXUS 110 IS / IXY Digital 510 IS" },
        { static_cast<long int>(0x2580000), "PowerShot A2100 IS" },
        { static_cast<long int>(0x2590000), "PowerShot A480" },
        { static_cast<long int>(0x2600000), "PowerShot SX200 IS" },
        { static_cast<long int>(0x2610000), "PowerShot SD970 IS / Digital IXUS 990 IS / IXY Digital 830 IS" },
        { static_cast<long int>(0x2620000), "PowerShot SD780 IS / Digital IXUS 100 IS / IXY Digital 210 IS" },
        { static_cast<long int>(0x2630000), "PowerShot A1100 IS" },
        { static_cast<long int>(0x2640000), "PowerShot SD1200 IS / Digital IXUS 95 IS / IXY Digital 110 IS" },
        { static_cast<long int>(0x2700000), "PowerShot G11" },
        { static_cast<long int>(0x2710000), "PowerShot SX120 IS" },
        { static_cast<long int>(0x2720000), "PowerShot S90" },
        { static_cast<long int>(0x2750000), "PowerShot SX20 IS" },
        { static_cast<long int>(0x2760000), "PowerShot SD980 IS / Digital IXUS 200 IS / IXY Digital 930 IS" },
        { static_cast<long int>(0x2770000), "PowerShot SD940 IS / Digital IXUS 120 IS / IXY Digital 220 IS" },
        { static_cast<long int>(0x2800000), "PowerShot A495" },
        { static_cast<long int>(0x2810000), "PowerShot A490" },
        { static_cast<long int>(0x2820000), "PowerShot A3100/A3150 IS" },
        { static_cast<long int>(0x2830000), "PowerShot A3000 IS" },
        { static_cast<long int>(0x2840000), "PowerShot SD1400 IS / IXUS 130 / IXY 400F" },
        { static_cast<long int>(0x2850000), "PowerShot SD1300 IS / IXUS 105 / IXY 200F" },
        { static_cast<long int>(0x2860000), "PowerShot SD3500 IS / IXUS 210 / IXY 10S" },
        { static_cast<long int>(0x2870000), "PowerShot SX210 IS" },
        { static_cast<long int>(0x2880000), "PowerShot SD4000 IS / IXUS 300 HS / IXY 30S" },
        { static_cast<long int>(0x2890000), "PowerShot SD4500 IS / IXUS 1000 HS / IXY 50S" },
        { static_cast<long int>(0x2920000), "PowerShot G12" },
        { static_cast<long int>(0x2930000), "PowerShot SX30 IS" },
        { static_cast<long int>(0x2940000), "PowerShot SX130 IS" },
        { static_cast<long int>(0x2950000), "PowerShot S95" },
        { static_cast<long int>(0x2980000), "PowerShot A3300 IS" },
        { static_cast<long int>(0x2990000), "PowerShot A3200 IS" },
        { static_cast<long int>(0x3000000), "PowerShot ELPH 500 HS / IXUS 310 HS / IXY 31S" },
        { static_cast<long int>(0x3010000), "PowerShot Pro90 IS" },
        { static_cast<long int>(0x3010001), "PowerShot A800" },
        { static_cast<long int>(0x3020000), "PowerShot ELPH 100 HS / IXUS 115 HS / IXY 210F" },
        { static_cast<long int>(0x3030000), "PowerShot SX230 HS" },
        { static_cast<long int>(0x3040000), "PowerShot ELPH 300 HS / IXUS 220 HS / IXY 410F" },
        { static_cast<long int>(0x3050000), "PowerShot A2200" },
        { static_cast<long int>(0x3060000), "PowerShot A1200" },
        { static_cast<long int>(0x3070000), "PowerShot SX220 HS" },
        { static_cast<long int>(0x3080000), "PowerShot G1 X" },
        { static_cast<long int>(0x3090000), "PowerShot SX150 IS" },
        { static_cast<long int>(0x3100000), "PowerShot ELPH 510 HS / IXUS 1100 HS / IXY 51S" },
        { static_cast<long int>(0x3110000), "PowerShot S100 (new)" },
        { static_cast<long int>(0x3130000), "PowerShot SX40 HS" },
        { static_cast<long int>(0x3120000), "PowerShot ELPH 310 HS / IXUS 230 HS / IXY 600F" },
        { static_cast<long int>(0x3140000), "IXY 32S" },
        { static_cast<long int>(0x3160000), "PowerShot A1300" },
        { static_cast<long int>(0x3170000), "PowerShot A810" },
        { static_cast<long int>(0x3180000), "PowerShot ELPH 320 HS / IXUS 240 HS / IXY 420F" },
        { static_cast<long int>(0x3190000), "PowerShot ELPH 110 HS / IXUS 125 HS / IXY 220F" },
        { static_cast<long int>(0x3200000), "PowerShot D20" },
        { static_cast<long int>(0x3210000), "PowerShot A4000 IS" },
        { static_cast<long int>(0x3220000), "PowerShot SX260 HS" },
        { static_cast<long int>(0x3230000), "PowerShot SX240 HS" },
        { static_cast<long int>(0x3240000), "PowerShot ELPH 530 HS / IXUS 510 HS / IXY 1" },
        { static_cast<long int>(0x3250000), "PowerShot ELPH 520 HS / IXUS 500 HS / IXY 3" },
        { static_cast<long int>(0x3260000), "PowerShot A3400 IS" },
        { static_cast<long int>(0x3270000), "PowerShot A2400 IS" },
        { static_cast<long int>(0x3280000), "PowerShot A2300" },
        { static_cast<long int>(0x3320000), "PowerShot S100V" },
        { static_cast<long int>(0x3330000), "PowerShot G15" },
        { static_cast<long int>(0x3340000), "PowerShot SX50 HS" },
        { static_cast<long int>(0x3350000), "PowerShot SX160 IS" },
        { static_cast<long int>(0x3360000), "PowerShot S110 (new)" },
        { static_cast<long int>(0x3370000), "PowerShot SX500 IS" },
        { static_cast<long int>(0x3380000), "PowerShot N" },
        { static_cast<long int>(0x3390000), "IXUS 245 HS / IXY 430F" },
        { static_cast<long int>(0x3400000), "PowerShot SX280 HS" },
        { static_cast<long int>(0x3410000), "PowerShot SX270 HS" },
        { static_cast<long int>(0x3420000), "PowerShot A3500 IS" },
        { static_cast<long int>(0x3430000), "PowerShot A2600" },
        { static_cast<long int>(0x3440000), "PowerShot SX275 HS" },
        { static_cast<long int>(0x3450000), "PowerShot A1400" },
        { static_cast<long int>(0x3460000), "PowerShot ELPH 130 IS / IXUS 140 / IXY 110F" },
        { static_cast<long int>(0x3470000), "PowerShot ELPH 115/120 IS / IXUS 132/135 / IXY 90F/100F" },
        { static_cast<long int>(0x3490000), "PowerShot ELPH 330 HS / IXUS 255 HS / IXY 610F" },
        { static_cast<long int>(0x3510000), "PowerShot A2500" },
        { static_cast<long int>(0x3540000), "PowerShot G16" },
        { static_cast<long int>(0x3550000), "PowerShot S120" },
        { static_cast<long int>(0x3560000), "PowerShot SX170 IS" },
        { static_cast<long int>(0x3580000), "PowerShot SX510 HS" },
        { static_cast<long int>(0x3590000), "PowerShot S200 (new)" },
        { static_cast<long int>(0x3600000), "IXY 620F" },
        { static_cast<long int>(0x3610000), "PowerShot N100" },
        { static_cast<long int>(0x3640000), "PowerShot G1 X Mark II" },
        { static_cast<long int>(0x3650000), "PowerShot D30" },
        { static_cast<long int>(0x3660000), "PowerShot SX700 HS" },
        { static_cast<long int>(0x3670000), "PowerShot SX600 HS" },
        { static_cast<long int>(0x3680000), "PowerShot ELPH 140 IS / IXUS 150 / IXY 130" },
        { static_cast<long int>(0x3690000), "PowerShot ELPH 135 / IXUS 145 / IXY 120" },
        { static_cast<long int>(0x3700000), "PowerShot ELPH 340 HS / IXUS 265 HS / IXY 630" },
        { static_cast<long int>(0x3710000), "PowerShot ELPH 150 IS / IXUS 155 / IXY 140" },
        { static_cast<long int>(0x3740000), "EOS M3" },
        { static_cast<long int>(0x3750000), "PowerShot SX60 HS" },
        { static_cast<long int>(0x3760000), "PowerShot SX520 HS" },
        { static_cast<long int>(0x3770000), "PowerShot SX400 IS" },
        { static_cast<long int>(0x3780000), "PowerShot G7 X" },
        { static_cast<long int>(0x3790000), "PowerShot N2" },
        { static_cast<long int>(0x3800000), "PowerShot SX530 HS" },
        { static_cast<long int>(0x3820000), "PowerShot SX710 HS" },
        { static_cast<long int>(0x3830000), "PowerShot SX610 HS" },
        { static_cast<long int>(0x3840000), "EOS M10" },
        { static_cast<long int>(0x3850000), "PowerShot G3 X" },
        { static_cast<long int>(0x3860000), "PowerShot ELPH 165 HS / IXUS 165 / IXY 160" },
        { static_cast<long int>(0x3870000), "PowerShot ELPH 160 / IXUS 160" },
        { static_cast<long int>(0x3880000), "PowerShot ELPH 350 HS / IXUS 275 HS / IXY 640" },
        { static_cast<long int>(0x3890000), "PowerShot ELPH 170 IS / IXUS 170" },
        { static_cast<long int>(0x3910000), "PowerShot SX410 IS" },
        { static_cast<long int>(0x3930000), "PowerShot G9 X" },
        { static_cast<long int>(0x3940000), "EOS M5" },
        { static_cast<long int>(0x3950000), "PowerShot G5 X" },
        { static_cast<long int>(0x3970000), "PowerShot G7 X Mark II" },
        { static_cast<long int>(0x3980000), "EOS M100" },
        { static_cast<long int>(0x3990000), "PowerShot ELPH 360 HS / IXUS 285 HS / IXY 650" },
        { static_cast<long int>(0x4010000), "PowerShot SX540 HS" },
        { static_cast<long int>(0x4020000), "PowerShot SX420 IS" },
        { static_cast<long int>(0x4030000), "PowerShot ELPH 190 IS / IXUS 180 / IXY 190" },
        { static_cast<long int>(0x4040000), "PowerShot G1" },
        { static_cast<long int>(0x4040001), "PowerShot ELPH 180 IS / IXUS 175 / IXY 180" },
        { static_cast<long int>(0x4050000), "PowerShot SX720 HS" },
        { static_cast<long int>(0x4060000), "PowerShot SX620 HS" },
        { static_cast<long int>(0x4070000), "EOS M6" },
        { static_cast<long int>(0x4100000), "PowerShot G9 X Mark II" },
        { static_cast<long int>(0x412), "EOS M50 / Kiss M" },
        { static_cast<long int>(0x4150000), "PowerShot ELPH 185 / IXUS 185 / IXY 200" },
        { static_cast<long int>(0x4160000), "PowerShot SX430 IS" },
        { static_cast<long int>(0x4170000), "PowerShot SX730 HS" },
        { static_cast<long int>(0x4180000), "PowerShot G1 X Mark III" },
        { static_cast<long int>(0x6040000), "PowerShot S100 / Digital IXUS / IXY Digital" },
        { static_cast<long int>(0x801), "PowerShot SX740 HS" },
        { static_cast<long int>(0x804), "PowerShot G5 X Mark II" },
        { static_cast<long int>(0x805), "PowerShot SX70 HS" },
        { static_cast<long int>(0x808), "PowerShot G7 X Mark III" },
        { static_cast<long int>(0x811), "EOS M6 Mark II" },
        { static_cast<long int>(0x812), "EOS M200" },
        { static_cast<long int>(0x4007d673), "DC19/DC21/DC22" },
        { static_cast<long int>(0x4007d674), "XH A1" },
        { static_cast<long int>(0x4007d675), "HV10" },
        { static_cast<long int>(0x4007d676), "MD130/MD140/MD150/MD160/ZR850" },
        { static_cast<long int>(0x4007d777), "DC50" },
        { static_cast<long int>(0x4007d778), "HV20" },
        { static_cast<long int>(0x4007d779), "DC211" },
        { static_cast<long int>(0x4007d77a), "HG10" },
        { static_cast<long int>(0x4007d77b), "HR10" },
        { static_cast<long int>(0x4007d77d), "MD255/ZR950" },
        { static_cast<long int>(0x4007d81c), "HF11" },
        { static_cast<long int>(0x4007d878), "HV30" },
        { static_cast<long int>(0x4007d87c), "XH A1S" },
        { static_cast<long int>(0x4007d87e), "DC301/DC310/DC311/DC320/DC330" },
        { static_cast<long int>(0x4007d87f), "FS100" },
        { static_cast<long int>(0x4007d880), "HF10" },
        { static_cast<long int>(0x4007d882), "HG20/HG21" },
        { static_cast<long int>(0x4007d925), "HF21" },
        { static_cast<long int>(0x4007d926), "HF S11" },
        { static_cast<long int>(0x4007d978), "HV40" },
        { static_cast<long int>(0x4007d987), "DC410/DC411/DC420" },
        { static_cast<long int>(0x4007d988), "FS19/FS20/FS21/FS22/FS200" },
        { static_cast<long int>(0x4007d989), "HF20/HF200" },
        { static_cast<long int>(0x4007d98a), "HF S10/S100" },
        { static_cast<long int>(0x4007da8e), "HF R10/R16/R17/R18/R100/R106" },
        { static_cast<long int>(0x4007da8f), "HF M30/M31/M36/M300/M306" },
        { static_cast<long int>(0x4007da90), "HF S20/S21/S200" },
        { static_cast<long int>(0x4007da92), "FS31/FS36/FS37/FS300/FS305/FS306/FS307" },
        { static_cast<long int>(0x4007dca0), "EOS C300" },
        { static_cast<long int>(0x4007dda9), "HF G25" },
        { static_cast<long int>(0x4007dfb4), "XC10" },
        { static_cast<long int>(0x4007e1c3), "EOS C200" },
        { static_cast<long int>(0x80000001), "EOS-1D" },
        { static_cast<long int>(0x80000167), "EOS-1DS" },
        { static_cast<long int>(0x80000168), "EOS 10D" },
        { static_cast<long int>(0x80000169), "EOS-1D Mark III" },
        { static_cast<long int>(0x80000170), "EOS Digital Rebel / 300D / Kiss Digital" },
        { static_cast<long int>(0x80000174), "EOS-1D Mark II" },
        { static_cast<long int>(0x80000175), "EOS 20D" },
        { static_cast<long int>(0x80000176), "EOS Digital Rebel XSi / 450D / Kiss X2" },
        { static_cast<long int>(0x80000188), "EOS-1Ds Mark II" },
        { static_cast<long int>(0x80000189), "EOS Digital Rebel XT / 350D / Kiss Digital N" },
        { static_cast<long int>(0x80000190), "EOS 40D" },
        { static_cast<long int>(0x80000213), "EOS 5D" },
        { static_cast<long int>(0x80000215), "EOS-1Ds Mark III" },
        { static_cast<long int>(0x80000218), "EOS 5D Mark II" },
        { static_cast<long int>(0x80000219), "WFT-E1" },
        { static_cast<long int>(0x80000232), "EOS-1D Mark II N" },
        { static_cast<long int>(0x80000234), "EOS 30D" },
        { static_cast<long int>(0x80000236), "EOS Digital Rebel XTi / 400D / Kiss Digital X" },
        { static_cast<long int>(0x80000241), "WFT-E2" },
        { static_cast<long int>(0x80000246), "WFT-E3" },
        { static_cast<long int>(0x80000250), "EOS 7D" },
        { static_cast<long int>(0x80000252), "EOS Rebel T1i / 500D / Kiss X3" },
        { static_cast<long int>(0x80000254), "EOS Rebel XS / 1000D / Kiss F" },
        { static_cast<long int>(0x80000261), "EOS 50D" },
        { static_cast<long int>(0x80000269), "EOS-1D X" },
        { static_cast<long int>(0x80000270), "EOS Rebel T2i / 550D / Kiss X4" },
        { static_cast<long int>(0x80000271), "WFT-E4" },
        { static_cast<long int>(0x80000273), "WFT-E5" },
        { static_cast<long int>(0x80000281), "EOS-1D Mark IV" },
        { static_cast<long int>(0x80000285), "EOS 5D Mark III" },
        { static_cast<long int>(0x80000286), "EOS Rebel T3i / 600D / Kiss X5" },
        { static_cast<long int>(0x80000287), "EOS 60D" },
        { static_cast<long int>(0x80000288), "EOS Rebel T3 / 1100D / Kiss X50" },
        { static_cast<long int>(0x80000289), "EOS 7D Mark II" },
        { static_cast<long int>(0x80000297), "WFT-E2 II" },
        { static_cast<long int>(0x80000298), "WFT-E4 II" },
        { static_cast<long int>(0x80000301), "EOS Rebel T4i / 650D / Kiss X6i" },
        { static_cast<long int>(0x80000302), "EOS 6D" },
        { static_cast<long int>(0x80000324), "EOS-1D C" },
        { static_cast<long int>(0x80000325), "EOS 70D" },
        { static_cast<long int>(0x80000326), "EOS Rebel T5i / 700D / Kiss X7i" },
        { static_cast<long int>(0x80000327), "EOS Rebel T5 / 1200D / Kiss X70 / Hi" },
        { static_cast<long int>(0x80000328), "EOS-1D X MARK II" },
        { static_cast<long int>(0x80000331), "EOS M" },
        { static_cast<long int>(0x80000350), "EOS 80D" },
        { static_cast<long int>(0x80000355), "EOS M2" },
        { static_cast<long int>(0x80000346), "EOS Rebel SL1 / 100D / Kiss X7" },
        { static_cast<long int>(0x80000347), "EOS Rebel T6s / 760D / 8000D" },
        { static_cast<long int>(0x80000349), "EOS 5D Mark IV" },
        { static_cast<long int>(0x80000382), "EOS 5DS" },
        { static_cast<long int>(0x80000393), "EOS Rebel T6i / 750D / Kiss X8i" },
        { static_cast<long int>(0x80000401), "EOS 5DS R" },
        { static_cast<long int>(0x80000404), "EOS Rebel T6 / 1300D / Kiss X80" },
        { static_cast<long int>(0x80000405), "EOS Rebel T7i / 800D / Kiss X9i" },
        { static_cast<long int>(0x80000406), "EOS 6D Mark II" },
        { static_cast<long int>(0x80000408), "EOS 77D / 9000D" },
        { static_cast<long int>(0x80000417), "EOS Rebel SL2 / 200D / Kiss X9" },
        { static_cast<long int>(0x80000421), "EOS R5" },        
        { static_cast<long int>(0x80000422), "EOS Rebel T100 / 4000D / 3000D" },
        { static_cast<long int>(0x80000424), "EOS R" },
        { static_cast<long int>(0x80000428), "EOS-1D X Mark III" },
        { static_cast<long int>(0x80000432), "EOS Rebel T7 / 2000D / 1500D / Kiss X90" },
        { static_cast<long int>(0x80000433), "EOS RP" },
        { static_cast<long int>(0x80000435), "EOS 850D / T8i / Kiss X10i" },
        { static_cast<long int>(0x80000436), "EOS SL3 / 250D / Kiss X10" },
        { static_cast<long int>(0x80000437), "EOS 90D" },
        { static_cast<long int>(0x80000453), "EOS R6" },
        //{ (long int)tbd, "EOS Ra" },
        //{ (long int)tbd, "EOS M50 Mark II" },
        {static_cast<long int>(0x80000520), "EOS D2000C"},
        {static_cast<long int>(0x80000560), "EOS D6000C"}};

    //! SerialNumberFormat, tag 0x0015
    constexpr TagDetails canonSerialNumberFormat[] = {
        {  static_cast<long int>(0x90000000), N_("Format 1") },
        {  static_cast<long int>(0xa0000000), N_("Format 2") },
    };

    //! SuperMacro, tag 0x001a
    constexpr TagDetails canonSuperMacro[] = {
        {  0, N_("Off")     },
        {  1, N_("On (1)")  },
        {  2, N_("On (2)")  }
    };

    // DateStampMode, tag 0x001c
    constexpr TagDetails canonDateStampMode[] = {
        { 0, N_("Off")  },
        { 1, N_("Date") },
        { 2, N_("Date & Time") }
    };



    // Categories, tag 0x0023
    EXV_UNUSED constexpr TagDetails canonCategories[] = {
        { 0x0001, N_("People")  },
        { 0x0002, N_("Scenery") },
        { 0x0004, N_("Events")  },
        { 0x0008, N_("User 1")  },
        { 0x0016, N_("User 2")  },
        { 0x0032, N_("User 3")  },
        { 0x0064, N_("To Do")   }
    };



    //! PictureStyle Values
    constexpr TagDetails canonPictureStyle[] = {
        { 0x00, N_("None")            },
        { 0x01, N_("Standard")        },
        { 0x02, N_("Portrait")        },
        { 0x03, N_("High Saturation") },
        { 0x04, N_("Adobe RGB")       },
        { 0x05, N_("Low Saturation")  },
        { 0x06, N_("CM Set 1")        },
        { 0x07, N_("CM Set 2")        },
        { 0x21, N_("User Def. 1")     },
        { 0x22, N_("User Def. 2")     },
        { 0x23, N_("User Def. 3")     },
        { 0x41, N_("PC 1")            },
        { 0x42, N_("PC 2")            },
        { 0x43, N_("PC 3")            },
        { 0x81, N_("Standard")        },
        { 0x82, N_("Portrait")        },
        { 0x83, N_("Landscape")       },
        { 0x84, N_("Neutral")         },
        { 0x85, N_("Faithful")        },
        { 0x86, N_("Monochrome")      },
        { 0x87, N_("Auto")            },
        { 0x88, N_("Fine Detail")     }
    };

    //! WhiteBalance, multiple tags
    constexpr TagDetails canonSiWhiteBalance[] = {
        {  0, N_("Auto")                        },
        {  1, N_("Daylight")                    },
        {  2, N_("Cloudy")                      },
        {  3, N_("Tungsten")                    },
        {  4, N_("Fluorescent")                 },
        {  5, N_("Flash")                       },
        {  6, N_("Custom")                      },
        {  7, N_("Black & White")               },
        {  8, N_("Shade")                       },
        {  9, N_("Manual Temperature (Kelvin)") },
        { 10, N_("PC Set 1")                    },
        { 11, N_("PC Set 2")                    },
        { 12, N_("PC Set 3")                    },
        { 14, N_("Daylight Fluorescent")        },
        { 15, N_("Custom 1")                    },
        { 16, N_("Custom 2")                    },
        { 17, N_("Underwater")                  },
        { 18, N_("Custom 3")                    },
        { 19, N_("Custom 3")                    },
        { 20, N_("PC Set 4")                    },
        { 21, N_("PC Set 5")                    },
        { 23, N_("Auto (ambience priority)")    }
    };


    //! ColorSpace, tag 0x00b4
    constexpr TagDetails canonColorSpace[] = {
        {  1, N_("sRGB")      },
        {  2, N_("Adobe RGB") }
    };

    //! Canon AF Area Mode, tag 0x2601
    constexpr TagDetails canonAFAreaMode[] = {
         {   0, N_("Off (Manual Focus)")           },
         {   1, N_("AF Point Expansion (surround)")},
         {   2, N_("Single-point AF")              },
         {   4, N_("Multi-point AF")               },
         {   5, N_("Face Detect AF")               },
         {   6, N_("Face + Tracking")              },
         {   7, N_("Zone AF")                      },
         {   8, N_("AF Point Expansion (4 point)") },
         {   9, N_("Spot AF")                      },
         {  10, N_("AF Point Expansion (8 point)") },
         {  11, N_("Flexizone Multi (49 point)")   },
         {  12, N_("Flexizone Multi (9 point)")    },
         {  13, N_("Flexizone Single")             },
         {  14, N_("Large Zone AF")                },
     };


    // Canon MakerNote Tag Info
    constexpr TagInfo CanonMakerNote::tagInfo_[] = {
        {0x0000, "0x0000", "0x0000", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x0001, "CameraSettings", N_("Camera Settings"), N_("Various camera settings"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x0002, "FocalLength", N_("Focal Length"), N_("Focal length"), canonId, makerTags, unsignedShort, -1, printFocalLength},
        {0x0003, "0x0003", "0x0003", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x0004, "ShotInfo", N_("Shot Info"), N_("Shot information"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x0005, "Panorama", N_("Panorama"), N_("Panorama"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x0006, "ImageType", N_("Image Type"), N_("Image type"), canonId, makerTags, asciiString, -1, printValue},
        {0x0007, "FirmwareVersion", N_("Firmware Version"), N_("Firmware version"), canonId, makerTags, asciiString, -1, printValue},
        {0x0008, "FileNumber", N_("File Number"), N_("File number"), canonId, makerTags, unsignedLong, -1, print0x0008},
        {0x0009, "OwnerName", N_("Owner Name"), N_("Owner Name"), canonId, makerTags, asciiString, -1, printValue},
        {0x000a, "0x000a", N_("0x000a"), N_("Unknow"), canonId, makerTags, unsignedLong, -1, print0x000c},
        {0x000c, "SerialNumber", N_("Serial Number"), N_("Camera serial number"), canonId, makerTags, unsignedLong, -1, print0x000c},
        {0x000d, "CameraInfo", N_("Camera Info"), N_("Camera info"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x000e, "FileLength", N_("FileLength"), N_("FileLength"), canonId, makerTags, unsignedLong, -1, printValue},
        {0x000f, "CustomFunctions", N_("Custom Functions"), N_("Custom Functions"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x0010, "ModelID", N_("ModelID"), N_("Model ID"), canonId, makerTags, unsignedLong, -1, EXV_PRINT_TAG(canonModelId)},
        {0x0011, "MovieInfo", N_("MovieInfo"), N_("Movie info"), canonId, makerTags, unsignedShort, -1, printValue},  
        {0x0012, "PictureInfo", N_("Picture Info"), N_("Picture info"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x0013, "ThumbnailImageValidArea", N_("Thumbnail Image Valid Area"), N_("Thumbnail image valid area"), canonId, makerTags, signedShort, -1, printValue},
        {0x0015, "SerialNumberFormat", N_("Serial Number Format"), N_("Serial number format"), canonId, makerTags, unsignedLong, -1, EXV_PRINT_TAG(canonSerialNumberFormat)},
        {0x001a, "SuperMacro", N_("Super Macro"), N_("Super macro"), canonId, makerTags, signedShort, -1, EXV_PRINT_TAG(canonSuperMacro)},
        {0x001c, "DateStampMode", N_("DateStampMode"), N_("Data_Stamp_Mode"), canonId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(canonDateStampMode)}, 
        {0x001d, "MyColors", N_("MyColors"), N_("My_Colors"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x001e, "FirmwareRevision", N_("FirmwareRevision"), N_("Firmware_Revision"), canonId, makerTags, unsignedLong, -1, printValue}, 
        // {0x0023, "Categories", N_("Categories"), N_("Categories"), canonId, makerTags, unsignedLong -1, EXV_PRINT_TAG(canonCategories)}, 
        {0x0024, "FaceDetect1", N_("FaceDetect1"), N_("FaceDetect1"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x0025, "FaceDetect2", N_("FaceDetect2"), N_("FaceDetect2"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x0026, "AFInfo", N_("AF Info"), N_("AF info"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x0027, "ContrastInfo", N_("ContrastInfo"), N_("ContrastInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x0028, "ImageUniqueID", N_("ImageUniqueID"), N_("ImageUniqueID"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x0029, "WBInfo", N_("WBInfo"), N_("WBInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x002f, "FaceDetect3", N_("FaceDetect3"), N_("FaceDetect3"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x0035, "TimeInfo", N_("Time Info"), N_("Time zone information"), canonId, makerTags, signedLong, -1, printValue},
        {0x0038, "BatteryType", N_("BatteryType"), N_("BatteryType"), canonId, makerTags, unsignedLong, -1, printValue}, 
        {0x003c, "AFInfo3", N_("AFInfo3"), N_("AFInfo3"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x0081, "RawDataOffset", N_("RawDataOffset"), N_("RawDataOffset"), canonId, makerTags, signedLong, -1, printValue}, 
        {0x0083, "OriginalDecisionDataOffset", N_("Original Decision Data Offset"), N_("Original decision data offset"), canonId, makerTags, signedLong, -1, printValue},
        {0x00a4, "WhiteBalanceTable", N_("White Balance Table"), N_("White balance table"), canonId, makerTags, unsignedShort, -1, printValue},
        // {0x0090, "CustomFunctions1D", N_("CustomFunctions1D"), N_("CustomFunctions1D"), canonId, makerTags, unsignedShort, -1, printValue}, // ToDo
        // {0x0091, "PersonalFunctions", N_("PersonalFunctions"), N_("PersonalFunctions"), canonId, makerTags, unsignedShort, -1, printValue}, // ToDo
        // {0x0092, "PersonalFunctionValues", N_("PersonalFunctionValues"), N_("PersonalFunctionValues"), canonId, makerTags, unsignedShort, -1, printValue}, // ToDo
        {0x0093, "CanonFileInfo", N_("CanonFileInfo"), N_("CanonFileInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x0094, "AFPointsInFocus1D", N_("AFPointsInFocus1D"), N_("AFPointsInFocus1D"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x0095, "LensModel", N_("Lens Model"), N_("Lens model"), canonId, makerTags, asciiString, -1, printValue},
        {0x0096, "InternalSerialNumber", N_("Internal Serial Number"), N_("Internal serial number"), canonId, makerTags, asciiString, -1, printValue},
        {0x0097, "DustRemovalData", N_("Dust Removal Data"), N_("Dust removal data"), canonId, makerTags, asciiString, -1, printValue},
        {0x0099, "CustomFunctions", N_("Custom Functions"), N_("Custom functions"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x009a, "AspectInfo", N_("AspectInfo"), N_("AspectInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00a0, "ProcessingInfo", N_("Processing Info"), N_("Processing info"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x00a1, "ToneCurveTable", N_("ToneCurveTable"), N_("ToneCurveTable"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00a2, "SharpnessTable", N_("SharpnessTable"), N_("SharpnessTable"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00a3, "SharpnessFreqTable", N_("SharpnessTable"), N_("SharpnessTable"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00a4, "WhiteBalanceTable", N_("SharpnessTable"), N_("SharpnessTable"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00a9, "ColorBalance", N_("ColorBalance"), N_("ColorBalance"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00aa, "MeasuredColor", N_("Measured Color"), N_("Measured color"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x00ae, "ColorTemperature", N_("ColorTemperature"), N_("ColorTemperature"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00b0, "CanonFlags", N_("CanonFlags"), N_("CanonFlags"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00b1, "ModifiedInfo", N_("ModifiedInfo"), N_("ModifiedInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00b2, "ToneCurveMatching", N_("ToneCurveMatching"), N_("ToneCurveMatching"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00b3, "WhiteBalanceMatching", N_("WhiteBalanceMatching"), N_("WhiteBalanceMatching"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00b4, "ColorSpace", N_("ColorSpace"), N_("ColorSpace"), canonId, makerTags, signedShort,  -1, EXV_PRINT_TAG(canonColorSpace)},
        {0x00b5, "0x00b5", "0x00b5", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x00b6, "PreviewImageInfo", "PreviewImageInfo", N_("PreviewImageInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x00c0, "0x00c0", "0x00c0", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x00c1, "0x00c1", "0x00c1", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x00d0, "VRDOffset", N_("VRD Offset"), N_("VRD offset"), canonId, makerTags, unsignedLong, -1, printValue},
        {0x00e0, "SensorInfo", N_("Sensor Info"), N_("Sensor info"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x2600, "AFInfoSize", N_("AF InfoSize"), N_("AF InfoSize"), canonId, makerTags, signedShort, -1, printValue},
        {0x2601, "AFAreaMode", N_("AF Area Mode"), N_("AF Area Mode"), canonId, makerTags, signedShort, -1, EXV_PRINT_TAG(canonAFAreaMode)},
        {0x2602, "AFNumPoints", N_("AF NumPoints"), N_("AF NumPoints"), canonId, makerTags, signedShort, -1, printValue},
        {0x2603, "AFValidPoints", N_("AF ValidPoints"), N_("AF ValidPoints"), canonId, makerTags, signedShort, -1, printValue},
        {0x2604, "AFCanonImageWidth", N_("AF ImageWidth"), N_("AF ImageWidth"), canonId, makerTags, signedShort, -1, printValue},
        {0x2605, "AFCanonImageHeight", N_("AF ImageHeight"), N_("AF ImageHeight"), canonId, makerTags, signedShort, -1, printValue},
        {0x2606, "AFImageWidth", N_("AF Width"), N_("AF Width"), canonId, makerTags, signedShort, -1, printValue},
        {0x2607, "AFImageHeight", N_("AF Height"), N_("AF Height"), canonId, makerTags, signedShort, -1, printValue},
        {0x2608, "AFAreaWidths", N_("AF Area Widths"), N_("AF Area Widths"), canonId, makerTags, signedShort, -1, printValue},
        {0x2609, "AFAreaHeights", N_("AF Area Heights"), N_("AF Area Heights"), canonId, makerTags, signedShort, -1, printValue},
        {0x260a, "AFXPositions", N_("AF X Positions"), N_("AF X Positions"), canonId, makerTags, signedShort, -1, printValue},
        {0x260b, "AFYPositions", N_("AF Y Positions"), N_("AF Y Positions"), canonId, makerTags, signedShort, -1, printValue},
        {0x260c, "AFPointsInFocus", N_("AF Points in Focus"), N_("AF Points in Focus"), canonId, makerTags, signedShort, -1,printBitmask},
        {0x260d, "AFPointsSelected", N_("AF Points Selected"), N_("AF Points Selected"), canonId, makerTags, signedShort, -1, printBitmask},
        {0x260e, "AFPointsUnusable", N_("AF Points Unusable"), N_("AF Points Unusable"), canonId, makerTags, signedShort, -1, printBitmask},
        {0x4001, "ColorData", N_("Color Data"), N_("Color data"), canonId, makerTags, unsignedShort, -1, printValue},
        {0x4002, "CRWParam", N_("CRWParam"), N_("CRWParam"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4003, "ColorInfo", N_("ColorInfo"), N_("ColorInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4005, "Flavor", N_("Flavor"), N_("Flavor"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4008, "PictureStyleUserDef", N_("PictureStyleUserDef"), N_("PictureStyleUserDef"), canonId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(canonPictureStyle)}, 
        // {0x4009, "PictureStylePC", N_("PictureStylePC"), N_("PictureStylePC"), canonId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(canonPictureStyle)}, 
        {0x4010, "CustomPictureStyleFileName", N_("CustomPictureStyleFileName"), N_("CustomPictureStyleFileName"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4013, "AFMicroAdj", N_("AFMicroAdj"), N_("AFMicroAdj"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4015, "VignettingCorr", N_("VignettingCorr"), N_("VignettingCorr"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4016, "VignettingCorr2", N_("VignettingCorr2"), N_("VignettingCorr2"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4018, "LightingOpt", N_("LightingOpt"), N_("LightingOpt"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4018, "LensInfo", N_("LensInfo"), N_("LensInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4020, "AmbienceInfo", N_("AmbienceInfo"), N_("AmbienceInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4021, "MultiExp", N_("MultiExp"), N_("MultiExp"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4024, "FilterInfo", N_("FilterInfo"), N_("FilterInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4025, "HDRInfo", N_("HDRInfo"), N_("HDRInfo"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x4028, "AFConfig", N_("AFConfig"), N_("AFConfig"), canonId, makerTags, unsignedShort, -1, printValue}, 
        {0x403f, "RawBurstModeRoll", N_("RawBurstModeRoll"), N_("RawBurstModeRoll"), canonId, makerTags, unsignedShort, -1, printValue}, 
        // End of list marker
        {0xffff, "(UnknownCanonMakerNoteTag)", "(UnknownCanonMakerNoteTag)", N_("Unknown CanonMakerNote tag"), canonId, makerTags, asciiString, -1, printValue},
    };

    const TagInfo* CanonMakerNote::tagList()
    {
        return tagInfo_;
    }
    
    // Canon Movie Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoMv_[] = {
         {0x0001, "FrameRate", N_("FrameRate"), N_("FrameRate"), canonMvId, makerTags, unsignedShort, -1, printValue},
         {0x0002, "FrameCount", N_("FrameCount"), N_("FrameCount"), canonMvId, makerTags, unsignedShort, -1, printValue},
         {0x0004, "FrameCount", N_("FrameCount"), N_("FrameCount"), canonMvId, makerTags, unsignedLong, -1, printValue},
         {0x0006, "FrameRate", N_("FrameCount"), N_("FrameCount"), canonMvId, makerTags, unsignedRational, -1, printValue},
         {0x006a, "Duration", N_("Duration"), N_("Duration"), canonMvId, makerTags, unsignedLong, -1, printValue},
         {0x006c, "AudioBitrate", N_("Audio Bitrate"), N_("Audio Bitrate"), canonMvId, makerTags, unsignedLong, -1, printValue},
         {0x006e, "AudioSampleRate", N_("Audio Sample Rate"), N_("Audio Sample Rate"), canonMvId, makerTags, unsignedLong, -1, printValue},
         {0x0070, "AudioChannels", N_("Audio Channels"), N_("Audio Channels"), canonMvId, makerTags, unsignedLong, -1, printValue},
         {0x0074, "VideoCodec", N_("Video Codec"), N_("Video Codec"), canonMvId, makerTags, asciiString, -1, printValue},
    };

    const TagInfo* CanonMakerNote::tagListMv()
    {
        return tagInfoMv_;
    }

    // MyColors, tag 0x001d
    constexpr TagDetails canonMyColors[] = {
        { 0, N_("Off")  },
        { 1, N_("Positive Film") },
        { 2, N_("Light Skin Tone") },
        { 3, N_("Dark Skin Tone")  },
        { 4, N_("Vivid Blue") },
        { 5, N_("Vivid Green") },
        { 6, N_("Vivid Red")  },
        { 7, N_("Color Accent") },
        { 8, N_("Color Swap") },
        { 9, N_("Custom")  },
        { 12, N_("Vivid") },
        { 13, N_("Neutral") },
        { 14, N_("Sepia") },
        { 15, N_("B&W") }
    };


    // Canon My Colors Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoMc_[] = {
         {0x0002, "MyColorMode", N_("My Color Mode"), N_("My Color Mode"), canonMyColorID, makerTags, unsignedShort, -1, EXV_PRINT_TAG(canonMyColors)},
    };
        
    const TagInfo* CanonMakerNote::tagListMc()
    {
        return tagInfoMc_;
    }

       // Canon FaceDetect 1 Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoFcd1_[] = {
         {0x0002, "FacesDetected", N_("Faces Detected"), N_("Faces Detected"), canonFcd1Id, makerTags, unsignedShort, -1, printValue},
         {0x0003, "FacesDetectedFrameSize", N_("Faces Detected Frame Size"), N_("Faces Detected Frame Size"), canonFcd1Id, makerTags, unsignedShort, -1, printValue},
         {0x0008, "Face1Position", N_("Face 1 Position"), N_("Face 1 Position"), canonFcd1Id, makerTags, signedShort, -1, printValue},
         {0x000a, "Face2Position", N_("Face 2 Position"), N_("Face 2 Position"), canonFcd1Id, makerTags, signedShort, -1, printValue},
         {0x000c, "Face3Position", N_("Face 3 Position"), N_("Face 3 Position"), canonFcd1Id, makerTags, signedShort, -1, printValue},
         {0x000e, "Face4Position", N_("Face 4 Position"), N_("Face 4 Position"), canonFcd1Id, makerTags, signedShort, -1, printValue},
         {0x0010, "Face5Position", N_("Face 5 Position"), N_("Face 5 Position"), canonFcd1Id, makerTags, signedShort, -1, printValue},
         {0x0012, "Face6Position", N_("Face 6 Position"), N_("Face 6 Position"), canonFcd1Id, makerTags, signedShort, -1, printValue},
         {0x0014, "Face7Position", N_("Face 7 Position"), N_("Face 7 Position"), canonFcd1Id, makerTags, signedShort, -1, printValue},
         {0x0016, "Face8Position", N_("Face 8 Position"), N_("Face 8 Position"), canonFcd1Id, makerTags, signedShort, -1, printValue},
         {0x0018, "Face9Position", N_("Face 9 Position"), N_("Face 9 Position"), canonFcd1Id, makerTags, signedShort, -1, printValue},
    };
        
    const TagInfo* CanonMakerNote::tagListFcd1()
    {
        return tagInfoFcd1_;
    }

    // Canon FaceDetect 2 Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoFcd2_[] = {
         {0x0001, "FaceWidth", N_("Face Width"), N_("Faces Width"), canonFcd2Id, makerTags, unsignedByte, -1, printValue},
         {0x0002, "FacesDetected", N_("Faces Detected"), N_("Faces Detected"), canonFcd2Id, makerTags, unsignedByte, -1, printValue},            
    };
        
    const TagInfo* CanonMakerNote::tagListFcd2()
    {
        return tagInfoFcd2_;
    }

    // Canon ContrastInfo, tag 0x001d
    constexpr TagDetails canonContrastInfo[] = {
        { 0x0, N_("Off")  },
        { 0x8, N_("On") },
        { 0xfff, N_("n/a") }
    };


    // Canon Contrast Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoCo_[] = {
         {0x0004, "IntelligentContrast", N_("Intelligent Contrast"), N_("Intelligent Contrast"), canonContrastId, makerTags, unsignedShort, -1, EXV_PRINT_TAG(canonContrastInfo)},       
    };
        
    const TagInfo* CanonMakerNote::tagListCo()
    {
        return tagInfoCo_;
    }

    // Canon WhiteBalance Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoWbi_[] = {
         {0x0002, "WB_GRGBLevelsAuto", N_("WB_G RGB Levels Auto"), N_("WB_G RGB Levels Auto"), canonWbId, makerTags, unsignedLong, -1, printValue},       
         {0x000a, "WB_GRGBLevelsDaylight", N_("WB_G RGB Levels Daylight"), N_("WB_G RGB Levels Daylight"), canonWbId, makerTags, unsignedLong, -1, printValue}, 
         {0x0012, "WB_GRGBLevelsCloudy", N_("WB_G RGB Levels Cloudy"), N_("WB_G RGB Levels Cloudy"), canonWbId, makerTags, unsignedLong, -1, printValue}, 
         {0x001a, "WB_GRGBLevelsTungsten", N_("WB_G RGB Levels Tungsten"), N_("WB_G RGB Levels Tungsten"), canonWbId, makerTags, unsignedLong, -1, printValue}, 
         {0x0022, "WB_GRGBLevelsFluorescent", N_("WB_G RGB Levels Flourescent"), N_("WB_G RGB Levels Flourescent"), canonWbId, makerTags, unsignedLong, -1, printValue}, 
         {0x002a, "WB_GRGBLevelsFluorHigh", N_("WB_G RGB Levels Flourecent High"), N_("WB_G RGB Levels Flourecent High"), canonWbId, makerTags, unsignedLong, -1, printValue}, 
         {0x0032, "WB_GRGBLevelsFlash", N_("WB_G RGB Levels Flash"), N_("WB_G RGB Levels Flash"), canonWbId, makerTags, unsignedLong, -1, printValue}, 
         {0x003a, "WB_GRGBLevelsUnderwater", N_("WB_G RGB Levels Underwater"), N_("WB_G RGB Levels Underwater"), canonWbId, makerTags, unsignedLong, -1, printValue}, 
         {0x0042, "WB_GRGBLevelsCustom1", N_("WB_G RGB Levels Custom 1"), N_("WB_G RGB Levels Custom 1"), canonWbId, makerTags, unsignedLong, -1, printValue}, 
         {0x004a, "WB_GRGBLevelsCustom2", N_("WB_G RGB Levels Custom 2"), N_("WB_G RGB Levels Custom 2"), canonWbId, makerTags, unsignedLong, -1, printValue},
    };
        
    const TagInfo* CanonMakerNote::tagListWbi()
    {
        return tagInfoWbi_;
    }

    // Canon FaceDetect 3 Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoFcd3_[] = {
         {0x0003, "FacesDetected", N_("Face Detected"), N_("Faces Detected"), canonFcd3Id, makerTags, unsignedShort, -1, printValue},         
    };
        
    const TagInfo* CanonMakerNote::tagListFcd3()
    {
        return tagInfoFcd3_;
    }

    
    // Canon AFInfo2 Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoAf2_[] = {
        {0x0000, "AFInfoSize", N_("AF InfoSize"), N_("AF InfoSize"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x0001, "AFAreaMode", N_("AF Area Mode"), N_("AF Area Mode"), canonAf2Id, makerTags, signedShort, -1, EXV_PRINT_TAG(canonAFAreaMode)},
        {0x0002, "AFNumPoints", N_("AF NumPoints"), N_("AF NumPoints"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x0003, "AFValidPoints", N_("AF ValidPoints"), N_("AF ValidPoints"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x0004, "AFCanonImageWidth", N_("AF ImageWidth"), N_("AF ImageWidth"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x0005, "AFCanonImageHeight", N_("AF ImageHeight"), N_("AF ImageHeight"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x0006, "AFImageWidth", N_("AF Width"), N_("AF Width"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x0007, "AFImageHeight", N_("AF Height"), N_("AF Height"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x0008, "AFAreaWidths", N_("AF Area Widths"), N_("AF Area Widths"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x0009, "AFAreaHeights", N_("AF Area Heights"), N_("AF Area Heights"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x000a, "AFXPositions", N_("AF X Positions"), N_("AF X Positions"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x000b, "AFYPositions", N_("AF Y Positions"), N_("AF Y Positions"), canonAf2Id, makerTags, signedShort, -1, printValue},
        {0x000c, "AFPointsInFocus", N_("AF Points in Focus"), N_("AF Points in Focus"), canonAf2Id, makerTags, signedShort, -1,printBitmask},
        {0x000d, "AFPointsSelected", N_("AF Points Selected"), N_("AF Points Selected"), canonAf2Id, makerTags, signedShort, -1, printBitmask},
        {0x000e, "AFPrimaryPoint", N_("AF Primary Point"), N_("AF Primary Point"), canonAf2Id, makerTags, signedShort, -1, printBitmask},         
    };
        
    const TagInfo* CanonMakerNote::tagListAf2()
    {
        return tagInfoAf2_;
    }


    // Canon AFInfo3 Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoAf3_[] = {
        {0x0000, "AFInfoSize", N_("AF InfoSize"), N_("AF InfoSize"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x0001, "AFAreaMode", N_("AF Area Mode"), N_("AF Area Mode"), canonAf3Id, makerTags, signedShort, -1, EXV_PRINT_TAG(canonAFAreaMode)},
        {0x0002, "AFNumPoints", N_("AF NumPoints"), N_("AF NumPoints"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x0003, "AFValidPoints", N_("AF ValidPoints"), N_("AF ValidPoints"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x0004, "AFCanonImageWidth", N_("AF ImageWidth"), N_("AF ImageWidth"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x0005, "AFCanonImageHeight", N_("AF ImageHeight"), N_("AF ImageHeight"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x0006, "AFImageWidth", N_("AF Width"), N_("AF Width"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x0007, "AFImageHeight", N_("AF Height"), N_("AF Height"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x0008, "AFAreaWidths", N_("AF Area Widths"), N_("AF Area Widths"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x0009, "AFAreaHeights", N_("AF Area Heights"), N_("AF Area Heights"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x000a, "AFXPositions", N_("AF X Positions"), N_("AF X Positions"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x000b, "AFYPositions", N_("AF Y Positions"), N_("AF Y Positions"), canonAf3Id, makerTags, signedShort, -1, printValue},
        {0x000c, "AFPointsInFocus", N_("AF Points in Focus"), N_("AF Points in Focus"), canonAf3Id, makerTags, signedShort, -1,printBitmask},
        {0x000d, "AFPointsSelected", N_("AF Points Selected"), N_("AF Points Selected"), canonAf3Id, makerTags, signedShort, -1, printBitmask},
        {0x000e, "AFPrimaryPoint", N_("AF Primary Point"), N_("AF Primary Point"), canonAf3Id, makerTags, signedShort, -1, printBitmask},         
    };
        
    const TagInfo* CanonMakerNote::tagListAf3()
    {
        return tagInfoAf3_;
    }


    /*
    // Canon Aspect Info, tag 0x001d
    constexpr TagDetails canonAspectInfo[] = {
        { 0, N_("3:2") },
        { 1, N_("1:1") },
        { 2, N_("4:3") },
        { 7, N_("16:9") },
        { 8, N_("4:5") },
        { 12, N_("3:2 (APS-H crop)") },
        { 13, N_("3:2 (APS-C crop)") }
    };
    */

    // Canon Aspect Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoAs_[] = {
         {0x0000, "AspectRatio", N_("Aspect Ratio"), N_("Aspect Ratio"), canonAsId, makerTags, unsignedLong, -1, printValue},
         {0x0001, "CroppedImageWidth", N_("Cropped Image Width"), N_("Cropped Image Width"), canonAsId, makerTags, unsignedLong, -1, printValue}, 
         {0x0002, "CroppedImageHeight", N_("Cropped Image Height"), N_("Cropped Image Height"), canonAsId, makerTags, unsignedLong, -1, printValue}, 
         {0x0003, "CroppedImageLeft", N_("Cropped Image Left"), N_("Cropped Image Left"), canonAsId, makerTags, unsignedLong, -1, printValue}, 
         {0x0004, "CroppedImageTop", N_("Cropped Image Top"), N_("Cropped Image Top"), canonAsId, makerTags, unsignedLong, -1, printValue},
    }; 
        
    const TagInfo* CanonMakerNote::tagListAs()
    {
        return tagInfoAs_;
    }
      
    
    // Canon Color Balance Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoCbi_[] = {
         {0x0001, "WB_RGGBLevelsAuto", N_("WB_RGGB Levels Auto"), N_("WB_RGGB Levels Auto"), canonCbId, makerTags, signedShort, -1, printValue},       
         {0x0005, "WB_RGGBLevelsDaylight", N_("WB_RGGB Levels Daylight"), N_("WB_RGGB Levels Daylight"), canonCbId, makerTags, signedShort, -1, printValue}, 
         {0x000d, "WB_RGGBLevelsShade", N_("WB_RGGB Levels Shade"), N_("WB_RGGB Levels Shade"), canonCbId, makerTags, signedShort, -1, printValue}, 
         {0x001a, "WB_RGGBLevelsCloudy", N_("WB_RGGB Levels Cloudy"), N_("WB_RGGB Levels Cloudy"), canonCbId, makerTags, signedShort, -1, printValue}, 
         {0x0011, "WB_RGGBLevelsTungsten", N_("WB_RGGB Levels Tungsten"), N_("WB_RGGB Levels Tungsten"), canonCbId, makerTags, signedShort, -1, printValue}, 
         {0x0015, "WB_RGGBLevelsFlourescent", N_("WB_RGGB Levels Flourecent"), N_("WB_RGGB Levels Flourecent"), canonCbId, makerTags, signedShort, -1, printValue}, 
         {0x0032, "WB_RGGBLevelsFlash", N_("WB_RGGB Levels Flash"), N_("WB_RGGB Levels Flash"), canonCbId, makerTags, signedShort, -1, printValue}, 
         {0x001d, "WB_RGGBLevelsCustomBlackLevels", N_("WB_RGGB Levels Custom Black Levels"), N_("WB_RGGB Levels Custom Black Levels"), canonCbId, makerTags, signedShort, -1, printValue}, 
         {0x0021, "WB_RGGBLevelsKelvin", N_("WB_RGGB Levels Kelvin"), N_("WB_RGGB Levels Kelvin"), canonCbId, makerTags, signedShort, -1, printValue}, 
         {0x0025, "WB_RGGBBlackLevels", N_("WB_RGGB Black Levels"), N_("WB_RGGB Black Levels"), canonCbId, makerTags, signedShort, -1, printValue},
    };
        
    const TagInfo* CanonMakerNote::tagListCbi()
    {
        return tagInfoCbi_;
    }

    // Canon Flags Tag 
    constexpr TagInfo CanonMakerNote::tagInfoFl_[] = {
         {0x0001, "ModifiedParamFlag", N_("Modified Param Flag"), N_("Modified Param Flag"), canonFlId, makerTags, signedShort, -1, printValue},  
    }; 
        
    const TagInfo* CanonMakerNote::tagListFl()
    {
        return tagInfoFl_;
    }

    // Canon Modified ToneCurve Info, tag 0x0001
    constexpr TagDetails canonModifiedToneCurve[] = {
        { 0, N_("Standard") },
        { 1, N_("Manual") },
        { 2, N_("Custom") } 
    };

    // Canon Modified Sharpness Freq Info, tag 0x0002
    constexpr TagDetails canonModifiedSharpnessFreq[] = {
        { 0, N_("n/a") },
        { 1, N_("Lowest") },
        { 2, N_("Low") },
        { 3, N_("Standard") },
        { 4, N_("High") },
        { 5, N_("Highest") }
    };    
    
    // Canon ModifiedInfo Tag 
    constexpr TagInfo CanonMakerNote::tagInfoMo_[] = {
         {0x0001, "ModifiedToneCurve", N_("Modified ToneCurve"), N_("Modified ToneCurve"), canonMoID, makerTags, signedShort, -1, EXV_PRINT_TAG(canonModifiedToneCurve)},
         {0x0002, "ModifiedSharpness", N_("Modified Sharpness"), N_("Modified Sharpness"), canonMoID, makerTags, signedShort, -1, EXV_PRINT_TAG(canonModifiedSharpnessFreq)},
         {0x0003, "ModifiedSharpnessFreq", N_("Modified Sharpness Freq"), N_("Modified Sharpness Freq"), canonMoID, makerTags, signedShort, -1, printValue},
         {0x0004, "ModifiedSensorRedLevel", N_("Modified Sensor Red Level"), N_("Modified Sensor Red Level"), canonMoID, makerTags, signedShort, -1, printValue},
         {0x0005, "ModifiedSensorBlueLevel", N_("Modified Sensor Blue Level"), N_("Modified Sensor Blue Level"), canonMoID, makerTags, signedShort, -1, printValue},
         {0x0006, "ModifiedWhiteBalanceRed", N_("Modified White Balance Red"), N_("Modified White Balance Red"), canonMoID, makerTags, signedShort, -1, printValue},
         {0x0007, "ModifiedWhiteBalanceBlue", N_("Modified White Balance Blue"), N_("Modified White Balance Blue"), canonMoID, makerTags, signedShort, -1, printValue},
         {0x0008, "ModifiedWhiteBalance", N_("Modified White Balance"), N_("Modified White Balance"), canonMoID, makerTags, signedShort, -1, EXV_PRINT_TAG(canonSiWhiteBalance)},
         {0x0009, "ModifiedColorTemp", N_("Modified Color Temp"), N_("Modified Color Temp"), canonMoID, makerTags, signedShort, -1, printValue},
         {0x000a, "ModifiedPictureStyle", N_("Modified Picture Style"), N_("Modified Picture Style"), canonMoID, makerTags, signedShort, -1, EXV_PRINT_TAG(canonPictureStyle)},
         {0x000b, "ModifiedDigitalGain", N_("Modified Param Flag"), N_("Modified Param Flag"), canonMoID, makerTags, signedShort, -1, printValue},
    };
        
    const TagInfo* CanonMakerNote::tagListMo()
    {
        return tagInfoMo_;
    }

       // Canon Preview Quality Info, tag 0x0001
    constexpr TagDetails canonPreviewQuality[] = {
        { -1, N_("n/a") },
        { 1, N_("Economy") },
        { 2, N_("Normal") },
        { 3, N_("Fine") },
        { 4, N_("RAW") },
        { 5, N_("Superfine") },
        { 7, N_("CRAW") },
        { 130, N_("Normal Movie") },
        { 131, N_("Movie (2)") }
    };    
    
    // Canon Preview Image Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoPreI_[] = {
         {0x0001, "PreviewQuality", N_("Preview Quality"), N_("Preview Quality"), canonPreID, makerTags, unsignedLong, -1, EXV_PRINT_TAG(canonPreviewQuality)},
         {0x0002, "PreviewImageLength", N_("Preview Image Length"), N_("Preview Image Length"), canonPreID, makerTags, unsignedLong, -1, printValue},
         {0x0003, "PreviewImageWidth", N_("Preview Image Width"), N_("Preview Image Width"), canonPreID, makerTags, unsignedLong, -1, printValue},
         {0x0004, "PreviewImageHeight", N_("Preview Image Height"), N_("Preview Image Height"), canonPreID, makerTags, unsignedLong, -1, printValue},
         {0x0005, "PreviewImageStart", N_("Preview Image Start"), N_("Preview Image Start"), canonPreID, makerTags, unsignedLong, -1, printValue},
    }; 
        
    const TagInfo* CanonMakerNote::tagListPreI()
    {
        return tagInfoPreI_;
    }

     // Canon Color Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoCi_[] = {
         {0x0001, "Saturation", N_("Saturation"), N_("Saturation"), canonCiId, makerTags, signedShort, -1, printValue},
         {0x0002, "ColorTone", N_("Color Tone"), N_("Color Tone"), canonCiId, makerTags, signedShort, -1, printValue},
         {0x0003, "ColorSpace", N_("Color Space"), N_("Color Space"), canonCiId, makerTags, signedShort, -1, EXV_PRINT_TAG(canonColorSpace)},
    }; 
        
    const TagInfo* CanonMakerNote::tagListCi()
    {
        return tagInfoCi_;
    }     

    // Canon AFMicroAdjMode Quality Info, tag 0x0001
    constexpr TagDetails canonAFMicroAdjMode[] = {
        { 0, N_("Disable") },
        { 1, N_("Adjust all by the same amount") },
        { 2, N_("Adjust by lens") }
    };         
        

     // Canon AFMicroAdj Info Tag 
    constexpr TagInfo CanonMakerNote::tagInfoAfMiAdj_[] = {
         {0x0001, "AFMicroAdjMode", N_("AFMicroAdjMode"), N_("AFMicroAdjMode"), canonAfMiAdjId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAFMicroAdjMode)},
         {0x0002, "AFMicroAdjValue", N_("AF Micro Adj Value"), N_("AF Micro Adj Value"), canonAfMiAdjId, makerTags, signedRational, -1, printValue},         
    }; 
        
    const TagInfo* CanonMakerNote::tagListAfMiAdj()
    {
        return tagInfoAfMiAdj_;
    }


     // Canon VignettingCorr Tag 
    constexpr TagInfo CanonMakerNote::tagInfoVigCor_[] = {
         {0x0000, "VignettingCorrVersion", N_("Vignetting Corr Version"), N_("Vignetting Corr Version"), canonVigCorId, makerTags, unsignedShort, -1, printValue},
         {0x0002, "PeripheralLighting", N_("Peripheral Lighting"), N_("Peripheral Lighting"), canonVigCorId, makerTags, signedShort, -1, EXV_PRINT_TAG(canonOffOn)},    
         {0x0003, "DistortionCorrection", N_("Distortion Correction"), N_("Distortion Correction"), canonVigCorId, makerTags, signedShort, -1, EXV_PRINT_TAG(canonOffOn)},
         {0x0004, "ChromaticAberrationCorr", N_("Chromatic Aberration Corr"), N_("Chromatic Aberration Corr"), canonVigCorId, makerTags, signedShort, -1, EXV_PRINT_TAG(canonOffOn)},
         {0x0005, "ChromaticAberrationCorr", N_("Chromatic Aberration Corr"), N_("Chromatic Aberration Corr"), canonVigCorId, makerTags, signedShort, -1, EXV_PRINT_TAG(canonOffOn)},
         {0x0006, "PeripheralLightingValue", N_("Peripheral Lighting Value"), N_("Peripheral Lighting Value"), canonVigCorId, makerTags, signedShort, -1, printValue},
         {0x0009, "DistortionCorrectionValue", N_("Distortion Correction Value"), N_("Distortion Correction Value"), canonVigCorId, makerTags, signedShort, -1, printValue},
         {0x000b, "OriginalImageWidth", N_("Original Image Width"), N_("Original Image Width"), canonVigCorId, makerTags, signedShort, -1, printValue},
         {0x000c, "OriginalImageHeight", N_("Original Image Height"), N_("Original Image Height"), canonVigCorId, makerTags, signedShort, -1, printValue},
    };
        
    const TagInfo* CanonMakerNote::tagListVigCor()
    {
        return tagInfoVigCor_;
    } 

    // Canon VignettingCorr2 Tag 
    constexpr TagInfo CanonMakerNote::tagInfoVigCor2_[] = {
         {0x0005, "PeripheralLightingSetting", N_("Peripheral Lighting Setting"), N_("Peripheral Lighting Setting"), canonVigCor2Id, makerTags, signedLong, -1, EXV_PRINT_TAG(canonOffOn)},
         {0x0006, "ChromaticAberrationSetting", N_("Chromatic Aberration Setting"), N_("Chromatic Aberration Setting"), canonVigCor2Id, makerTags, signedLong, -1, EXV_PRINT_TAG(canonOffOn)},    
         {0x0007, "DistortionCorrectionSetting", N_("Distortion Correction Setting"), N_("Distortion Correction Setting"), canonVigCor2Id, makerTags, signedLong, -1, EXV_PRINT_TAG(canonOffOn)},         
    };
        
    const TagInfo* CanonMakerNote::tagListVigCor2()
    {
        return tagInfoVigCor2_;
    }

     // Canon AutoLightingOptimizer, tag 0x0002
    constexpr TagDetails canonAutoLightingOptimizer[] = {
        { 0, N_("Standard") },
        { 1, N_("Low") },
        { 2, N_("Strong") },
        { 2, N_("Off") }
    };  

    // Canon HighISONoiseReduction, tag 0x0004
    constexpr TagDetails canonLongExposureNoiseReduction[] = {
        { 0, N_("Off") },
        { 1, N_("Auto") },
        { 2, N_("On") }        
    }; 

    // Canon HighISONoiseReduction, tag 0x0005
    constexpr TagDetails canonHighISONoiseReduction[] = {
        { 0, N_("Standard") },
        { 1, N_("Low") },
        { 2, N_("Strong") },
        { 2, N_("off") }
    };  

     // Canon LightingOpt Tag 
    constexpr TagInfo CanonMakerNote::tagInfoLiOp_[] = {
         {0x0001, "PeripheralIlluminationCorr", N_("Peripheral Lighting Setting"), N_("Peripheral Lighting Setting"), canonLiOpId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonOffOn)},
         {0x0002, "AutoLightingOptimizer", N_("Chromatic Aberration Setting"), N_("Chromatic Aberration Setting"), canonLiOpId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAutoLightingOptimizer)},    
         {0x0003, "HighlightTonePriority", N_("Distortion Correction Setting"), N_("Distortion Correction Setting"), canonLiOpId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonOffOn)},         
         {0x0004, "LongExposureNoiseReduction", N_("Distortion Correction Setting"), N_("Distortion Correction Setting"), canonLiOpId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonLongExposureNoiseReduction)},         
         {0x0005, "HighISONoiseReduction", N_("Distortion Correction Setting"), N_("Distortion Correction Setting"), canonLiOpId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonHighISONoiseReduction)},         
    };
        
    const TagInfo* CanonMakerNote::tagListLiOp()
    {
        return tagInfoLiOp_;
    }

    // Canon LensInfo Tag
    constexpr TagInfo CanonMakerNote::tagInfoLe_[] = {
         {0x0000, "LensSerialNumber", N_("Lens Seria lNumber"), N_("Lens Serial Number"), canonLeId, makerTags, asciiString, -1, printValue},         
    };
        
    const TagInfo* CanonMakerNote::tagListLe()
    {
        return tagInfoLe_;
    } 


    // Canon AmbienceSelection, tag 0x0001
    constexpr TagDetails canonAmbienceSelection[] = {
        { 0, N_("Standard") },
        { 1, N_("Vivid") },
        { 2, N_("Warm") },
        { 3, N_("Soft") },
        { 4, N_("Cool") },
        { 5, N_("Intense") },
        { 6, N_("Brighter") },
        { 7, N_("Darker") },
        { 8, N_("Monochrome") }
    };  


    // Canon Ambience Tag
    constexpr TagInfo CanonMakerNote::tagInfoAm_[] = {
         {0x0001, "AmbienceSelection", N_("Ambience Selection"), N_("Ambience Selection"), canonAmId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAmbienceSelection)},         
    };
        
    const TagInfo* CanonMakerNote::tagListAm()
    {
        return tagInfoAm_;
    } 

    // Canon MultiExposure, tag 0x0001
    constexpr TagDetails canonMultiExposure[] = {
        { 0, N_("Off") },
        { 1, N_("On") },
        { 2, N_("On (RAW)") }
    }; 

    // Canon MultiExposureControl, tag 0x0001
    constexpr TagDetails canonMultiExposureControl[] = {
        { 0, N_("Additive") },
        { 1, N_("Average") },
        { 2, N_("Bright (comparative)") },
        { 3, N_("Dark (comparative)") }
    }; 

    // Canon MultiExp Tag
    constexpr TagInfo CanonMakerNote::tagInfoMe_[] = {
         {0x0001, "MultiExposure", N_("Multi Exposure"), N_("Multi Exposure"), canonMeId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonMultiExposure)},
         {0x0002, "MultiExposureControl", N_("Multi Exposure Control"), N_("Multi Exposure Control"), canonMeId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonMultiExposureControl)},
         {0x0003, "MultiExposureShots", N_("Multi Exposure Shots"), N_("Multi Exposure Shots"), canonMeId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonMultiExposure)},
    };
        
    const TagInfo* CanonMakerNote::tagListMe()
    {
        return tagInfoMe_;
    }

    // Canon FilterInfo, tag 0x0001
    constexpr TagDetails canonFilterInfo[] = {
        { -1, N_("Off") }
    }; 

    // Canon MiniatureFilterOrientation, tag 0x0001
    constexpr TagDetails canonMiniatureFilterOrientation[] = {
        { 0, N_("Horizontal") },
        { 1, N_("Vertical") }
    }; 

    //  Canon Filter Info Tag
    constexpr TagInfo CanonMakerNote::tagInfoFil_[] = {
         {0x0101, "GrainyBWFilter", N_("Grainy BW Filter"), N_("Grainy BW Filter"), canonFilId, makerTags, asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
         {0x0201, "SoftFocusFilter", N_("Soft Focus Filter"), N_("Soft Focus Filter"), canonFilId, makerTags, asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
         {0x0301, "ToyCameraFilter", N_("Toy Camera Filter"), N_("Toy Camera Filter"), canonFilId, makerTags, asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
         {0x0401, "MiniatureFilter", N_("Miniature Filter"), N_("Miniature Filter"), canonFilId, makerTags, asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
         {0x0402, "MiniatureFilterOrientation", N_("Miniature Filter Orientation"), N_("Miniature Filter Orientation"), canonFilId, makerTags, asciiString, -1, EXV_PRINT_TAG(canonMiniatureFilterOrientation)},                
         {0x0403, "MiniatureFilterPosition", N_("Miniature Filter Position"), N_("Miniature Filter Position"), canonFilId, makerTags, asciiString, -1, printValue},
         {0x0404, "MiniatureFilterParameter", N_("Miniature Filter Parameter"), N_("Miniature Filter Parameter"), canonFilId, makerTags, asciiString, -1, printValue},
         {0x0501, "FisheyeFilter", N_("Fisheye Filter"), N_("Fisheye Filter"), canonFilId, makerTags, asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
         {0x0601, "PaintingFilter", N_("Painting Filter"), N_("Painting Filter"), canonFilId, makerTags, asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
         {0x0701, "WatercolorFilter", N_("Watercolor Filter"), N_("Watercolor Filter"), canonFilId, makerTags, asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
    }; 
      
        
    const TagInfo* CanonMakerNote::tagListFil()
    {
        return tagInfoFil_;
    } 


     // Canon HDR, tag 0x0001
    constexpr TagDetails canonHdr[] = {
        { 0, N_("Off") },
        { 1, N_("On") },
        { 2, N_("On (RAW") }
    }; 

    // Canon HDREffect, tag 0x0001
    constexpr TagDetails canonHdrEffect[] = {
        { 0, N_("Natural") },
        { 1, N_("Art (standard)") },
        { 2, N_("Art (vivid)") },
        { 3, N_("Art (bold)") },
        { 4, N_("Art (embossed)") }
    }; 

    // Canon HDR Info Tag
   constexpr TagInfo CanonMakerNote::tagInfoHdr_[] = {
         {0x0001, "HDR", N_("HDR"), N_("HDR"), canonHdrId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonHdr)},
         {0x0002, "HDREffect", N_("HDR Effect"), N_("HDR Effect"), canonHdrId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonHdrEffect)},    
    };
      
        
    const TagInfo* CanonMakerNote::tagListHdr()
    {
        return tagInfoHdr_;
    }

     // Canon AIServoFirstImage, tag 0x0001
    constexpr TagDetails canonAIServoFirstImage[] = {
        { 0, N_("Equal Priority") },
        { 1, N_("Release Priority") },
        { 2, N_("Focus Priority") }
    }; 

         // Canon AIServoSecondImage, tag 0x0001
    constexpr TagDetails canonAIServoSecondImage[] = {
        { 0, N_("Equal Priority") },
        { 1, N_("Release Priority") },
        { 2, N_("Focus Priority") },
        { 3, N_("Release High Priority") },
        { 4, N_("Focus High Priority") },
    }; 
    
    // Canon USMLensElectronicMF, tag 0x0001
    constexpr TagDetails canonUSMLensElectronicMF[] = {
        { 0, N_("Enable After AF") },
        { 1, N_("Disable After AF") },
        { 2, N_("Disable in AF Mode") }
    }; 

    // Canon AFAssistBeam, tag 0x0001
    constexpr TagDetails canonAFAssistBeam[] = {
        { 0, N_("Enable") },
        { 1, N_("Disable") },
        { 2, N_("IR AF Assist Beam Mode") }
    }; 

    // Canon OneShotAFRelease, tag 0x0001
    constexpr TagDetails canonOneShotAFRelease[] = {
        { 0, N_("Focus Priority") },
        { 1, N_("Release Priortiy") }        
    }; 

        // Canon AutoAFPointSelEOSiTRAF, tag 0x0001
    constexpr TagDetails canonAutoAFPointSelEOSiTRAF[] = {
        { 0, N_("Enable") },
        { 1, N_("Disable") }      
    };

    // Canon LensDriveWhenAFImpossible, tag 0x0001
    constexpr TagDetails canonLensDriveWhenAFImpossible[] = {
        { 0, N_("Continue Focus Search") },
        { 1, N_("Stop Focus Search") }        
    };

    // Canon SelectAFAreaSelectionMode, tag 0x0001
    constexpr TagDetails canonSelectAFAreaSelectionMode[] = {
        { 0, N_("Single-Point-AF") },
        { 1, N_("Auto") },      
        { 2, N_("Zone AF") },
        { 3, N_("AF Point Expansion (4 point)") },
        { 4, N_("Spot AF") },
        { 5, N_("AF Point Expansion (8 point)") }
    };

    // Canon AFAreaSelectionMethod, tag 0x0001
    constexpr TagDetails canonAFAreaSelectionMethod[] = {
        { 0, N_("M-Fn Button") },
        { 1, N_("Main Dial") }        
    };

    // Canon OrientationLinkedAF, tag 0x0001
    constexpr TagDetails canonOrientationLinkedAF[] = {
        { 0, N_("Same for Vert/Horiz Points") },
        { 1, N_("Separate for Vert/Horiz Points") },      
        { 2, N_("Separate Area+Points") }
    };

    // Canon ManualAFPointSelPattern, tag 0x0001
    constexpr TagDetails canonManualAFPointSelPattern[] = {
        { 0, N_("Stops at AF Area Edges") },
        { 1, N_("Continous") }        
    };

    // Canon AFPointDisplayDuringFocus, tag 0x0001
    constexpr TagDetails canonAFPointDisplayDuringFocus[] = {
        { 0, N_("Selected (constant)") },
        { 1, N_("All (constant)") },        
        { 1, N_("Selected (pre-AF, focused)") },
        { 1, N_("Selected (focused)") },
        { 1, N_("Disabled") }
    };

    // Canon VFDisplayIllumination, tag 0x0001
    constexpr TagDetails canonAVFDisplayIllumination[] = {
        { 0, N_("Auto") },
        { 1, N_("Enable") },
        { 2, N_("Disable") },    
    };

    // Canon AFStatusViewfinder, tag 0x0001
    constexpr TagDetails canonAFStatusViewfinder[] = {
        { 0, N_("Auto") },
        { 1, N_("Enable") },
        { 2, N_("Disable") },    
    };

       // Canon InitialAFPointInServo, tag 0x0001
    constexpr TagDetails canonInitialAFPointInServo[] = {
        { 0, N_("Initial AF Point Selected") },
        { 1, N_("Manual AF Point") },
        { 2, N_("Auto") },    
    };

    //Canon AFConfig Tags
    constexpr TagInfo CanonMakerNote::tagInfoAfC_[] = {
         {0x0001, "AFConfigTool", N_("AF Config Tool"), N_("AF Config Tool"), canonAfCId, makerTags, signedLong, -1, printValue},
         {0x0002, "AFTrackingSensitivity", N_("AF Tracking Sensitivity"), N_("AFTrackingSensitivity"), canonAfCId, makerTags, signedLong, -1, printValue},
         {0x0003, "AFAccelDecelTracking", N_("AF Accel Decel Tracking"), N_("AF Accel Decel Tracking"), canonAfCId, makerTags, signedLong, -1, printValue},
         {0x0004, "AFPointSwitching", N_("AF PointS witching"), N_("AF Point Switching"), canonAfCId, makerTags, signedLong, -1, printValue},
         {0x0005, "AIServoFirstImage", N_("AI Servo First Image"), N_("AI Servo First Image"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAIServoFirstImage)},
         {0x0006, "AIServoSecondImage", N_("AI Servo Second Image"), N_("AI Servo Second Image"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAIServoSecondImage)},
         {0x0007, "USMLensElectronicMF", N_("USM Lens Electronic MF"), N_("USM Lens Electronic MF"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonUSMLensElectronicMF)},
         {0x0008, "AFAssistBeam", N_("AF Assist Beam"), N_("AF Assist Beam"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAFAssistBeam)},
         {0x0009, "OneShotAFRelease", N_("One Shot AF Release"), N_("One Shot AF Release"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonOneShotAFRelease)},
         {0x000a, "AutoAFPointSelEOSiTRAF", N_("Auto AF Point Sel EOS iTRAF"), N_("Auto AF Point Sel EOS iTRAF"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAutoAFPointSelEOSiTRAF)},
         {0x000b, "LensDriveWhenAFImpossible", N_("Lens Drive When AF Impossible"), N_("Lens Drive When AF Impossible"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonLensDriveWhenAFImpossible)},
         {0x000c, "SelectAFAreaSelectionMode", N_("Select AF Area Selection Mode"), N_("Select AF Area Selection Mode"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonSelectAFAreaSelectionMode)},
         {0x000d, "AFAreaSelectionMethod", N_("AF Area Selection Method"), N_("AF Area Selection Method"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAFAreaSelectionMethod)},
         {0x000e, "OrientationLinkedAF", N_("Orientation Linked AF"), N_("Orientation Linked AF"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonOrientationLinkedAF)},
         {0x000f, "ManualAFPointSelPattern", N_("Manual AF Point Sel Pattern"), N_("Manual AF Point Sel Pattern"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonManualAFPointSelPattern)},
         {0x0010, "AFPointDisplayDuringFocus", N_("AF Point Display During Focus"), N_("AF Point Display During Focus"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAFPointDisplayDuringFocus)},
         {0x0011, "VFDisplayIllumination", N_("VF Display Illumination"), N_("VF Display Illumination"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAVFDisplayIllumination)},
         {0x0012, "AFStatusViewfinder", N_("AF Status Viewfinder"), N_("AF Status Viewfinder"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonAFStatusViewfinder)},
         {0x0013, "InitialAFPointInServo", N_("Initial AF Point In Servo"), N_("Initial AF Point In Servo"), canonAfCId, makerTags, signedLong, -1, EXV_PRINT_TAG(canonInitialAFPointInServo)},         
    };

    const TagInfo* CanonMakerNote::tagListAfC()
    {
        return tagInfoAfC_;
    }



   // Canon RawBurstInfo Info Tag
   constexpr TagInfo CanonMakerNote::tagInfoRawB_[] = {
         {0x0001, "RawBurstImageNum", N_("Raw Burst Image Num"), N_("Raw Burst Image Num"), canonRawBId, makerTags, unsignedLong, -1, printValue},
         {0x0002, "RawBurstImageCount", N_("Raw Burst Image Count"), N_("Raw Burst Image Count"), canonRawBId, makerTags, unsignedLong, -1, printValue}    
    };

    const TagInfo* CanonMakerNote::tagListRawB()
    {
        return tagInfoRawB_;
    }

    //! Macro, tag 0x0001
    constexpr TagDetails canonCsMacro[] = {
        { 1, N_("On")  },
        { 2, N_("Off") }
    };

    //! Quality, tag 0x0003
    constexpr TagDetails canonCsQuality[] = {
        { -1,  N_("n/a")          },
        { 1,   N_("Economy")      },
        { 2,   N_("Normal")       },
        { 3,   N_("Fine")         },
        { 4,   N_("RAW")          },
        { 5,   N_("Superfine")    },
        { 7,   N_("CRAW")         },
        { 130, N_("Normal Movie") },
        { 131, N_("Movie (2)")    }
    };

    //! FlashMode, tag 0x0004
    constexpr TagDetails canonCsFlashMode[] = {
        {  0, N_("Off")            },
        {  1, N_("Auto")           },
        {  2, N_("On")             },
        {  3, N_("Red-eye")        },
        {  4, N_("Slow sync")      },
        {  5, N_("Auto + red-eye") },
        {  6, N_("On + red-eye")   },
        { 16, N_("External")       },
        { 16, N_("External")       }    // To silence compiler warning
    };

    //! DriveMode, tag 0x0005
    constexpr TagDetails canonCsDriveMode[] = {
        {  0, N_("Single / timer")             },
        {  1, N_("Continuous")                 },
        {  2, N_("Movie")                      },
        {  3, N_("Continuous, speed priority") },
        {  3, N_("Continuous, tracking priority") },
        {  4, N_("Continuous, low")            },
        {  5, N_("Continuous, high")           },
        {  6, N_("Silent Single")              },
        {  9, N_("Single, Silent")             },
        { 10, N_("Continuous, Silent")         }
    };

    //! FocusMode, tag 0x0007
    constexpr TagDetails canonCsFocusMode[] = {
        {   0, N_("One shot AF")      },
        {   1, N_("AI servo AF")      },
        {   2, N_("AI focus AF")      },
        {   3, N_("Manual focus (3)") },
        {   4, N_("Single")           },
        {   5, N_("Continuous")       },
        {   6, N_("Manual focus (6)") },
        {  16, N_("Pan focus")        },
        { 256, N_("AF + MF")          },
        { 512, N_("Movie Snap Focus") },
        { 519, N_("Movie Servo AF")   },
        { 519, N_("Movie Servo AF")   }    // To silence compiler warning
    };

   //! RecordMode, tag 0x0009
    constexpr TagDetails canonCsRecordMode[] = {
        {   1, N_("JPEG")               },
        {   2, N_("CRW+THM")            },
        {   3, N_("AVI+THM")            },
        {   4, N_("TIF")                },
        {   5, N_("TIF+JPEG")           },
        {   6, N_("CR2")                },
        {   7, N_("CR2+JPEG")           },
        {  9, N_("MOV")                 },
        { 10, N_("MP4")                 },
        { 11, N_("CRM")                 },
        { 12, N_("CR3")                 },
        { 13, N_("CR3+JPEG")            },    
        { 14, N_("HIF")                 },    
        { 15, N_("CR3+HIF")             }   
    };

    //! ImageSize, tag 0x000a
    constexpr TagDetails canonCsImageSize[] = {
        {   0, N_("Large")             },
        {   1, N_("Medium")            },
        {   2, N_("Small")             },
        {   5, N_("Medium 1")          },
        {   6, N_("Medium 2")          },
        {   7, N_("Medium 3")          },
        {   8, N_("Postcard")          },
        {   9, N_("Widescreen")        },
        {  10, N_("Medium Widescreen") },
        {  14, N_("Small 1")           },
        {  15, N_("Small 2")           },
        {  16, N_("Small 3")           },
        { 128, N_("640x480 Movie")     },
        { 129, N_("Medium Movie")      },
        { 130, N_("Small Movie")       },
        { 137, N_("1280x720 Movie")    },
        { 142, N_("1920x1080 Movie")   }
    };

    //! EasyMode, tag 0x000b
    constexpr TagDetails canonCsEasyMode[] = {
        {   0, N_("Full auto")              },
        {   1, N_("Manual")                 },
        {   2, N_("Landscape")              },
        {   3, N_("Fast shutter")           },
        {   4, N_("Slow shutter")           },
        {   5, N_("Night")                  },
        {   6, N_("Gray Scale")             },
        {   7, N_("Sepia")                  },
        {   8, N_("Portrait")               },
        {   9, N_("Sports")                 },
        {  10, N_("Macro")                  },
        {  11, N_("Black & White")          },
        {  12, N_("Pan focus")              },
        {  13, N_("Vivid")                  },
        {  14, N_("Neutral")                },
        {  15, N_("Flash Off")              },
        {  16, N_("Long Shutter")           },
        {  17, N_("Super Macro")            },
        {  18, N_("Foliage")                },
        {  19, N_("Indoor")                 },
        {  20, N_("Fireworks")              },
        {  21, N_("Beach")                  },
        {  22, N_("Underwater")             },
        {  23, N_("Snow")                   },
        {  24, N_("Kids & Pets")            },
        {  25, N_("Night Snapshot")         },
        {  26, N_("Digital Macro")          },
        {  27, N_("My Colors")              },
        {  28, N_("Movie Snap")             },
        {  29, N_("Super Macro 2")          },
        {  30, N_("Color Accent")           },
        {  31, N_("Color Swap")             },
        {  32, N_("Aquarium")               },
        {  33, N_("ISO 3200")               },
        {  34, N_("ISO 6400")               },
        {  35, N_("Creative Light Effect")  },
        {  36, N_("Easy")                   },
        {  37, N_("Quick Shot")             },
        {  38, N_("Creative Auto")          },
        {  39, N_("Zoom Blur")              },
        {  40, N_("Low Light")              },
        {  41, N_("Nostalgic")              },
        {  42, N_("Super Vivid")            },
        {  43, N_("Poster Effect")          },
        {  44, N_("Face Self-timer")        },
        {  45, N_("Smile")                  },
        {  46, N_("Wink Self-timer")        },
        {  47, N_("Fisheye Effect")         },
        {  48, N_("Miniature Effect")       },
        {  49, N_("High-speed Burst")       },
        {  50, N_("Best Image Selection")   },
        {  51, N_("High Dynamic Range")     },
        {  52, N_("Handheld Night Scene")   },
        {  53, N_("Movie Digest")           },
        {  54, N_("Live View Control")      },
        {  55, N_("Discreet")               },
        {  56, N_("Blur Reduction")         },
        {  57, N_("Monochrome")             },
        {  58, N_("Toy Camera Effect")      },
        {  59, N_("Scene Intelligent Auto") },
        {  60, N_("High-speed Burst HQ")    },
        {  61, N_("Smooth Skin")            },
        {  62, N_("Soft Focus")             },
        { 257, N_("Spotlight")              },
        { 258, N_("Night 2")                },
        { 259, N_("Night+")                 },
        { 260, N_("Super Night")            },
        { 261, N_("Sunset")                 },
        { 263, N_("Night Scene")            },
        { 264, N_("Surface")                },
        { 265, N_("Low Light 2")            }
    };

    //! DigitalZoom, tag 0x000c
    constexpr TagDetails canonCsDigitalZoom[] = {
        { 0, N_("None")  },
        { 1, "2x"        },
        { 2, "4x"        },
        { 3, N_("Other") },
        { 3, N_("Other") }                      // To silence compiler warning
    };

    //! Contrast, Saturation Sharpness, tags 0x000d, 0x000e, 0x000f
    constexpr TagDetails canonCsLnh[] = {
        { 0xffff, N_("Low")    },
        { 0x0000, N_("Normal") },
        { 0x0001, N_("High")   }
    };

    //! ISOSpeeds, tag 0x0010
    constexpr TagDetails canonCsISOSpeed[] = {
        {     0, N_("n/a")       },
        {    14, N_("Auto High") },
        {    15, N_("Auto")      },
        {    16,    "50"         },
        {    17,   "100"         },
        {    18,   "200"         },
        {    19,   "400"         },
        {    20,   "800"         },
        { 16464,    "80"         },
        { 16484,   "100"         },
        { 16509,   "125"         },
        { 16544,   "160"         },
        { 16584,   "200"         },
        { 16634,   "250"         },
        { 16704,   "320"         },
        { 16784,   "400"         },
        { 16884,   "500"         },
        { 17024,   "640"         },
        { 17184,   "800"         },
        { 17384,  "1000"         },
        { 17634,  "1250"         },
        { 17984,  "1600"         },
        { 18384,  "2000"         },
        { 18884,  "2500"         },
        { 19584,  "3200"         },
        { 20384,  "4000"         },
        { 21384,  "5000"         },
        { 22784,  "6400"         },
        { 24384,  "8000"         },
        { 26384, "10000"         },
        { 29184, "12800"         },        
        { 29184, "16000"         }, 
        { 29184, "20000"         }, 
        { 29184, "25600"         }, 
        { 29184, "32000"         }, 
        { 29184, "40000"         }, 
        { 29184, "H1(51200)"     }, 
        { 29184, "H2(102400)"    } 
    };

    //! MeteringMode, tag 0x0011
    constexpr TagDetails canonCsMeteringMode[] = {
        { 0, N_("Default")                 },
        { 1, N_("Spot")                    },
        { 2, N_("Average")                 },
        { 3, N_("Evaluative")              },
        { 4, N_("Partial")                 },
        { 5, N_("Center-weighted average") }
    };

    //! FocusType, tag 0x0012
    constexpr TagDetails canonCsFocusType[] = {
        {  0, N_("Manual")       },
        {  1, N_("Auto")         },
        {  2, N_("Not known")    },
        {  3, N_("Macro")        },
        {  4, N_("Very close")   },
        {  5, N_("Close")        },
        {  6, N_("Middle range") },
        {  7, N_("Far range")    },
        {  8, N_("Pan focus")    },
        {  9, N_("Super macro")  },
        { 10, N_("Infinity")     }
    };

    //! AFPoint, tag 0x0013
    constexpr TagDetails canonCsAfPoint[] = {
        { 0x2005, N_("Manual AF point selection") },
        { 0x3000, N_("None (MF)")                 },
        { 0x3001, N_("Auto-selected")             },
        { 0x3002, N_("Right")                     },
        { 0x3003, N_("Center")                    },
        { 0x3004, N_("Left")                      },
        { 0x4001, N_("Auto AF point selection")   },
        { 0x4006, N_("Face Detect")               }
    };

    //! ExposureProgram, tag 0x0014
    constexpr TagDetails canonCsExposureProgram[] = {
        { 0, N_("Easy shooting (Auto)")   },
        { 1, N_("Program (P)")            },
        { 2, N_("Shutter priority (Tv)")  },
        { 3, N_("Aperture priority (Av)") },
        { 4, N_("Manual (M)")             },
        { 5, N_("A-DEP")                  },
        { 6, N_("M-DEP")                  },
        { 7, N_("Bulb")                   }
    };

    //! LensType, tag 0x0016
    constexpr TagDetails canonCsLensType[] = {
        {   -1, "n/a"                                                       },
        {    1, "Canon EF 50mm f/1.8"                                       },
        {    2, "Canon EF 28mm f/2.8"                                       },
        {    2, "Sigma 24mm f/2.8 Super Wide II"                            }, // 1
        {    3, "Canon EF 135mm f/2.8 Soft"                                 },
        {    4, "Canon EF 35-105mm f/3.5-4.5"                               },
        {    4, "Sigma UC Zoom 35-135mm f/4-5.6"                            }, // 1
        {    5, "Canon EF 35-70mm f/3.5-4.5"                                },
        {    6, "Canon EF 28-70mm f/3.5-4.5"                                },
        {    6, "Sigma 18-50mm f/3.5-5.6 DC"                                }, // 1
        {    6, "Sigma 18-125mm f/3.5-5.6 DC IF ASP"                        }, // 2
        {    6, "Tokina AF 193-2 19-35mm f/3.5-4.5"                         }, // 3
        {    6, "Sigma 28-80mm f/3.5-5.6 II Macro"                          }, // 4
        {    6, "Sigma 28-300mm f/3.5-6.3 DG Macro"                         }, // 5
        {    7, "Canon EF 100-300mm f/5.6L"                                 },
        {    8, "Canon EF 100-300mm f/5.6"                                  },
        {    8, "Sigma 70-300mm f/4-5.6 [APO] DG Macro"                     }, // 1
        {    8, "Tokina AT-X 242 AF 24-200mm f/3.5-5.6"                     }, // 2
        {    9, "Canon EF 70-210mm f/4"                                     },
        {    9, "Sigma 55-200mm f/4-5.6 DC"                                 }, // 1
        {   10, "Canon EF 50mm f/2.5 Macro"                                 },
        {   10, "Sigma 50mm f/2.8 EX"                                       }, // 1
        {   10, "Sigma 28mm f/1.8"                                          }, // 2
        {   10, "Sigma 105mm f/2.8 Macro EX"                                }, // 3
        {   10, "Sigma 70mm f/2.8 EX DG Macro EF"                           }, // 4
        {   11, "Canon EF 35mm f/2"                                         },
        {   13, "Canon EF 15mm f/2.8 Fisheye"                               },
        {   14, "Canon EF 50-200mm f/3.5-4.5L"                              },
        {   15, "Canon EF 50-200mm f/3.5-4.5"                               },
        {   16, "Canon EF 35-135mm f/3.5-4.5"                               },
        {   17, "Canon EF 35-70mm f/3.5-4.5A"                               },
        {   18, "Canon EF 28-70mm f/3.5-4.5"                                },
        {   20, "Canon EF 100-200mm f/4.5A"                                 },
        {   21, "Canon EF 80-200mm f/2.8L"                                  },
        {   22, "Canon EF 20-35mm f/2.8L"                                   },
        {   22, "Tokina AT-X 280 AF Pro 28-80mm f/2.8 Aspherical"           }, // 1
        {   23, "Canon EF 35-105mm f/3.5-4.5"                               },
        {   24, "Canon EF 35-80mm f/4-5.6 Power Zoom"                       },
        {   25, "Canon EF 35-80mm f/4-5.6 Power Zoom"                       },
        {   26, "Canon EF 100mm f/2.8 Macro"                                },
        {   26, "Cosina 100mm f/3.5 Macro AF"                               }, // 1
        {   26, "Tamron SP AF 90mm f/2.8 Di Macro"                          }, // 2
        {   26, "Tamron SP AF 180mm f/3.5 Di Macro"                         }, // 3
        {   26, "Carl Zeiss Planar T* 50mm f/1.4"                           }, // 4
        {   26, "Voigtlander APO Lanthar 125mm F2.5 SL Macro"               }, // 5
        {   26, "Carl Zeiss Planar T 85mm f/1.4 ZE"                         }, // 6
        {   27, "Canon EF 35-80mm f/4-5.6"                                  },
        {   28, "Canon EF 80-200mm f/4.5-5.6"                               },
        {   28, "Tamron SP AF 28-105mm f/2.8 LD Aspherical IF"              }, // 1
        {   28, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro" }, // 2
        {   28, "Tamron AF 70-300mm f/4-5.6 Di LD 1:2 Macro"                }, // 3
        {   28, "Tamron AF Aspherical 28-200mm f/3.8-5.6"                   }, // 4
        {   29, "Canon EF 50mm f/1.8 II"                                    },
        {   30, "Canon EF 35-105mm f/4.5-5.6"                               },
        {   31, "Canon EF 75-300mm f/4-5.6"                                 },
        {   31, "Tamron SP AF 300mm f/2.8 LD IF"                            }, // 1
        {   32, "Canon EF 24mm f/2.8"                                       },
        {   32, "Sigma 15mm f/2.8 EX Fisheye"                               }, // 1
        {   33, "Voigtlander Ultron 40mm f/2 SLII Aspherical"               },
        {   33, "Voigtlander Color Skopar 20mm f/3.5 SLII Aspherical"       }, // 1
        {   33, "Voigtlander APO-Lanthar 90mm f/3.5 SLII Close Focus"       }, // 2
        {   33, "Carl Zeiss Distagon T* 15mm f/2.8 ZE"                      }, // 3
        {   33, "Carl Zeiss Distagon T* 18mm f/3.5 ZE"                      }, // 4
        {   33, "Carl Zeiss Distagon T* 21mm f/2.8 ZE"                      }, // 5
        {   33, "Carl Zeiss Distagon T* 25mm f/2 ZE"                        }, // 6
        {   33, "Carl Zeiss Distagon T* 28mm f/2 ZE"                        }, // 7
        {   33, "Carl Zeiss Distagon T* 35mm f/2 ZE"                        }, // 8
        {   33, "Carl Zeiss Distagon T* 35mm f/1.4 ZE"                      }, // 9
        {   33, "Carl Zeiss Planar T* 50mm f/1.4 ZE"                        }, // 10
        {   33, "Carl Zeiss Makro-Planar T* 50mm f/2 ZE"                    }, // 11
        {   33, "Carl Zeiss Makro-Planar T* 100mm f/2 ZE"                   }, // 12
        {   33, "Carl Zeiss Apo-Sonnar T* 135mm f/2 ZE"                     }, // 13
        {   35, "Canon EF 35-80mm f/4-5.6"                                  },
        {   36, "Canon EF 38-76mm f/4.5-5.6"                                },
        {   37, "Canon EF 35-80mm f/4-5.6"                                  },
        {   37, "Tamron 70-200mm f/2.8 Di LD IF Macro"                      }, // 1
        {   37, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro"}, // 2
        {   37, "Tamron SP AF 17-50mm f/2.8 XR Di II VC LD Aspherical [IF]" }, // 3
        {   37, "Tamron AF 18-270mm f/3.5-6.3 Di II VC LD Aspherical [IF] Macro"}, // 4
        {   38, "Canon EF 80-200mm f/4.5-5.6"                               },
        {   39, "Canon EF 75-300mm f/4-5.6"                                 },
        {   40, "Canon EF 28-80mm f/3.5-5.6"                                },
        {   41, "Canon EF 28-90mm f/4-5.6"                                  },
        {   42, "Canon EF 28-200mm f/3.5-5.6"                               },
        {   42, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro"}, // 1
        {   43, "Canon EF 28-105mm f/4-5.6"                                 },
        {   44, "Canon EF 90-300mm f/4.5-5.6"                               },
        {   45, "Canon EF-S 18-55mm f/3.5-5.6 [II]"                         },
        {   46, "Canon EF 28-90mm f/4-5.6"                                  },
        {   47, "Zeiss Milvus 35mm f/2"                                     },
        {   47, "Zeiss Milvus 50mm f/2 Makro"                               }, // 1
        {   47, "Zeiss Milvus 135mm f/2 ZE"                                 }, // 2
        {   48, "Canon EF-S 18-55mm f/3.5-5.6 IS"                           },
        {   49, "Canon EF-S 55-250mm f/4-5.6 IS"                            },
        {   50, "Canon EF-S 18-200mm f/3.5-5.6 IS"                          },
        {   51, "Canon EF-S 18-135mm f/3.5-5.6 IS"                          },
        {   52, "Canon EF-S 18-55mm f/3.5-5.6 IS II"                        },
        {   53, "Canon EF-S 18-55mm f/3.5-5.6 III"                          },
        {   54, "Canon EF-S 55-250mm f/4-5.6 IS II"                         },
        {   60, "Irix 11mm f/4"                                             },
        {   80, "Canon TS-E 50mm f/2.8L Macro"                              },
        {   81, "Canon TS-E 90mm f/2.8L Macro"                              },
        {   82, "Canon TS-E 135mm f/4L Macro"                               },
        {   94, "Canon TS-E 17mm f/4L"                                      },
        {   95, "Canon TS-E 24mm f/3.5L II"                                 },
        {  103, "Samyang AF 14mm f/2.8 EF"                                  },
        {  103, "Rokinon SP 14mm f/2.4"                                     }, // 1
        {  103, "Rokinon AF 14mm f/2.8 EF"                                  }, // 2
        {  106, "Rokinon SP / Samyang XP 35mm f/1.2"                        },
        {  112, "Sigma 28mm f/1.5 FF High-speed Prime"                      },
        {  112, "Sigma 40mm f/1.5 FF High-speed Prime"                      }, // 1
        {  112, "Sigma 105mm f/1.5 FF High-speed Prime"                     }, // 2
        {  117, "Tamron 35-150mm f/2.8-4.0 Di VC OSD"                       },
        {  117, "Tamron SP 35mm f/1.4 Di USD"                               }, // 1
        {  124, "Canon MP-E 65mm f/2.8 1-5x Macro Photo"                    },
        {  125, "Canon TS-E 24mm f/3.5L"                                    },
        {  126, "Canon TS-E 45mm f/2.8"                                     },
        {  127, "Canon TS-E 90mm f/2.8"                                     },
        {  127, "Tamron 18-200mm f/3.5-6.3 Di II VC"                        }, // 1
        {  129, "Canon EF 300mm f/2.8L USM"                                 },
        {  130, "Canon EF 50mm f/1.0L USM"                                  },
        {  131, "Canon EF 28-80mm f/2.8-4L USM"                             },
        {  131, "Sigma 8mm f/3.5 EX DG Circular Fisheye"                    }, // 1
        {  131, "Sigma 17-35mm f/2.8-4 EX DG Aspherical HSM"                }, // 2
        {  131, "Sigma 17-70mm f/2.8-4.5 DC Macro"                          }, // 3
        {  131, "Sigma APO 50-150mm f/2.8 [II] EX DC HSM"                   }, // 4
        {  131, "Sigma APO 120-300mm f/2.8 EX DG HSM"                       }, // 5
        {  131, "Sigma 70-200mm f/2.8 APO EX HSM"                           }, // 6
        {  131, "Sigma 28-70mm f/2.8-4 DG"                                  }, // 7
        {  132, "Canon EF 1200mm f/5.6L USM"                                },
        {  134, "Canon EF 600mm f/4L IS USM"                                },
        {  135, "Canon EF 200mm f/1.8L USM"                                 },
        {  136, "Canon EF 300mm f/2.8L USM"                                 },
        {  136, "Tamron SP 15-30mm f/2.8 Di VC USD"                         }, // 1
        {  137, "Canon EF 85mm f/1.2L USM"                                  },
        {  137, "Sigma 18-50mm f/2.8-4.5 DC OS HSM"                         }, // 1
        {  137, "Sigma 50-200mm f/4-5.6 DC OS HSM"                          }, // 2
        {  137, "Sigma 18-250mm f/3.5-6.3 DC OS HSM"                        }, // 3
        {  137, "Sigma 24-70mm f/2.8 IF EX DG HSM"                          }, // 4
        {  137, "Sigma 18-125mm f/3.8-5.6 DC OS HSM"                        }, // 5
        {  137, "Sigma 17-70mm f/2.8-4 DC Macro OS HSM | C"                 }, // 6
        {  137, "Sigma 17-50mm f/2.8 OS HSM"                                }, // 7
        {  137, "Sigma 18-200mm f/3.5-6.3 DC OS HSM [II]"                   }, // 8
        {  137, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"                 }, // 9
        {  137, "Sigma 8-16mm f/4.5-5.6 DC HSM"                             }, // 10
        {  137, "Tamron SP 60mm f/2 Macro Di II"                            }, // 11
        {  137, "Sigma 10-20mm f/3.5 EX DC HSM"                             }, // 12
        {  137, "Sigma 18-35mm f/1.8 DC HSM | A"                            }, // 13
        {  137, "Sigma 12-24mm f/4.5-5.6 DG HSM II"                         }, // 14
        {  137, "Sigma 70-300mm f/4-5.6 DG OS"                              }, // 15
        {  138, "Canon EF 28-80mm f/2.8-4L"                                 },
        {  139, "Canon EF 400mm f/2.8L USM"                                 },
        {  140, "Canon EF 500mm f/4.5L USM"                                 },
        {  141, "Canon EF 500mm f/4.5L USM"                                 },
        {  142, "Canon EF 300mm f/2.8L IS USM"                              },
        {  143, "Canon EF 500mm f/4L IS USM"                                },
        {  143, "Sigma 17-70mm f/2.8-4 DC Macro OS HSM"                     }, // 1
        {  143, "Sigma 24-105mm F4 DG OS HSM | A"                           }, // 2
        {  144, "Canon EF 35-135mm f/4-5.6 USM"                             },
        {  145, "Canon EF 100-300mm f/4.5-5.6 USM"                          },
        {  146, "Canon EF 70-210mm f/3.5-4.5 USM"                           },
        {  147, "Canon EF 35-135mm f/4-5.6 USM"                             },
        {  148, "Canon EF 28-80mm f/3.5-5.6 USM"                            },
        {  149, "Canon EF 100mm f/2 USM"                                    },
        {  150, "Canon EF 14mm f/2.8L USM"                                  },
        {  150, "Sigma 20mm EX f/1.8"                                       }, // 1
        {  150, "Sigma 30mm f/1.4 DC HSM"                                   }, // 2
        {  150, "Sigma 24mm f/1.8 DG Macro EX"                              }, // 3
        {  150, "Sigma 28mm f/1.8 DG Macro EX"                              }, // 4
        {  150, "Sigma 18-35mm f/1.8 DC HSM | A"                            }, // 5
        {  151, "Canon EF 200mm f/2.8L USM"                                 },
        {  152, "Canon EF 300mm f/4L IS USM"                                },
        {  152, "Sigma 12-24mm f/4.5-5.6 EX DG ASPHERICAL HSM"              }, // 1
        {  152, "Sigma 14mm f/2.8 EX Aspherical HSM"                        }, // 2
        {  152, "Sigma 10-20mm f/4-5.6"                                     }, // 3
        {  152, "Sigma 100-300mm f/4"                                       }, // 4
        {  152, "Sigma 300-800mm f/5.6 APO EX DG HSM"                       }, // 5
        {  153, "Canon EF 35-350mm f/3.5-5.6L USM"                          },
        {  153, "Sigma 50-500mm f/4-6.3 APO HSM EX"                         }, // 1
        {  153, "Tamron AF 28-300mm f/3.5-6.3 XR LD Aspherical [IF] Macro"  }, // 2
        {  153, "Tamron AF 18-200mm f/3.5-6.3 XR Di II LD Aspherical [IF] Macro"}, // 3
        {  153, "Tamron 18-250mm f/3.5-6.3 Di II LD Aspherical [IF] Macro"  }, // 4
        {  154, "Canon EF 20mm f/2.8 USM"                                   },
        {  154, "Zeiss Milvus 21mm f/2.8"                                   }, // 1
        {  154, "Zeiss Milvus 15mm f/2.8 ZE"                                }, // 2
        {  154, "Zeiss Milvus 18mm f/2.8 ZE"                                }, // 3
        {  155, "Canon EF 85mm f/1.8 USM"                                   },
        {  155, "Sigma 14mm f/1.8 DG HSM | A"                               }, // 1
        {  156, "Canon EF 28-105mm f/3.5-4.5 USM"                           },
        {  156, "Tamron SP 70-300mm f/4-5.6 Di VC USD"                      }, // 1
        {  156, "Tamron SP AF 28-105mm f/2.8 LD Aspherical IF"              }, // 2
        {  160, "Canon EF 20-35mm f/3.5-4.5 USM"                            },
        {  160, "Tamron AF 19-35mm f/3.5-4.5"                               }, // 1
        {  160, "Tokina AT-X 124 AF Pro DX 12-24mm f/4"                     }, // 2
        {  160, "Tokina AT-X 107 AF DX 10-17mm f/3.5-4.5 Fisheye"           }, // 3
        {  160, "Tokina AT-X 116 AF Pro DX 11-16mm f/2.8"                   }, // 4
        {  160, "Tokina AT-X 11-20 F2.8 PRO DX Aspherical 11-20mm f/2.8"    }, // 5
        {  161, "Canon EF 28-70mm f/2.8L USM"                               },
        {  161, "Sigma 24-70mm f/2.8 EX"                                    }, // 1
        {  161, "Sigma 28-70mm f/2.8 EX"                                    }, // 2
        {  161, "Sigma 24-60mm f/2.8 EX DG"                                 }, // 3
        {  161, "Tamron AF 17-50mm f/2.8 Di-II LD Aspherical"               }, // 4
        {  161, "Tamron 90mm f/2.8"                                         }, // 5
        {  161, "Tamron SP AF 17-35mm f/2.8-4 Di LD Aspherical IF"          }, // 6
        {  161, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro" }, // 7
        {  162, "Canon EF 200mm f/2.8L USM"                                 },
        {  163, "Canon EF 300mm f/4L"                                       },
        {  164, "Canon EF 400mm f/5.6L"                                     },
        {  165, "Canon EF 70-200mm f/2.8L USM"                              },
        {  166, "Canon EF 70-200mm f/2.8L USM + 1.4x"                       },
        {  167, "Canon EF 70-200mm f/2.8L USM + 2x"                         },
        {  168, "Canon EF 28mm f/1.8 USM"                                   },
        {  168, "Sigma 50-100mm f/1.8 DC HSM | A"                           }, // 1
        {  169, "Canon EF 17-35mm f/2.8L USM"                               },
        {  169, "Sigma 18-200mm f/3.5-6.3 DC OS"                            }, // 1
        {  169, "Sigma 15-30mm f/3.5-4.5 EX DG Aspherical"                  }, // 2
        {  169, "Sigma 18-50mm f/2.8 Macro"                                 }, // 3
        {  169, "Sigma 50mm f/1.4 EX DG HSM"                                }, // 4
        {  169, "Sigma 85mm f/1.4 EX DG HSM"                                }, // 5
        {  169, "Sigma 30mm f/1.4 EX DC HSM"                                }, // 6
        {  169, "Sigma 35mm f/1.4 DG HSM"                                   }, // 7
        {  169, "Sigma 35mm f/1.5 FF High-Speed Prime | 017"                }, // 8
        {  169, "Sigma 70mm f/2.8 Macro EX DG"                              }, // 9
        {  170, "Canon EF 200mm f/2.8L II USM"                              },
        {  170, "Sigma 300mm f/2.8 APO EX DG HSM"                           }, // 1
        {  170, "Sigma 800mm f/5.6 APO EX DG HSM"                           }, // 2
        {  171, "Canon EF 300mm f/4L USM"                                   },
        {  172, "Canon EF 400mm f/5.6L USM"                                 },
        {  172, "Sigma 150-600mm f/5-6.3 DG OS HSM | S"                     }, // 1
        {  172, "Sigma 500mm f/4.5 APO EX DG HSM"                           }, // 2
        {  173, "Canon EF 180mm Macro f/3.5L USM"                           },
        {  173, "Sigma APO Macro 150mm f/2.8 EX DG HSM"                     }, // 1
        {  173, "Sigma 10mm f/2.8 EX DC Fisheye"                            }, // 2
        {  173, "Sigma 15mm f/2.8 EX DG Diagonal Fisheye"                   }, // 3
        {  173, "Venus Laowa 100mm F2.8 2X Ultra Macro APO"                 }, // 4
        {  173, "Sigma 150-500mm f/5-6.3 APO DG OS HSM + 2x"                }, // 5
        {  174, "Canon EF 135mm f/2L USM"                                   },
        {  174, "Sigma 70-200mm f/2.8 EX DG APO OS HSM"                     }, // 1
        {  174, "Sigma 50-500mm f/4.5-6.3 APO DG OS HSM"                    }, // 2
        {  174, "Sigma 150-500mm f/5-6.3 APO DG OS HSM"                     }, // 3
        {  174, "Zeiss Milvus 100mm f/2 Makro"                              }, // 4
        {  174, "Sigma APO 50-150mm f/2.8 EX DC OS HSM"                     }, // 5
        {  174, "Sigma APO 120-300mm f/2.8 EX DG OS HSM"                    }, // 6
        {  174, "Sigma 120-300mm f/2.8 DG OS HSM S013"                      }, // 7
        {  174, "Sigma 120-400mm f/4.5-5.6 APO DG OS HSM"                   }, // 8
        {  174, "Sigma 200-500mm f/2.8 APO EX DG"                           }, // 9
        {  175, "Canon EF 400mm f/2.8L USM"                                 },
        {  176, "Canon EF 24-85mm f/3.5-4.5 USM"                            },
        {  177, "Canon EF 300mm f/4L IS USM"                                },
        {  178, "Canon EF 28-135mm f/3.5-5.6 IS"                            },
        {  179, "Canon EF 24mm f/1.4L USM"                                  },
        {  180, "Canon EF 35mm f/1.4L USM"                                  },
        {  180, "Sigma 50mm f/1.4 DG HSM | A"                               }, // 1
        {  180, "Sigma 24mm f/1.4 DG HSM | A"                               }, // 2
        {  180, "Zeiss Milvus 85mm f/1.4"                                   }, // 3
        {  180, "Zeiss Otus 28mm f/1.4 ZE"                                  }, // 4
        {  180, "Sigma 24mm f/1.5 FF High-Speed Prime | 017"                }, // 5
        {  180, "Sigma 50mm f/1.5 FF High-Speed Prime | 017"                }, // 6
        {  180, "Sigma 85mm f/1.5 FF High-Speed Prime | 017"                }, // 7
        {  180, "Sigma 20mm f/1.4 DG HSM | A"                               }, // 8
        {  181, "Canon EF 100-400mm f/4.5-5.6L IS USM + 1.4x"               },
        {  181, "Sigma 150-600mm f/5-6.3 DG OS HSM | S + 1.4x"              }, // 1
        {  182, "Canon EF 100-400mm f/4.5-5.6L IS USM + 2x"                 },
        {  182, "Sigma 150-600mm f/5-6.3 DG OS HSM | S + 2x"                }, // 1
        {  183, "Canon EF 100-400mm f/4.5-5.6L IS USM"                      },
        {  183, "Sigma 150mm f/2.8 EX DG OS HSM APO Macro"                  }, // 1
        {  183, "Sigma 105mm f/2.8 EX DG OS HSM Macro"                      }, // 2
        {  183, "Sigma 180mm f/2.8 EX DG OS HSM APO Macro"                  }, // 3
        {  183, "Sigma 150-600mm f/5-6.3 DG OS HSM | C"                     }, // 4
        {  183, "Sigma 100-400mm f/5-6.3 DG OS HSM"                         }, // 6
        {  183, "Sigma 180mm f/3.5 APO Macro EX DG IF HSM"                  }, // 7
        {  184, "Canon EF 400mm f/2.8L USM + 2x"                            },
        {  185, "Canon EF 600mm f/4L IS USM"                                },
        {  186, "Canon EF 70-200mm f/4L USM"                                },
        {  187, "Canon EF 70-200mm f/4L USM + 1.4x"                         },
        {  188, "Canon EF 70-200mm f/4L USM + 2x"                           },
        {  189, "Canon EF 70-200mm f/4L USM + 2.8x"                         },
        {  190, "Canon EF 100mm f/2.8 Macro USM"                            },
        {  191, "Canon EF 400mm f/4 DO IS"                                  },
        {  191, "Sigma 500mm f/4 DG OS HSM"                                 }, // 1
        {  193, "Canon EF 35-80mm f/4-5.6 USM"                              },
        {  194, "Canon EF 80-200mm f/4.5-5.6 USM"                           },
        {  195, "Canon EF 35-105mm f/4.5-5.6 USM"                           },
        {  196, "Canon EF 75-300mm f/4-5.6 USM"                             },
        {  197, "Canon EF 75-300mm f/4-5.6 IS USM"                          },
        {  197, "Sigma 18-300mm f/3.5-6.3 DC Macro OS HSM"                  }, // 1
        {  198, "Canon EF 50mm f/1.4 USM"                                   },
        {  198, "Zeiss Otus 55mm f/1.4 ZE"                                  }, // 1
        {  198, "Zeiss Otus 85mm f/1.4 ZE"                                  }, // 2
        {  198, "Zeiss Milvus 25mm f/1.4"                                   }, // 3
        {  198, "Zeiss Otus 100mm f/1.4"                                    }, // 4
        {  198, "Zeiss Milvus 35mm f/1.4 ZE"                                }, // 5
        {  198, "Yongnuo YN 35mm f/2"                                       }, // 6
        {  199, "Canon EF 28-80mm f/3.5-5.6 USM"                            },
        {  200, "Canon EF 75-300mm f/4-5.6 USM"                             },
        {  201, "Canon EF 28-80mm f/3.5-5.6 USM"                            },
        {  202, "Canon EF 28-80mm f/3.5-5.6 USM IV"                         },
        {  208, "Canon EF 22-55mm f/4-5.6 USM"                              },
        {  209, "Canon EF 55-200mm f/4.5-5.6"                               },
        {  210, "Canon EF 28-90mm f/4-5.6 USM"                              },
        {  211, "Canon EF 28-200mm f/3.5-5.6 USM"                           },
        {  212, "Canon EF 28-105mm f/4-5.6 USM"                             },
        {  213, "Canon EF 90-300mm f/4.5-5.6 USM"                           },
        {  213, "Tamron SP 150-600mm f/5-6.3 Di VC USD"                     }, // 1
        {  213, "Tamron 16-300mm f/3.5-6.3 Di II VC PZD Macro"              }, // 2
        {  213, "Tamron SP 35mm f/1.8 Di VC USD"                            }, // 3
        {  213, "Tamron SP 45mm f/1.8 Di VC USD"                            }, // 4
        {  214, "Canon EF-S 18-55mm f/3.5-5.6 USM"                          },
        {  215, "Canon EF 55-200mm f/4.5-5.6 II USM"                        },
        {  217, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"                 },
        {  220, "Yongnuo YN 50mm f/1.8"                                     },
        {  224, "Canon EF 70-200mm f/2.8L IS USM"                           },
        {  225, "Canon EF 70-200mm f/2.8L IS USM + 1.4x"                    },
        {  226, "Canon EF 70-200mm f/2.8L IS USM + 2x"                      },
        {  227, "Canon EF 70-200mm f/2.8L IS USM + 2.8x"                    },
        {  228, "Canon EF 28-105mm f/3.5-4.5 USM"                           },
        {  229, "Canon EF 16-35mm f/2.8L USM"                               },
        {  230, "Canon EF 24-70mm f/2.8L USM"                               },
        {  231, "Canon EF 17-40mm f/4L USM"                                 },
        {  231, "Sigma 12-24mm f/4 DG HSM A016"                             }, // 1
        {  232, "Canon EF 70-300mm f/4.5-5.6 DO IS USM"                     },
        {  233, "Canon EF 28-300mm f/3.5-5.6L IS USM"                       },
        {  234, "Canon EF-S 17-85mm f/4-5.6 IS USM"                         },
        {  234, "Tokina AT-X 12-28 PRO DX 12-28mm f/4"                      }, // 1
        {  235, "Canon EF-S 10-22mm f/3.5-4.5 USM"                          },
        {  236, "Canon EF-S 60mm f/2.8 Macro USM"                           },
        {  237, "Canon EF 24-105mm f/4L IS USM"                             },
        {  238, "Canon EF 70-300mm f/4-5.6 IS USM"                          },
        {  239, "Canon EF 85mm f/1.2L II USM"                               },
        {  239, "Rokinon SP 85mm f/1.2"                                     }, // 1
        {  240, "Canon EF-S 17-55mm f/2.8 IS USM"                           },
        {  240, "Sigma 17-50mm f/2.8 EX DC OS HSM"                          }, // 1
        {  241, "Canon EF 50mm f/1.2L USM"                                  },
        {  242, "Canon EF 70-200mm f/4L IS USM"                             },
        {  243, "Canon EF 70-200mm f/4L IS USM + 1.4x"                      },
        {  244, "Canon EF 70-200mm f/4L IS USM + 2x"                        },
        {  245, "Canon EF 70-200mm f/4L IS USM + 2.8x"                      },
        {  246, "Canon EF 16-35mm f/2.8L II USM"                            },
        {  247, "Canon EF 14mm f/2.8L II USM"                               },
        {  248, "Canon EF 200mm f/2L IS USM"                                },
        {  248, "Sigma 24-35mm f/2 DG HSM | A"                              }, // 1
        {  248, "Sigma 135mm f/2 FF High-Speed Prime | 017"                 }, // 2
        {  248, "Sigma 24-35mm f/2.2 FF Zoom | 017"                         }, // 3
        {  248, "Sigma 135mm f/1.8 DG HSM A017"                             }, // 4
        {  249, "Canon EF 800mm f/5.6L IS USM"                              },
        {  250, "Canon EF 24mm f/1.4L II USM"                               },
        {  250, "Sigma 20mm f/1.4 DG HSM | A"                               }, // 1
        {  250, "Sigma 20mm f/1.5 FF High-Speed Prime | 017"                }, // 2
        {  250, "Tokina Opera 16-28mm f/2.8 FF"                             }, // 3
        {  250, "Sigma 85mm f/1.4 DG HSM A016"                              }, // 4
        {  251, "Canon EF 70-200mm f/2.8L IS II USM"                        },
        {  251, "Canon EF 70-200mm f/2.8L IS III USM"                       }, // 1
        {  252, "Canon EF 70-200mm f/2.8L IS II USM + 1.4x"                 },
        {  252, "Canon EF 70-200mm f/2.8L IS III USM + 1.4x"                }, // 1
        {  253, "Canon EF 70-200mm f/2.8L IS II USM + 2x"                   },
        {  253, "Canon EF 70-200mm f/2.8L IS III USM + 2x"                  }, // 1
        {  254, "Canon EF 100mm f/2.8L Macro IS USM"                        }, // 1
        {  254, "Tamron SP 90mm f/2.8 Di VC USD Macro 1:1"                  }, // 2
        {  255, "Sigma 24-105mm f/4 DG OS HSM | A"                          },
        {  255, "Sigma 180mm f/2.8 EX DG OS HSM APO Macro"                  }, // 1
        {  368, "Sigma 14-24mm f/2.8 DG HSM | A"                            },
        {  368, "Sigma 20mm f/1.4 DG HSM | A"                               }, // 1
        {  368, "Sigma 50mm f/1.4 DG HSM | A"                               }, // 2
        {  368, "Sigma 40mm f/1.4 DG HSM | A"                               }, // 3
        {  368, "Sigma 60-600mm f/4.5-6.3 DG OS HSM | S"                    }, // 4
        {  368, "Sigma 28mm f/1.4 DG HSM | A"                               }, // 5
        {  368, "Sigma 150-600mm f/5-6.3 DG OS HSM | S"                     }, // 6
        {  368, "Sigma 85mm f/1.4 DG HSM | A"                               }, // 7
        {  368, "Sigma 105mm f/1.4 DG HSM"                                  }, // 8
        {  368, "Sigma 14-24mm f/2.8 DG HSM"                                }, // 9
        {  368, "Sigma 70mm f/2.8 DG Macro"                                 }, // 10
        {  368, "Sigma 18-35mm f/1.8 DC HSM | A"                            }, // 11
        {  368, "Sigma 35mm f/1.4 DG HSM | A"                               }, // 12
        {  488, "Canon EF-S 15-85mm f/3.5-5.6 IS USM"                       },
        {  489, "Canon EF 70-300mm f/4-5.6L IS USM"                         },
        {  490, "Canon EF 8-15mm f/4L Fisheye USM"                          },
        {  491, "Canon EF 300mm f/2.8L IS II USM"                           },
        {  491, "Tamron SP 70-200mm f/2.8 Di VC USD G2"                     }, // 1
        {  491, "Tamron 18-400mm f/3.5-6.3 Di II VC HLD"                    }, // 2
        {  491, "Tamron 100-400mm f/4.5-6.3 Di VC USD"                      }, // 3
        {  491, "Tamron 70-210mm f/4 Di VC USD"                             }, // 4
        {  491, "Tamron 70-210mm f/4 Di VC USD + 1.4x"                      }, // 5
        {  491, "Tamron SP 24-70mm f/2.8 Di VC USD G2"                      }, // 6
        {  492, "Canon EF 400mm f/2.8L IS II USM"                           },
        {  493, "Canon EF 500mm f/4L IS II USM"                             },
        {  493, "Canon EF 24-105mm f/4L IS USM"                             }, // 1
        {  494, "Canon EF 600mm f/4L IS II USM"                             },
        {  495, "Canon EF 24-70mm f/2.8L II USM"                            },
        {  495, "Sigma 24-70mm f/2.8 DG OS HSM | A"                         }, // 1
        {  496, "Canon EF 200-400mm f/4L IS USM"                            },
        {  499, "Canon EF 200-400mm f/4L IS USM + 1.4x"                     },
        {  502, "Canon EF 28mm f/2.8 IS USM"                                },
        {  502, "Tamron 35mm f/1.8 Di VC USD"                               }, // 1
        {  503, "Canon EF 24mm f/2.8 IS USM"                                },
        {  504, "Canon EF 24-70mm f/4L IS USM"                              },
        {  505, "Canon EF 35mm f/2 IS USM"                                  },
        {  506, "Canon EF 400mm f/4 DO IS II USM"                           },
        {  507, "Canon EF 16-35mm f/4L IS USM"                              },
        {  508, "Canon EF 11-24mm f/4L USM"                                 },
        {  508, "Tamron 10-24mm f/3.5-4.5 Di II VC HLD"                     }, // 1
        {  624, "Sigma 70-200mm f/2.8 DG OS HSM | S"                        },
        {  747, "Canon EF 100-400mm f/4.5-5.6L IS II USM"                   },
        {  747, "Tamron SP 150-600mm f/5-6.3 Di VC USD G2"                  }, // 1
        {  748, "Canon EF 100-400mm f/4.5-5.6L IS II USM + 1.4x"            },
        {  748, "Tamron 100-400mm f/4.5-6.3 Di VC USD + 1.4x"               }, // 1
        {  748, "Tamron 70-210mm f/4 Di VC USD + 2x"                        }, // 2
        {  749, "Tamron 100-400mm f/4.5-6.3 Di VC USD + 2x"                 },
        {  750, "Canon EF 35mm f/1.4L II USM"                               },
        {  750, "Tamron SP 85mm f/1.8 Di VC USD"                            }, // 1
        {  750, "Tamron SP 45mm f/1.8 Di VC USD"                            }, // 2
        {  751, "Canon EF 16-35mm f/2.8L III USM"                           },
        {  752, "Canon EF 24-105mm f/4L IS II USM"                          },
        {  753, "Canon EF 85mm f/1.4L IS USM"                               },
        {  754, "Canon EF 70-200mm f/4L IS II USM"                          },
        {  757, "Canon EF 400mm f/2.8L IS III USM"                          },
        {  758, "Canon EF 600mm f/4L IS III USM"                            },
        { 1136, "Sigma 24-70mm f/2.8 DG OS HSM | Art 017"                   },
        { 4142, "Canon EF-S 18-135mm f/3.5-5.6 IS STM"                      },
        { 4143, "Canon EF-M 18-55mm f/3.5-5.6 IS STM"                       },
        { 4143, "Tamron 18-200mm f/3.5-6.3 Di III VC"                       }, // 1
        { 4144, "Canon EF 40mm f/2.8 STM"                                   },
        { 4145, "Canon EF-M 22mm f/2 STM"                                   },
        { 4146, "Canon EF-S 18-55mm f/3.5-5.6 IS STM"                       },
        { 4147, "Canon EF-M 11-22mm f/4-5.6 IS STM"                         },
        { 4148, "Canon EF-S 55-250mm f/4-5.6 IS STM"                        },
        { 4149, "Canon EF-M 55-200mm f/4.5-6.3 IS STM"                      },
        { 4150, "Canon EF-S 10-18mm f/4.5-5.6 IS STM"                       },
        { 4152, "Canon EF 24-105mm f/3.5-5.6 IS STM"                        },
        { 4153, "Canon EF-M 15-45mm f/3.5-6.3 IS STM"                       },
        { 4154, "Canon EF-S 24mm f/2.8 STM"                                 },
        { 4155, "Canon EF-M 28mm f/3.5 Macro IS STM"                        },
        { 4156, "Canon EF 50mm f/1.8 STM"                                   },
        { 4157, "Canon EF-M 18-150mm f/3.5-6.3 IS STM"                      },
        { 4158, "Canon EF-S 18-55mm f/4-5.6 IS STM"                         },
        { 4159, "Canon EF-M 32mm f/1.4 STM"                                 },
        { 4160, "Canon EF-S 35mm f/2.8 Macro IS STM"                        },
        { 4208, "Sigma 56mm f/1.4 DC DN | C"                                },
        {36910, "Canon EF 70-300mm f/4-5.6 IS II USM"                       },
        {36912, "Canon EF-S 18-135mm f/3.5-5.6 IS USM"                      },
        {61491, "Canon CN-E 14mm T3.1 L F"                                  },
        {61492, "Canon CN-E 24mm T1.5 L F"                                  },
        {61494, "Canon CN-E 85mm T1.3 L F"                                  },
        {61495, "Canon CN-E 135mm T2.2 L F"                                 },
        {61496, "Canon CN-E 35mm T1.5 L F"                                  },
        {61182, "Canon RF 35mm F1.8 Macro IS STM"                           },
        {61182, "Canon RF 50mm F1.2 L USM"                                  }, // 1
        {61182, "Canon RF 24-105mm F4 L IS USM"                             }, // 2
        {61182, "Canon RF 28-70mm F2 L USM"                                 }, // 3
        {61182, "Canon RF 85mm F1.2L USM"                                   }, // 4
        {61182, "Canon RF 24-240mm F4-6.3 IS USM"                           }, // 5
        {61182, "Canon RF 24-70mm F2.8 L IS USM"                            }, // 6
        {61182, "Canon RF 15-35mm F2.8 L IS USM"                            }, // 7
        {61182, "Canon RF 50mm F1.8 STM"                                    }, // 8
        {61182, "Canon RF 70-200mm F4L IS USM"                              }, // 9        
        {61182, "Canon RF 28-70mm F2L USM"                                  }, // 10           
        {61182, "Canon RF 85mm F2 MACRO IS STM"                             }, // 11 
        {61182, "Canon RF 100-500mm F4.5-7.1L IS USM + RF2x"                }, // 12 
        {61182, "Canon RF 35mm F1.8 MACRO IS STM"                           }, // 13                                                        
        {61182, "Canon RF 85mm F1.2L USM DS"                                }, // 14  
        {61182, "Canon RF 24-70mm F2.8L IS USM"                             }, // 15     
        {61182, "Canon RF 15-35mm F2.8L IS USM"                             }, // 16
        {61182, "Canon RF 24-240mm F4-6.3 IS USM"                           }, // 17
        {61182, "Canon RF 70-200mm F2.8L IS USM"                            }, // 18      
        {61182, "Canon RF 600mm F11 IS STM"                                 }, // 19      
        {61182, "Canon RF 600mm F11 IS STM + RF1.4x"                        }, // 20      
        {61182, "Canon RF 600mm F11 IS STM + RF2x"                          }, // 21      
        {61182, "Canon RF 800mm F11 IS STM"                                 }, // 22      
        {61182, "Canon RF 800mm F11 IS STM + RF1.4x"                        }, // 23     
        {61182, "Canon RF 800mm F11 IS STM + RF2x"                          }, // 24 
        {61182, "Canon RF 24-105mm F4-7.1 IS STM"                           }, // 25             
        {61182, "Canon RF 100-500mm F4.5-7.1 L IS USM"                      }, // 26 
        {61182, "Canon RF 100-500mm F4.5-7.1L IS USM + RF2x"                }, // 27    
        {65535, "n/a"                                                       }
    };

    //! A lens id and a pretty-print function for special treatment of the id.
    struct LensIdFct {
        long     id_;                           //!< Lens id
        PrintFct fct_;                          //!< Pretty-print function
        //! Comparison operator for find template
        bool operator==(long id) const { return id_ == id; }
    };

    //! List of lens ids which require special treatment with the medicine
    const LensIdFct lensIdFct[] = {
        {   4, printCsLensByFocalLength }, // not tested
        {   6, printCsLensByFocalLength },
        {   8, printCsLensByFocalLength },
        {   9, printCsLensByFocalLength },
        {  10, printCsLensByFocalLengthAndMaxAperture }, // works partly
        {  22, printCsLensByFocalLength },
        {  26, printCsLensByFocalLengthAndMaxAperture }, // works partly
        {  28, printCsLensByFocalLength },
        {  31, printCsLensByFocalLength },
        {  32, printCsLensByFocalLength },
        {  33, printCsLensByFocalLengthAndMaxAperture }, // works partly
        {  37, printCsLensByFocalLength },
        {  42, printCsLensByFocalLength },
        {  47, printCsLensByFocalLength }, // not tested
        { 131, printCsLensByFocalLength },
        { 136, printCsLensByFocalLength },
        { 137, printCsLensByFocalLength }, // not tested
        { 143, printCsLensByFocalLength },
        { 150, printCsLensByFocalLength },
        { 152, printCsLensByFocalLength },
        { 153, printCsLensByFocalLength },
        { 154, printCsLensByFocalLength }, // not tested
        { 155, printCsLensByFocalLength },
        { 156, printCsLensByFocalLengthAndMaxAperture },
        { 160, printCsLensByFocalLength },
        { 161, printCsLensByFocalLength },
        { 168, printCsLensByFocalLength },
        { 169, printCsLensByFocalLengthAndMaxAperture },
        { 172, printCsLensByFocalLengthTC }, // not tested
        { 173, printCsLensByFocalLengthTC }, // works partly
        { 174, printCsLensByFocalLength }, // not tested
        { 197, printCsLensByFocalLength },
        { 180, printCsLensByFocalLength },
        { 181, printCsLensByFocalLengthTC }, // not tested
        { 182, printCsLensByFocalLengthTC }, // not tested
        { 183, printCsLensByFocalLength }, // not tested
        { 198, printCsLensByFocalLength }, // not tested
        { 213, printCsLensByFocalLength }, // not tested
        { 234, printCsLensByFocalLength }, // not tested
        { 248, printCsLensByFocalLength }, // not tested
        { 250, printCsLensByFocalLength }, // not tested
        { 254, printCsLensByFocalLength },
        { 255, printCsLensByFocalLength }, // not tested
        { 368, printCsLensByFocalLengthAndMaxAperture },
        { 491, printCsLensByFocalLength },
        { 493, printCsLensByFocalLength }, // not tested
        { 624, printCsLensByFocalLengthTC },
        { 747, printCsLensByFocalLength }, // not tested
        { 4143,printCsLensByFocalLength }, // not tested
        { 4154,printCsLensByFocalLength }, // not tested
        {61182,printCsLensByFocalLength },
       {0xffff,printCsLensFFFF          }
    };

    //! FlashActivity, tag 0x001c
    constexpr TagDetails canonCsFlashActivity[] = {
        { 0, N_("Did not fire") },
        { 1, N_("Fired")        }
    };

    //! FlashDetails, tag 0x001d
    constexpr TagDetailsBitmask canonCsFlashDetails[] = {
        { 0x4000, N_("External flash")        },
        { 0x2000, N_("Internal flash")        },
        { 0x0001, N_("Manual")                },
        { 0x0002, N_("TTL")                   },
        { 0x0004, N_("A-TTL")                 },
        { 0x0008, N_("E-TTL")                 },
        { 0x0010, N_("FP sync enabled")       },
        { 0x0080, N_("2nd-curtain sync used") },
        { 0x0800, N_("FP sync used")          }
    };

    //! FocusContinuous, tag 0x0020
    constexpr TagDetails canonCsFocusContinuous[] = {
        { 0, N_("Single")     },
        { 1, N_("Continuous") },
        { 8, N_("Manual")     }
    };

    //! AESetting, tag 0x0021
    constexpr TagDetails canonCsAESetting[] = {
        { 0, N_("Normal AE")                       },
        { 1, N_("Exposure compensation")           },
        { 2, N_("AE lock")                         },
        { 3, N_("AE lock + exposure compensation") },
        { 4, N_("No AE")                           }
    };

    //! ImageStabilization, tag 0x0022
    constexpr TagDetails canonCsImageStabilization[] = {
        {   0, N_("Off")            },
        {   1, N_("On")             },
        {   2, N_("Shoot Only")     },
        {   3, N_("Panning")        },
        {   4, N_("Dynamic")        },
        { 256, N_("Off (2)")        },
        { 257, N_("On (2)")         },
        { 258, N_("Shoot Only (2)") },
        { 259, N_("Panning (2)")    },
        { 260, N_("Dynamic (2)")    }
    };

    //! SpotMeteringMode, tag 0x0027
    constexpr TagDetails canonCsSpotMeteringMode[] = {
        { 0,   N_("Center")   },
        { 1,   N_("AF Point") }
    };

    //! PhotoEffect, tag 0x0028
    constexpr TagDetails canonCsPhotoEffect[] = {
        { 0,   N_("Off")           },
        { 1,   N_("Vivid")         },
        { 2,   N_("Neutral")       },
        { 3,   N_("Smooth")        },
        { 4,   N_("Sepia")         },
        { 5,   N_("B&W")           },
        { 6,   N_("Custom")        },
        { 100, N_("My color data") },
        { 100, N_("My color data") }    // To silence compiler warning
    };

    //! ManualFlashOutput, tag 0x0029
    EXV_UNUSED constexpr TagDetails canonCsManualFlashOutput[] = {
        { 0x0000, N_("n/a")    },
        { 0x0500, N_("Full")   },
        { 0x0502, N_("Medium") },
        { 0x0504, N_("Low")    },
        { 0x7fff, N_("n/a")    }
    };

    //! SRAWQuality, tag 0x002e
    constexpr TagDetails canonCsSRAWQuality[] = {
        { 0, N_("n/a")          },
        { 1, N_("sRAW1 (mRAW)") },
        { 2, N_("sRAW2 (sRAW)") }
    };

    // Canon Camera Settings Tag Info
    constexpr TagInfo CanonMakerNote::tagInfoCs_[] = {
        {0x0001, "Macro", N_("Macro"), N_("Macro mode"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsMacro)},
        {0x0002, "Selftimer", N_("Selftimer"), N_("Self timer"), canonCsId, makerTags, signedShort, 1, printCs0x0002},
        {0x0003, "Quality", N_("Quality"), N_("Quality"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsQuality)},
        {0x0004, "FlashMode", N_("Flash Mode"), N_("Flash mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFlashMode)},
        {0x0005, "DriveMode", N_("Drive Mode"), N_("Drive mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsDriveMode)},
        {0x0006, "0x0006", "0x0006", N_("Unknown"), canonCsId, makerTags, unsignedShort, 1, printValue},
        {0x0007, "FocusMode", N_("Focus Mode"), N_("Focus mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFocusMode)},
        {0x0008, "0x0008", "0x0008", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue},
        {0x0009, "RecordMode", "RecordMode", N_("Record Mode"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsRecordMode)},
        {0x000a, "ImageSize", N_("Image Size"), N_("Image size"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsImageSize)},
        {0x000b, "EasyMode", N_("Easy Mode"), N_("Easy shooting mode"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsEasyMode)},
        {0x000c, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsDigitalZoom)},
        {0x000d, "Contrast", N_("Contrast"), N_("Contrast setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsLnh)},
        {0x000e, "Saturation", N_("Saturation"), N_("Saturation setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsLnh)},
        {0x000f, "Sharpness", N_("Sharpness"), N_("Sharpness setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsLnh)},
        {0x0010, "ISOSpeed", N_("ISO Speed Mode"), N_("ISO speed setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsISOSpeed)},
        {0x0011, "MeteringMode", N_("Metering Mode"), N_("Metering mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsMeteringMode)},
        {0x0012, "FocusType", N_("Focus Type"), N_("Focus type setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFocusType)},
        {0x0013, "AFPoint", N_("AF Point"), N_("AF point selected"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsAfPoint)},
        {0x0014, "ExposureProgram", N_("Exposure Program"), N_("Exposure mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsExposureProgram)},
        {0x0015, "0x0015", "0x0015", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue},
        {0x0016, "LensType", N_("Lens Type"), N_("Lens type"), canonCsId, makerTags, signedShort, 1, printCsLensType},
        {0x0017, "Lens", N_("Lens"), N_("'long' and 'short' focal length of lens (in 'focal units') and 'focal units' per mm"), canonCsId, makerTags, unsignedShort, 3, printCsLens},
        {0x0018, "ShortFocal", N_("Short Focal"), N_("Short focal"), canonCsId, makerTags, unsignedShort, 1, printValue},
        {0x0019, "FocalUnits", N_("Focal Units"), N_("Focal units"), canonCsId, makerTags, signedShort, 1, printValue},
        {0x001a, "MaxAperture", N_("Max Aperture"), N_("Max aperture"), canonCsId, makerTags, signedShort, 1, printSi0x0015},
        {0x001b, "MinAperture", N_("Min Aperture"), N_("Min aperture"), canonCsId, makerTags, signedShort, 1, printSi0x0015},
        {0x001c, "FlashActivity", N_("Flash Activity"), N_("Flash activity"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFlashActivity)},
        {0x001d, "FlashDetails", N_("Flash Details"), N_("Flash details"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG_BITMASK(canonCsFlashDetails)},
        {0x001e, "0x001e", "0x001e", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue},
        {0x001f, "0x001f", "0x001f", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue},
        {0x0020, "FocusContinuous", N_("Focus Continuous"), N_("Focus continuous setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFocusContinuous)},
        {0x0021, "AESetting", N_("AESetting"), N_("AE setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsAESetting)},
        {0x0022, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsImageStabilization)},
        {0x0023, "DisplayAperture", N_("Display Aperture"), N_("Display aperture"), canonCsId, makerTags, signedShort, 1, printValue},
        {0x0024, "ZoomSourceWidth", N_("Zoom Source Width"), N_("Zoom source width"), canonCsId, makerTags, signedShort, 1, printValue},
        {0x0025, "ZoomTargetWidth", N_("Zoom Target Width"), N_("Zoom target width"), canonCsId, makerTags, signedShort, 1, printValue},
        {0x0026, "0x0026", "0x0026", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue},
        {0x0027, "SpotMeteringMode", N_("Spot Metering Mode"), N_("Spot metering mode"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSpotMeteringMode)},
        {0x0028, "PhotoEffect", N_("Photo Effect"), N_("Photo effect"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsPhotoEffect)},
        {0x0029, "ManualFlashOutput", N_("Manual Flash Output"), N_("Manual flash output"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSRAWQuality)},
        {0x002a, "ColorTone", N_("Color Tone"), N_("Color tone"), canonCsId, makerTags, signedShort, 1, printValue},
        {0x002e, "SRAWQuality", N_("SRAW Quality Tone"), N_("SRAW quality"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSRAWQuality)},
        // End of list marker
        {0xffff, "(UnknownCanonCsTag)", "(UnknownCanonCsTag)", N_("Unknown Canon Camera Settings 1 tag"), canonCsId, makerTags, signedShort, 1, printValue},
    };

    const TagInfo* CanonMakerNote::tagListCs()
    {
        return tagInfoCs_;
    }


    //! AFPointUsed, tag 0x000e
    constexpr TagDetailsBitmask canonSiAFPointUsed[] = {
        { 0x0004, N_("left")   },
        { 0x0002, N_("center") },
        { 0x0001, N_("right")  }
    };

    //! FlashBias, tag 0x000f
    constexpr TagDetails canonSiFlashBias[] = {
        { 0xffc0, "-2 EV"    },
        { 0xffcc, "-1.67 EV" },
        { 0xffd0, "-1.50 EV" },
        { 0xffd4, "-1.33 EV" },
        { 0xffe0, "-1 EV"    },
        { 0xffec, "-0.67 EV" },
        { 0xfff0, "-0.50 EV" },
        { 0xfff4, "-0.33 EV" },
        { 0x0000, "0 EV"     },
        { 0x000c, "0.33 EV"  },
        { 0x0010, "0.50 EV"  },
        { 0x0014, "0.67 EV"  },
        { 0x0020, "1 EV"     },
        { 0x002c, "1.33 EV"  },
        { 0x0030, "1.50 EV"  },
        { 0x0034, "1.67 EV"  },
        { 0x0040, "2 EV"     }
    };

    constexpr TagDetails cameraType[] = {
        {   0, "n/a"           },
        { 248, "EOS High-end"  },
        { 250, "Compact"       },
        { 252, "EOS Mid-range" },
        { 255, "DV Camera"     },
    };

    constexpr TagDetails autoExposureBracketing[] = {
        { 65535, "On"          },
        { 0,     "Off"         },
        { 1,     "On (shot 1)" },
        { 2,     "On (shot 2)" },
        { 3,     "On (shot 3)" },
    };

    constexpr TagDetails slowShutter[] = {
        { 65535, "n/a"         },
        { 0,     "Off"         },
        { 1,     "Night Scene" },
        { 2,     "On"          },
        { 3,     "None"        },
    };


    // Canon Shot Info Tag
    constexpr TagInfo CanonMakerNote::tagInfoSi_[] = {
        {0x0001, "AutoISO", N_("AutoISO"), N_("AutoISO"), canonSiId, makerTags, unsignedShort, 1, printSi0x0001},
        {0x0002, "ISOSpeed", N_("ISO Speed Used"), N_("ISO speed used"), canonSiId, makerTags, unsignedShort, 1, printSi0x0002},
        {0x0003, "MeasuredEV", N_("Measured EV"), N_("Measured EV"), canonSiId, makerTags, unsignedShort, 1, printSi0x0003},
        {0x0004, "TargetAperture", N_("Target Aperture"), N_("Target Aperture"), canonSiId, makerTags, unsignedShort, 1, printSi0x0015},
        {0x0005, "TargetShutterSpeed", N_("Target Shutter Speed"), N_("Target shutter speed"), canonSiId, makerTags, unsignedShort, 1, printSi0x0016},
        {0x0006, "0x0006", "0x0006", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue},
        {0x0007, "WhiteBalance", N_("White Balance"), N_("White balance setting"), canonSiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonSiWhiteBalance)},
        {0x0008, "SlowShutter", N_("Slow Shutter"), N_("Slow shutter"), canonSiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(slowShutter)},
        {0x0009, "Sequence", N_("Sequence"), N_("Sequence number (if in a continuous burst)"), canonSiId, makerTags, unsignedShort, 1, printSi0x0009},
        {0x000a, "0x000a", "0x000a", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue},
        {0x000b, "0x000b", "0x000b", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue},
        {0x000c, "CameraTemperature", N_("Camera Temperature"), N_("Camera temperature"), canonSiId, makerTags, signedShort, 1, printSi0x000c},
        {0x000d, "FlashGuideNumber", N_("Flash Guide Number"), N_("Flash guide number"), canonSiId, makerTags, unsignedShort, 1, printSi0x000d},
        {0x000e, "AFPointUsed", N_("AF Point Used"), N_("AF point used"), canonSiId, makerTags, unsignedShort, 1, printSi0x000e},
        {0x000f, "FlashBias", N_("Flash Bias"), N_("Flash bias"), canonSiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonSiFlashBias)},
        {0x0010, "AutoExposureBracketing", N_("Auto Exposure Bracketing"), N_("Auto exposure bracketing"), canonSiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(autoExposureBracketing)},
        {0x0011, "0x0011", "0x0011", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue},
        {0x0012, "0x0012", "0x0012", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue},
        {0x0013, "SubjectDistance", N_("Subject Distance"), N_("Subject distance"), canonSiId, makerTags, unsignedShort, 1, printSi0x0013},
        {0x0014, "0x0014", "0x0014", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue},
        {0x0015, "ApertureValue", N_("Aperture Value"), N_("Aperture"), canonSiId, makerTags, unsignedShort, 1, printSi0x0015},
        {0x0016, "ShutterSpeedValue", N_("Shutter Speed Value"), N_("Shutter speed"), canonSiId, makerTags, unsignedShort, 1, printSi0x0016},
        {0x0017, "MeasuredEV2", N_("Measured EV 2"), N_("Measured EV 2"), canonSiId, makerTags, unsignedShort, 1, printSi0x0017},
        {0x0018, "BulbDuration", N_("Bulb Duration"), N_("Bulb duration"), canonSiId, makerTags, unsignedShort, 1, printSi0x0018},
        {0x0019, "0x0019", "0x0019", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue},
        {0x001a, "CameraType", N_("Camera Type"), N_("Camera type"), canonSiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(cameraType)},
        {0x001b, "AutoRotate", N_("Auto Rotate"), N_("Auto rotate"), canonSiId, makerTags, signedShort, 1, printValue},
        // End of list marker
        {0xffff, "(UnknownCanonCsTag)", "(UnknownCanonCsTag)", N_("Unknown Canon Camera Settings 1 tag"), canonCsId, makerTags, unsignedShort, 1, printValue},
    };

    const TagInfo* CanonMakerNote::tagListSi()
    {
        return tagInfoSi_;
    }

    //! PanoramaDirection, tag 0x0005
    constexpr TagDetails canonPaDirection[] = {
        { 0, N_("Left to right")          },
        { 1, N_("Right to left")          },
        { 2, N_("Bottom to top")          },
        { 3, N_("Top to bottom")          },
        { 4, N_("2x2 matrix (Clockwise)") }
    };

    // Canon Panorama Info
    constexpr TagInfo CanonMakerNote::tagInfoPa_[] = {
        {0x0002, "PanoramaFrame", N_("Panorama Frame"), N_("Panorama frame number"), canonPaId, makerTags, unsignedShort, 1, printValue},
        {0x0005, "PanoramaDirection", N_("Panorama Direction"), N_("Panorama direction"), canonPaId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonPaDirection)},
        // End of list marker
        {0xffff, "(UnknownCanonCs2Tag)", "(UnknownCanonCs2Tag)", N_("Unknown Canon Panorama tag"), canonPaId, makerTags, unsignedShort, 1, printValue},
    };

    const TagInfo* CanonMakerNote::tagListPa()
    {
        return tagInfoPa_;
    }

    // Canon Custom Function Tag Info
    constexpr TagInfo CanonMakerNote::tagInfoCf_[] = {
        {0x0001, "NoiseReduction", N_("Noise Reduction"), N_("Long exposure noise reduction"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x0002, "ShutterAeLock", N_("Shutter Ae Lock"), N_("Shutter/AE lock buttons"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x0003, "MirrorLockup", N_("Mirror Lockup"), N_("Mirror lockup"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x0004, "ExposureLevelIncrements", N_("Exposure Level Increments"), N_("Tv/Av and exposure level"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x0005, "AFAssist", N_("AF Assist"), N_("AF assist light"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x0006, "FlashSyncSpeedAv", N_("Flash Sync Speed Av"), N_("Shutter speed in Av mode"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x0007, "AEBSequence", N_("AEB Sequence"), N_("AEB sequence/auto cancellation"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x0008, "ShutterCurtainSync", N_("Shutter Curtain Sync"), N_("Shutter curtain sync"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x0009, "LensAFStopButton", N_("Lens AF Stop Button"), N_("Lens AF stop button Fn. Switch"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x000a, "FillFlashAutoReduction", N_("Fill Flash Auto Reduction"), N_("Auto reduction of fill flash"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x000b, "MenuButtonReturn", N_("Menu Button Return"), N_("Menu button return position"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x000c, "SetButtonFunction", N_("Set Button Function"), N_("SET button func. when shooting"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x000d, "SensorCleaning", N_("Sensor Cleaning"), N_("Sensor cleaning"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x000e, "SuperimposedDisplay", N_("Superimposed Display"), N_("Superimposed display"), canonCfId, makerTags, unsignedShort, 1, printValue},
        {0x000f, "ShutterReleaseNoCFCard", N_("Shutter Release No CF Card"), N_("Shutter Release W/O CF Card"), canonCfId, makerTags, unsignedShort, 1, printValue},
        // End of list marker
        {0xffff, "(UnknownCanonCfTag)", "(UnknownCanonCfTag)", N_("Unknown Canon Custom Function tag"), canonCfId, makerTags, unsignedShort, 1, printValue},
    };

    const TagInfo* CanonMakerNote::tagListCf()
    {
        return tagInfoCf_;
    }

    //! AFPointsUsed, tag 0x0016
    constexpr TagDetailsBitmask canonPiAFPointsUsed[] = {
        { 0x01, N_("right")     },
        { 0x02, N_("mid-right") },
        { 0x04, N_("bottom")    },
        { 0x08, N_("center")    },
        { 0x10, N_("top")       },
        { 0x20, N_("mid-left")  },
        { 0x40, N_("left")      }
    };

    //! AFPointsUsed20D, tag 0x001a
    constexpr TagDetailsBitmask canonPiAFPointsUsed20D[] = {
        { 0x001, N_("top")         },
        { 0x002, N_("upper-left")  },
        { 0x004, N_("upper-right") },
        { 0x008, N_("left")        },
        { 0x010, N_("center")      },
        { 0x020, N_("right")       },
        { 0x040, N_("lower-left")  },
        { 0x080, N_("lower-right") },
        { 0x100, N_("bottom")      }
    };

    // Canon Picture Info Tag
    constexpr TagInfo CanonMakerNote::tagInfoPi_[] = {
        {0x0002, "ImageWidth", N_("Image Width"), N_("Image width"), canonPiId, makerTags, unsignedShort, 1, printValue},
        {0x0003, "ImageHeight", N_("Image Height"), N_("Image height"), canonPiId, makerTags, unsignedShort, 1, printValue},
        {0x0004, "ImageWidthAsShot", N_("Image Width As Shot"), N_("Image width (as shot)"), canonPiId, makerTags, unsignedShort, 1, printValue},
        {0x0005, "ImageHeightAsShot", N_("Image Height As Shot"), N_("Image height (as shot)"), canonPiId, makerTags, unsignedShort, 1, printValue},
        {0x0016, "AFPointsUsed", N_("AF Points Used"), N_("AF points used"), canonPiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG_BITMASK(canonPiAFPointsUsed)},
        {0x001a, "AFPointsUsed20D", N_("AF Points Used 20D"), N_("AF points used (20D)"), canonPiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG_BITMASK(canonPiAFPointsUsed20D)},
        // End of list marker
        {0xffff, "(UnknownCanonPiTag)", "(UnknownCanonPiTag)", N_("Unknown Canon Picture Info tag"), canonPiId, makerTags, unsignedShort, 1, printValue},
    };

    const TagInfo* CanonMakerNote::tagListPi()
    {
        return tagInfoPi_;
    }

    //! BracketMode, tag 0x0003
    constexpr TagDetails canonBracketMode[] = {
        { 0, N_("Off") },
        { 1, N_("AEB") },
        { 2, N_("FEB") },
        { 3, N_("ISO") },
        { 4, N_("WB")  }
    };

    //! RawJpgSize, tag 0x0007
    constexpr TagDetails canonRawJpgSize[] = {
        {  -1, N_("n/a")               },
        {   0, N_("Large")             },
        {   1, N_("Medium")            },
        {   2, N_("Small")             },
        {   5, N_("Medium 1")          },
        {   6, N_("Medium 2")          },
        {   7, N_("Medium 3")          },
        {   8, N_("Postcard")          },
        {   9, N_("Widescreen")        },
        {  10, N_("Medium Widescreen") },
        {  14, N_("Small 1")           },
        {  15, N_("Small 2")           },
        {  16, N_("Small 3")           },
        { 128, N_("640x480 Movie")     },
        { 129, N_("Medium Movie")      },
        { 130, N_("Small Movie")       },
        { 137, N_("1280x720 Movie")    },
        { 142, N_("1920x1080 Movie")   },
        { 143, N_("4096x2160 Movie")   }
    };

    //! NoiseReduction, tag 0x0008
    constexpr TagDetails canonNoiseReduction[] = {
        { 0, N_("Off")  },
        { 1, N_("On 1") },
        { 2, N_("On 2") },
        { 3, N_("On")   },
        { 4, N_("Auto") }
    };

    //! WBBracketMode, tag 0x0009
    constexpr TagDetails canonWBBracketMode[] = {
        { 0, N_("Off")           },
        { 1, N_("On (shift AB)") },
        { 2, N_("On (shift GM)") }
    };

    //! FilterEffect, tag 0x000e
    constexpr TagDetails canonFilterEffect[] = {
        { 0, N_("None")   },
        { 1, N_("Yellow") },
        { 2, N_("Orange") },
        { 3, N_("Red")    },
        { 4, N_("Green")  }
    };

    //! ToningEffect, tag 0x000e
    constexpr TagDetails canonToningEffect[] = {
        { 0, N_("None")   },
        { 1, N_("Sepia")  },
        { 2, N_("Blue")   },
        { 3, N_("Purple") },
        { 4, N_("Green")  }
    };

    //! RFLensType, tag 0x003D
    constexpr TagDetails canonRFLensType[] = {
        { 0, N_("n/a")   },
        { 257, N_("Canon RF 50mm F1.2L USM")  },
        { 257, N_("Canon RF 24-105mm F4L IS USM")  },
        { 257, N_("Canon RF 28-70mm F2L USM")  },
        { 257, N_("Canon RF 35mm F1.8 MACRO IS STM")  },
        { 257, N_("Canon RF 85mm F1.2L USM")  },
        { 257, N_("Canon RF 85mm F1.2L USM DS")  },
        { 257, N_("Canon RF 24-70mm F2.8L IS USM")  },
        { 257, N_("Canon RF 15-35mm F2.8L IS USM")  },
        { 257, N_("Canon RF 24-240mm F4-6.3 IS USM")  },
        { 257, N_("Canon RF 70-200mm F2.8L IS USM")  },
        { 257, N_("Canon RF 85mm F2 MACRO IS STM")  },
        { 257, N_("Canon RF 600mm F11 IS STM")  },
        { 257, N_("Canon RF 600mm F11 IS STM + RF1.4x")  },
        { 257, N_("Canon RF 600mm F11 IS STM + RF2x")  },
        { 257, N_("Canon RF 800mm F11 IS STM")  },
        { 257, N_("Canon RF 800mm F11 IS STM + RF1.4x")  },
        { 257, N_("Canon RF 800mm F11 IS STM + RF2x")  },
        { 257, N_("Canon RF 24-105mm F4-7.1 IS STM")  },
        { 257, N_("Canon RF 100-500mm F4.5-7.1L IS USM")  },
        { 257, N_("Canon RF 100-500mm F4.5-7.1L IS USM + RF1.4x")  },
        { 257, N_("Canon RF 100-500mm F4.5-7.1L IS USM + RF2x")  }
    };

    // Canon File Info Tag
    constexpr TagInfo CanonMakerNote::tagInfoFi_[] = {
        {0x0001, "FileNumber", N_("File Number"), N_("File Number"), canonFiId, makerTags, unsignedLong, 1, printFiFileNumber},
        {0x0003, "BracketMode", N_("Bracket Mode"), N_("Bracket Mode"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonBracketMode)},
        {0x0004, "BracketValue", N_("Bracket Value"), N_("Bracket Value"), canonFiId, makerTags, signedShort, 1, printValue},
        {0x0005, "BracketShotNumber", N_("Bracket Shot Number"), N_("Bracket Shot Number"), canonFiId, makerTags, signedShort, 1, printValue},
        {0x0006, "RawJpgQuality", N_("Raw Jpg Quality"), N_("Raw Jpg Quality"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsQuality)},
        {0x0007, "RawJpgSize", N_("Raw Jpg Size"), N_("Raw Jpg Size"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonRawJpgSize)},
        {0x0008, "NoiseReduction", N_("Noise Reduction"), N_("Noise Reduction"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonNoiseReduction)},
        {0x0009, "WBBracketMode", N_("WB Bracket Mode"), N_("WB Bracket Mode"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonWBBracketMode)},
        {0x000c, "WBBracketValueAB", N_("WB Bracket Value AB"), N_("WB Bracket Value AB"), canonFiId, makerTags, signedShort, 1, printValue},
        {0x000d, "WBBracketValueGM", N_("WB Bracket Value GM"), N_("WB Bracket Value GM"), canonFiId, makerTags, signedShort, 1, printValue},
        {0x000e, "FilterEffect", N_("Filter Effect"), N_("Filter Effect"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonFilterEffect)},
        {0x000f, "ToningEffect", N_("Toning Effect"), N_("Toning Effect"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonToningEffect)},
        {0x0010, "MacroMagnification", N_("Macro Magnification"), N_("Macro magnification"), canonFiId, makerTags, signedShort, 1, printValue},
        {0x0013, "LiveViewShooting", N_("Live View Shooting"), N_("Live view shooting"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonOffOn)},
        {0x0014, "FocusDistanceUpper", N_("Focus Distance Upper"), N_("Focus Distance Upper"), canonFiId, makerTags, unsignedShort, 1, printFiFocusDistance},
        {0x0015, "FocusDistanceLower", N_("Focus Distance Lower"), N_("Focus Distance Lower"), canonFiId, makerTags, unsignedShort, 1, printFiFocusDistance},
        {0x0019, "FlashExposureLock", N_("Flash Exposure Lock"), N_("Flash exposure lock"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonOffOn)},
        {0x003D, "RFLensType", N_("RF Lens Type"), N_("RF Lens Type"), canonFiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonRFLensType)}, 
        // End of list marker
        {0xffff, "(UnknownCanonFiTag)", "(UnknownCanonFiTag)", N_("Unknown Canon File Info tag"), canonFiId, makerTags, signedShort, 1, printValue},
    };

    const TagInfo* CanonMakerNote::tagListFi()
    {
        return tagInfoFi_;
    }

    //! Tone Curve Values
    constexpr TagDetails canonToneCurve[] = {
        { 0, N_("Standard") },
        { 1, N_("Manual")   },
        { 2, N_("Custom")   }
    };

    //! Sharpness Frequency Values
    constexpr TagDetails canonSharpnessFrequency[] = {
        { 0, N_("n/a")      },
        { 1, N_("Lowest")   },
        { 2, N_("Low")      },
        { 3, N_("Standard") },
        { 4, N_("High")     },
        { 5, N_("Highest")  }
    };


    // Canon Processing Info Tag
    constexpr TagInfo CanonMakerNote::tagInfoPr_[] = {
        {0x0001, "ToneCurve", N_("ToneCurve"), N_("Tone curve"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonToneCurve)},
        {0x0002, "Sharpness", N_("Sharpness"), N_("Sharpness"), canonPrId, makerTags, signedShort, 1, printValue},
        {0x0003, "SharpnessFrequency", N_("SharpnessFrequency"), N_("Sharpness frequency"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonSharpnessFrequency)},
        {0x0004, "SensorRedLevel", N_("SensorRedLevel"), N_("Sensor red level"), canonPrId, makerTags, signedShort, 1, printValue},
        {0x0005, "SensorBlueLevel", N_("SensorBlueLevel"), N_("Sensor blue level"), canonPrId, makerTags, signedShort, 1, printValue},
        {0x0006, "WhiteBalanceRed", N_("WhiteBalanceRed"), N_("White balance red"), canonPrId, makerTags, signedShort, 1, printValue},
        {0x0007, "WhiteBalanceBlue", N_("WhiteBalanceBlue"), N_("White balance blue"), canonPrId, makerTags, signedShort, 1, printValue},
        {0x0008, "WhiteBalance", N_("WhiteBalance"), N_("White balance"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonSiWhiteBalance)},
        {0x0009, "ColorTemperature", N_("ColorTemperature"), N_("Color Temperature"), canonPrId, makerTags, signedShort, 1, printValue},
        {0x000a, "PictureStyle", N_("PictureStyle"), N_("Picture style"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonPictureStyle)},
        {0x000b, "DigitalGain", N_("DigitalGain"), N_("Digital gain"), canonPrId, makerTags, signedShort, 1, printValue},
        {0x000c, "WBShiftAB", N_("WBShiftAB"), N_("WBShift AB"), canonPrId, makerTags, signedShort, 1, printValue},
        {0x000d, "WBShiftGM", N_("WBShiftGM"), N_("WB Shift GM"), canonPrId, makerTags, signedShort, 1, printValue},
        {0xffff, "(UnknownCanonPrTag)", "(UnknownCanonPrTag)", N_("Unknown Canon Processing Info tag"), canonPrId, makerTags, signedShort, 1, printValue},
    };

    const TagInfo* CanonMakerNote::tagListPr()
    {
        return tagInfoPr_;
    }

    //! canonTimeZoneCity - array of cityID/cityName used by Canon
    constexpr TagDetails canonTimeZoneCity[] = {
        { 0x0000, N_("n/a")                 },
        { 0x0001, N_("Chatham Islands")     },
        { 0x0002, N_("Wellington")          },
        { 0x0003, N_("Solomon Islands")     },
        { 0x0004, N_("Sydney")              },
        { 0x0005, N_("Adelaide")            },
        { 0x0006, N_("Tokyo")               },
        { 0x0007, N_("Hong Kong")           },
        { 0x0008, N_("Bangkok")             },
        { 0x0009, N_("Yangon")              },
        { 0x000a, N_("Dhaka")               },
        { 0x000b, N_("Kathmandu")           },
        { 0x000c, N_("Delhi")               },
        { 0x000d, N_("Karachi")             },
        { 0x000e, N_("Kabul")               },
        { 0x000f, N_("Dubai")               },
        { 0x0010, N_("Tehran")              },
        { 0x0011, N_("Moscow")              },
        { 0x0012, N_("Cairo")               },
        { 0x0013, N_("Paris")               },
        { 0x0014, N_("London")              },
        { 0x0015, N_("Azores")              },
        { 0x0016, N_("Fernando de Noronha") },
        { 0x0017, N_("Sao Paulo")           },
        { 0x0018, N_("Newfoundland")        },
        { 0x0019, N_("Santiago")            },
        { 0x001a, N_("Caracas")             },
        { 0x001b, N_("New York")            },
        { 0x001c, N_("Chicago")             },
        { 0x001d, N_("Denver")              },
        { 0x001e, N_("Los Angeles")         },
        { 0x001f, N_("Anchorage")           },
        { 0x0020, N_("Honolulu")            },
        { 0x0021, N_("Samoa")               },
        { 0x7ffe, N_("(not set)")           },
    };

    // Canon Time Info Tag
    constexpr TagInfo CanonMakerNote::tagInfoTi_[] = {
        {0x0001, "TimeZone", N_("Time zone offset"), N_("Time zone offset in minutes"), canonTiId, makerTags, signedLong, 1, printValue},
        {0x0002, "TimeZoneCity", N_("Time zone city"), N_("Time zone city"), canonTiId, makerTags, signedLong, 1, EXV_PRINT_TAG(canonTimeZoneCity)},
        {0x0003, "DaylightSavings", N_("Daylight Savings"), N_("Daylight Saving Time"), canonTiId, makerTags, signedLong, 1, printValue},
        {0xffff, "(UnknownCanonTiTag)", "(UnknownCanonTiTag)", N_("Unknown Canon Time Info tag"), canonTiId, makerTags, signedLong, 1, printValue},
    };

    const TagInfo* CanonMakerNote::tagListTi()
    {
        return tagInfoTi_;
    }

    std::ostream& CanonMakerNote::printFiFileNumber(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        std::ios::fmtflags f(os.flags());
        if (!metadata || value.typeId() != unsignedLong || value.count() == 0) {
            os << "(" << value << ")";
            os.flags(f);
            return os;
        }

        auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
        if (pos == metadata->end())
            return os << "(" << value << ")";

        // Ported from Exiftool
        std::string model = pos->toString();
        if (model.find("20D") != std::string::npos || model.find("350D") != std::string::npos ||
            model.substr(model.size() - 8, 8) == "REBEL XT" || model.find("Kiss Digital N") != std::string::npos) {
            uint32_t val = value.toLong();
            uint32_t dn = (val & 0xffc0) >> 6;
            uint32_t fn = ((val >> 16) & 0xff) + ((val & 0x3f) << 8);
            os << std::dec << dn << "-" << std::setw(4) << std::setfill('0') << fn;
            os.flags(f);
            return os;
        }
        if (model.find("30D") != std::string::npos || model.find("400D") != std::string::npos ||
            model.find("REBEL XTi") != std::string::npos || model.find("Kiss Digital X") != std::string::npos ||
            model.find("K236") != std::string::npos) {
            uint32_t val = value.toLong();
            uint32_t dn = (val & 0xffc00) >> 10;
            while (dn < 100)
                dn += 0x40;
            uint32_t fn = ((val & 0x3ff) << 4) + ((val >> 20) & 0x0f);
            os << std::dec << dn << "-" << std::setw(4) << std::setfill('0') << fn;
            os.flags(f);
            return os;
        }

        os.flags(f);
        return os << "(" << value << ")";
    }

    std::ostream& CanonMakerNote::printFocalLength(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        std::ios::fmtflags f(os.flags());
        if (!metadata || value.count() < 4 || value.typeId() != unsignedShort) {
            os.flags(f);
            return os << value;
        }

        ExifKey key("Exif.CanonCs.Lens");
        auto pos = metadata->findKey(key);
        if (pos != metadata->end() && pos->value().count() >= 3 && pos->value().typeId() == unsignedShort) {
            float fu = pos->value().toFloat(2);
            if (fu != 0.0F) {
                float fl = value.toFloat(1) / fu;
                std::ostringstream oss;
                oss.copyfmt(os);
                os << std::fixed << std::setprecision(1);
                os << fl << " mm";
                os.copyfmt(oss);
                os.flags(f);
                return os;
            }
        }

        os.flags(f);
        return os << value;
    }

    std::ostream& CanonMakerNote::print0x0008(std::ostream& os,
                                              const Value& value,
                                              const ExifData*)
    {
        std::string n = value.toString();
        if (n.length() < 4) return os << "(" << n << ")";
        return os << n.substr(0, n.length() - 4) << "-"
                  << n.substr(n.length() - 4);
    }

    std::ostream& CanonMakerNote::print0x000c(std::ostream& os,
                                              const Value& value,
                                              const ExifData*)
    {
        std::istringstream is(value.toString());
        uint32_t l;
        is >> l;
        return os << std::setw(4) << std::setfill('0') << std::hex
                  << ((l & 0xffff0000) >> 16)
                  << std::setw(5) << std::setfill('0') << std::dec
                  << (l & 0x0000ffff);
    }

    std::ostream& CanonMakerNote::printCs0x0002(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        if (l == 0) {
            os << "Off";
        }
        else {
            os << l / 10.0 << " s";
        }
        return os;
    }

    std::ostream& printCsLensFFFF(std::ostream& os,
                                const Value& value,
                                const ExifData* metadata)
    {
        try {
            // 1140
            const auto itModel = metadata->findKey(ExifKey("Exif.Image.Model"));
            const auto itLens  = metadata->findKey(ExifKey("Exif.CanonCs.Lens"));
            const auto itApert = metadata->findKey(ExifKey("Exif.CanonCs.MaxAperture"));

            if( itModel != metadata->end() && itModel->value().toString() == "Canon EOS 30D"
            &&  itLens  != metadata->end() && itLens->value().toString() == "24 24 1"
            &&  itApert != metadata->end() && itApert->value().toString() == "95" // F2.8
            ){
                return os << "Canon EF-S 24mm f/2.8 STM" ;
            }
        } catch (std::exception&) {};

        return EXV_PRINT_TAG(canonCsLensType)(os, value, metadata);
    }

    //! Helper structure
    struct LensTypeAndFocalLengthAndMaxAperture {
        long        lensType_;                  //!< Lens type
        float       focalLengthMin_;            //!< Mininum focal length
        float       focalLengthMax_;            //!< Maximum focal length
        std::string focalLength_;               //!< Focal length as a string
        std::string maxAperture_;               //!< Aperture
    };

    //! Compare tag details with a lens entry
    bool operator==(const TagDetails& td, const LensTypeAndFocalLengthAndMaxAperture& ltfl) {
        return (   td.val_ == ltfl.lensType_
                && std::string(td.label_).find(ltfl.focalLength_) != std::string::npos
                && std::string(td.label_).find(ltfl.maxAperture_) != std::string::npos);
    }

    //! extractLensFocalLength from metadata
    void extractLensFocalLength(LensTypeAndFocalLengthAndMaxAperture& ltfl,
                                const ExifData* metadata)
    {
        ExifKey key("Exif.CanonCs.Lens");
        auto pos = metadata->findKey(key);
        ltfl.focalLengthMin_ = 0.0F;
        ltfl.focalLengthMax_ = 0.0F;
        if (pos != metadata->end()) {
            const Value &value = pos->value();
            if (   value.count() >= 3
                && value.typeId() == unsignedShort) {
                float fu = value.toFloat(2);
                if (fu != 0.0F) {
                    ltfl.focalLengthMin_ = value.toLong(1) / fu;
                    ltfl.focalLengthMax_ = value.toLong(0) / fu;
                }
            }
        }
    }

    //! convertFocalLength to a human readable string
    void convertFocalLength(LensTypeAndFocalLengthAndMaxAperture& ltfl, float divisor)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0);
        if (ltfl.focalLengthMin_ == ltfl.focalLengthMax_) {
            oss << " " << (ltfl.focalLengthMin_ / divisor) << "mm";
        } else {
            oss << " " << (ltfl.focalLengthMin_ / divisor) << "-" << (ltfl.focalLengthMax_ / divisor) << "mm";
        }
        ltfl.focalLength_ = oss.str();
    }

    //! printCsLensByFocalLengthAndMaxAperture to human readable string
    std::ostream& printCsLensByFocalLengthAndMaxAperture(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata)
    {
        if (   !metadata || value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        LensTypeAndFocalLengthAndMaxAperture ltfl;
        ltfl.lensType_ = value.toLong();

        extractLensFocalLength(ltfl, metadata);
        if (ltfl.focalLengthMax_ == 0.0F)
            return os << value;
        convertFocalLength(ltfl, 1.0F);

        ExifKey key("Exif.CanonCs.MaxAperture");
        auto pos = metadata->findKey(key);
        if (   pos != metadata->end()
            && pos->value().count() == 1
            && pos->value().typeId() == unsignedShort) {

            long val = static_cast<int16_t>(pos->value().toLong(0));
            if (val > 0)
            {
                std::ostringstream oss;
                oss << std::setprecision(2);
                oss << fnumber(canonEv(val));
                ltfl.maxAperture_ = oss.str();
            }
        }
        if (ltfl.maxAperture_.empty()) return os << value;

        const TagDetails* td = find(canonCsLensType, ltfl);
        if (!td) return os << value;
        return os << td->label_;
    }

    //! printCsLensByFocalLength to human readable string
    std::ostream& printCsLensByFocalLength(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata)
    {
        if (   !metadata || value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        LensTypeAndFocalLengthAndMaxAperture ltfl;
        ltfl.lensType_ = value.toLong();

        extractLensFocalLength(ltfl, metadata);
        if (ltfl.focalLengthMax_ == 0.0F)
            return os << value;
        convertFocalLength(ltfl, 1.0F);

        if (ltfl.focalLength_.empty()) return os << value;

        const TagDetails* td = find(canonCsLensType, ltfl);
        if (!td) return os << value;
        return os << td->label_;
    }

    //! printCsLensByFocalLengthTC to human readable string
    std::ostream& printCsLensByFocalLengthTC(std::ostream& os,
                                             const Value& value,
                                             const ExifData* metadata)
    {
        if (   !metadata || value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        LensTypeAndFocalLengthAndMaxAperture ltfl;
        ltfl.lensType_ = value.toLong();

        extractLensFocalLength(ltfl, metadata);
        if (ltfl.focalLengthMax_ == 0.0F)
            return os << value;
        convertFocalLength(ltfl, 1.0F);  // just lens
        const TagDetails* td = find(canonCsLensType, ltfl);
        if (!td) {
            convertFocalLength(ltfl, 1.4F);  // lens + 1.4x TC
            td = find(canonCsLensType, ltfl);
            if (!td) {
                convertFocalLength(ltfl, 2.0F);  // lens + 2x TC
                td = find(canonCsLensType, ltfl);
                if (!td) return os << value;
            }
        }
        return os << td->label_;
    }

    //! printCsLensType by searching the config file if necessary
    std::ostream& CanonMakerNote::printCsLensType(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData* metadata)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << "(" << value << ")";

        // #1034
        const std::string undefined("undefined") ;
        const std::string section  ("canon");
        if ( Internal::readExiv2Config(section,value.toString(),undefined) != undefined ) {
            return os << Internal::readExiv2Config(section,value.toString(),undefined);
        }

        const LensIdFct* lif = find(lensIdFct, value.toLong());
        if (!lif) {
            return EXV_PRINT_TAG(canonCsLensType)(os, value, metadata);
        }
        if (metadata && lif->fct_) {
            return lif->fct_(os, value, metadata);
        }
        return os << value;
    }

    std::ostream& CanonMakerNote::printCsLens(std::ostream& os,
                                              const Value& value,
                                              const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );

        if (   value.count() < 3
            || value.typeId() != unsignedShort) {
            os << "(" << value << ")";
            os.flags(f);
            return os;
        }

        float fu = value.toFloat(2);
        if (fu == 0.0F)
            return os << value;
        float len1 = value.toLong(0) / fu;
        float len2 = value.toLong(1) / fu;
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1);
        if (len1 == len2) {
            os << len1 << " mm";
        } else {
            os << len2 << " - " << len1 << " mm";
        }
        os.copyfmt(oss);
        os.flags(f);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0001(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if (   value.typeId() == unsignedShort
            && value.count() > 0) {
            os << std::exp(canonEv(value.toLong()) / 32 * std::log(2.0F)) * 100.0F;
        }
        os.flags(f);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0002(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if (   value.typeId() == unsignedShort
            && value.count() > 0) {
            // Ported from Exiftool by Will Stokes
            os << std::exp(canonEv(value.toLong()) * std::log(2.0F)) * 100.0F / 32.0F;
        }
        os.flags(f);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0003(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() == unsignedShort
            && value.count() > 0) {
            // The offset of '5' seems to be ok for most Canons (see Exiftool)
            // It might be explained by the fakt, that most Canons have a longest
            // exposure of 30s which is 5 EV below 1s
            // see also printSi0x0017
            std::ostringstream oss;
            oss.copyfmt(os);
            int res = static_cast<int>(100.0 * (static_cast<short>(value.toLong()) / 32.0 + 5.0) + 0.5);
            os << std::fixed << std::setprecision(2) << res / 100.0;
            os.copyfmt(oss);
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0009(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        os << l << "";
        // Todo: determine unit
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x000c(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (value.toLong() == 0) return os << "--";

        return os << value.toLong() - 128 << " °C";
    }

    std::ostream& CanonMakerNote::printSi0x000d(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (value.toLong() == 65535) return os << "--";

        return os << value.toLong() / 32;
    }

    std::ostream& CanonMakerNote::printSi0x000e(std::ostream& os,
                                                const Value& value,
                                                const ExifData* pExifData)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        long num = (l & 0xf000) >> 12;
        os << num << " focus points; ";
        long used = l & 0x0fff;
        if (used == 0) {
            os << "none";
        }
        else {
            EXV_PRINT_TAG_BITMASK(canonSiAFPointUsed)(os, value, pExifData);
        }
        os << " used";
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0013(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        if (l == 0xffff) {
            os << "Infinite";
        }
        else {
            os << value.toLong()/100.0 << " m";
        }
        os.flags(f);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0015(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        std::ostringstream oss;
        oss.copyfmt(os);
        long val = static_cast<int16_t>(value.toLong());
        if (val < 0) return os << value;
        os << std::setprecision(2)
           << "F" << fnumber(canonEv(val));
        os.copyfmt(oss);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0016(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        URational ur = exposureTime(canonEv(value.toLong()));
        os << ur.first;
        if (ur.second > 1) {
            os << "/" << ur.second;
        }
        os.flags(f);
        return os << " s";
    }

    std::ostream& CanonMakerNote::printSi0x0017(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(2)
           << value.toLong() / 8.0 - 6.0;
        os.copyfmt(oss);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0018(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        return os << value.toLong() / 10;
    }

    std::ostream& CanonMakerNote::printFiFocusDistance(std::ostream& os,
                                                       const Value& value,
                                                       const ExifData*)
    {
       std::ios::fmtflags f( os.flags() );
       if (   value.typeId() != signedShort
         || value.count() == 0) return os << value;

      std::ostringstream oss;
      oss.copyfmt(os);
      os << std::fixed << std::setprecision(2);

      long l = value.toLong();
      if (l == -1) {
        os << "Infinite";
      }
      else {
        os << value.toLong()/100.0 << " m";
      }

      os.copyfmt(oss);
      os.flags(f);
      return os;
    }

// *****************************************************************************
// free functions

    float canonEv(long val)
    {
        // temporarily remove sign
        int sign = 1;
        if (val < 0) {
            sign = -1;
            val = -val;
        }
        // remove fraction
        auto frac = static_cast<float>(val & 0x1f);
        val -= long(frac);
        // convert 1/3 (0x0c) and 2/3 (0x14) codes
        if (frac == 0x0c) {
            frac = 32.0F / 3;
        }
        else if (frac == 0x14) {
            frac = 64.0F / 3;
        }
        else if ((val == 160) && (frac == 0x08)) { // for Sigma f/6.3 lenses that report f/6.2 to camera
            frac = 30.0F / 3;
        }
        return sign * (val + frac) / 32.0F;
    }

    }  // namespace Internal
}  // namespace Exiv2
