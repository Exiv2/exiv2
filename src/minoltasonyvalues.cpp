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
  File:      minoltasonyvalues.cpp
  Version:   $Rev$
  Author(s): Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History:   14-Apr-10, gc: submitted
  Credits:   See header file.
 */

// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "minoltasonyvalues.hpp"
#include "value.hpp"
#include "i18n.h"                // NLS support.

#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

// *****************************************************************************
// member definitions
namespace Exiv2 {

        //! Lookup table to translate Minolta/Sony Lens id values to readable labels
    /* NOTE:
       - duplicate tags value are:
       0/25520, 4/25920, 13/25610, 19/25910, 22/26050/26070,
       25500/25501/26130, 25540/25541/25850, 25580/25581, 2564025641,
       25720/25721, 25790/25791, 25960/25961, 25980/25981, 26150/26151
       - No need to i18n these string.
    */
    extern TagDetails const minoltaSonyLensID[] = {
        { 0,     "Minolta AF 28-85mm F3.5-4.5 New" },
        { 1,     "Minolta AF 80-200mm F2.8 HS-APO G" },
        { 2,     "Minolta AF 28-70mm F2.8 G" },
        { 3,     "Minolta AF 28-80mm F4-5.6" },
        { 4,     "Minolta AF 85mm F1.4G" },
        { 5,     "Minolta AF 35-70mm F3.5-4.5 [II]" },
        { 6,     "Minolta AF 24-85mm F3.5-4.5 [New]" },
        { 7,     "Minolta AF 100-300mm F4.5-5.6(D) APO [New] | "
                 "Minolta AF 100-400mm F4.5-6.7(D) | "
                 "Sigma AF 100-300mm F4 EX DG IF" },
        { 8,     "Minolta AF 70-210mm F4.5-5.6 [II]" },
        { 9,     "Minolta AF 50mm F3.5 Macro" },
        { 10,    "Minolta AF 28-105mm F3.5-4.5 [New]" },
        { 11,    "Minolta AF 300mm F4 HS-APO G" },
        { 12,    "Minolta AF 100mm F2.8 Soft Focus" },
        { 13,    "Minolta AF 75-300mm F4.5-5.6 (New or II)" },
        { 14,    "Minolta AF 100-400mm F4.5-6.7 APO" },
        { 15,    "Minolta AF 400mm F4.5 HS-APO G" },
        { 16,    "Minolta AF 17-35mm F3.5 G" },
        { 17,    "Minolta AF 20-35mm F3.5-4.5" },
        { 18,    "Minolta AF 28-80mm F3.5-5.6 II" },
        { 19,    "Minolta AF 35mm F1.4 G" },
        { 20,    "Minolta/Sony 135mm F2.8 [T4.5] STF" },
        { 22,    "Minolta AF 35-80mm F4-5.6 II" },
        { 23,    "Minolta AF 200mm F4 Macro APO G" },
        { 24,    "Minolta/Sony AF 24-105mm F3.5-4.5 (D) | "
                 "Sigma 18-50mm F2.8 | "
                 "Sigma 17-70mm F2.8-4.5 (D) | "
                 "Sigma 20-40mm F2.8 EX DG Aspherical IF | "
                 "Sigma 18-200mm F3.5-6.3 DC | "
                 "Sigma 20-40mm F2.8 EX DG Aspherical IF | "
                 "Sigma DC 18-125mm F4-5,6 D | "
                 "Tamron SP AF 28-75mm F2.8 XR Di (IF) Macro" },
        { 25,    "Minolta AF 100-300mm F4.5-5.6 APO (D) | "
                 "Sigma 100-300mm F4 EX (APO (D) or D IF) | "
                 "Sigma 70mm F2.8 EX DG Macro | "
                 "Sigma 20mm F1.8 EX DG Aspherical RF | "
                 "Sigma 30mm F1.4 DG EX" },
        { 27,    "Minolta AF 85mm F1.4 G (D)" },
        { 28,    "Minolta/Sony AF 100mm F2.8 Macro (D) | "
                 "Tamron SP AF 90mm F2.8 Di Macro" },
        { 29,    "Minolta/Sony AF 75-300mm F4.5-5.6 (D) " },
        { 30,    "Minolta AF 28-80mm F3.5-5.6 (D) | "
                 "Sigma AF 10-20mm F4-5.6 EX DC | "
                 "Sigma AF 12-24mm F4.5-5.6 EX DG | "
                 "Sigma 28-70mm EX DG F2.8 | "
                 "Sigma 55-200mm F4-5.6 DC" },
        { 31,    "Minolta AF 50mm F2.8 Macro(D) | "
                 "Minolta AF 50mm F3.5 Macro" },
        { 32,    "Minolta AF 100-400mm F4.5-6.7(D) | "
                 "Minolta AF 300mm F2.8G APO(D) SSM" },
        { 33,    "Minolta/Sony AF 70-200mm F2.8 G" },
        { 35,    "Minolta AF 85mm F1.4 G (D) Limited" },
        { 36,    "Minolta AF 28-100mm F3.5-5.6 (D)" },
        { 38,    "Minolta AF 17-35mm F2.8-4 (D)" },
        { 39,    "Minolta AF 28-75mm F2.8 (D)" },
        { 40,    "Minolta/Sony AF DT 18-70mm F3.5-5.6 (D) | "
                 "Sony AF DT 18-200mm F3.5-6.3" },
        { 41,    "Minolta/Sony AF DT 11-18mm F4.5-5.6 (D) | "
                 "Tamron SP AF 11-18mm F4.5-5.6 Di II LD Aspherical IF" },
        { 42,    "Minolta/Sony AF DT 18-200mm F3.5-6.3 (D)" },
        { 43,    "Sony 35mm F1.4 G (SAL-35F14G)" },
        { 44,    "Sony 50mm F1.4 (SAL-50F14)" },
        { 45,    "Carl Zeiss Planar T* 85mm F1.4 ZA" },
        { 46,    "Carl Zeiss Vario-Sonnar T* DT 16-80mm F3.5-4.5 ZA" },
        { 47,    "Carl Zeiss Sonnar T* 135mm F1.8 ZA" },
        { 48,    "Carl Zeiss Vario-Sonnar T* 24-70mm F2.8 ZA SSM (SAL-2470Z)" },
        { 49,    "Sony AF DT 55-200mm F4-5.6" },
        { 50,    "Sony AF DT 18-250mm F3.5-6.3" },
        { 51,    "Sony AF DT 16-105mm F3.5-5.6 | "
                 "Sony AF DT 55-200mm F4-5.5" },
        { 52,    "Sony 70-300mm F4.5-5.6 G SSM" },
        { 53,    "Sony AF 70-400mm F4.5-5.6 G SSM (SAL-70400G)" },
        { 54,    "Carl Zeiss Vario-Sonnar T* 16-35mm F2.8 ZA SSM (SAL-1635Z)" },
        { 55,    "Sony DT 18-55mm F3.5-5.6 SAM (SAL-1855)" },
        { 56,    "Sony AF DT 55-200mm F4-5.6 SAM" },
        { 57,    "Sony AF DT 50mm F1.8 SAM" },
        { 58,    "Sony AF DT 30mm F2.8 SAM Macro" },
        { 128,   "Sigma 70-200mm F2.8 APO EX DG MACRO | "
                 "Tamron 18-200mm F3.5-6.3 | "
                 "Tamron 28-300mm F3.5-6.3 | "
                 "Tamron 80-300mm F3.5-6.3 | "
                 "Tamron AF 28-200mm F3.8-5.6 XR Di Aspherical [IF] MACRO | "
                 "Tamron SP AF 17-35mm F2.8-4 Di LD Aspherical IF | "
                 "Sigma AF 50-150mm F2.8 EX DC APO HSM II | "
                 "Sigma 10-20mm F3.5 EX DC HSM | "
                 "Sigma 70-200mm F2.8 II EX DG APO MACRO HSM" },
        { 129,   "Tamron 200-400mm F5.6 LD | "
                 "Tamron 70-300mm F4-5.6 LD" },
        { 131,   "Tamron 20-40mm F2.7-3.5 SP Aspherical IF" },
        { 135,   "Vivitar 28-210mm F3.5-5.6" },
        { 136,   "Tokina EMZ M100 AF 100mm F3.5" },
        { 137,   "Cosina 70-210mm F2.8-4 AF" },
        { 138,   "Soligor 19-35mm F3.5-4.5" },
        { 142,   "Voigtlander 70-300mm F4.5-5.6" },
        { 146,   "Voigtlander Macro APO-Lanthar 125mm F2.5 SL" },
        { 193,   "Minolta AF 1.4x APO II" },
        { 255,   "Tamron SP AF 17-50mm F2.8 XR Di II LD Aspherical | "
                 "Tamron AF 18-250mm F3.5-6.3 XR Di II LD | "
                 "Tamron AF 55-200mm F4-5.6 Di II | "
                 "Tamron AF 70-300mm F4-5.6 Di LD MACRO 1:2 | "
                 "Tamron SP AF 200-500mm F5.0-6.3 Di LD IF | "
                 "Tamron SP AF 10-24mm F3.5-4.5 Di II LD Aspherical IF | "
                 "Tamron SP AF 70-200mm F2.8 Di LD IF Macro | "
                 "Tamron SP AF 28-75mm F2.8 XR Di LD Aspherical IF" },
        { 25500, "Minolta AF 50mm F1.7" },
        { 25501, "Minolta AF 50mm F1.7" },
        { 25510, "Minolta AF 35-70mm F1.4" },
        { 25511, "Minolta AF 35-70mm F4 | "
                 "Sigma UC AF 28-70mm F3.5-4.5 | "
                 "Sigma AF 28-70mm F2.8 | "
                 "Sigma M-AF 70-200mm F2.8 EX Aspherical | "
                 "Quantaray M-AF 35-80mm F4-5.6 " },
        { 25520, "Minolta AF 28-85mm F3.5-4.5" },
        { 25521, "Minolta AF 28-85mm F3.5-4.5 | "
                 "Tokina 19-35mm F3.5-4.5 | "
                 "Tokina 28-70mm F2.8 AT-X | "
                 "Tokina 80-400mm F4.5-5.6 AT-X AF II 840 | "
                 "Tokina AF PRO 28-80mm F2.8 AT-X 280 | "
                 "Tokina AT-X PRO II AF 28-70mm F2.6-2.8 270 | "
                 "Tamron AF 19-35mm F3.5-4.5 | "
                 "Angenieux AF 28-70mm F2.6" },
        { 25530, "Minolta AF 28-135mm F4-4.5" },
        { 25531, "Minolta AF 28-135mm F4-4.5 | "
                 "Sigma ZOOM-alpha 35-135mm F3.5-4.5 | "
                 "Sigma 28-105mm F2.8-4 Aspherical" },
        { 25540, "Minolta AF 35-105mm F3.5-4.5" },
        { 25541, "Minolta AF 35-105mm F3.5-4.5" },
        { 25550, "Minolta AF 70-210mm F4" },
        { 25551, "Minolta AF 70-210mm F4 Macro | "
                 "Sigma 70-210mm F4-5.6 APO | "
                 "Sigma M-AF 70-200mm F2.8 EX APO | "
                 "Sigma 75-200mm F2.8-3.5" },
        { 25560, "Minolta AF 135mm F2.8" },
        { 25561, "Minolta AF 135mm F2.8" },
        { 25570, "Minolta AF 28mm F2.8" },
        { 25571, "Minolta/Sony AF 28mm F2.8" },
        { 25580, "Minolta AF 24-50mm F4" },
        { 25581, "Minolta AF 24-50mm F4" },
        { 25600, "Minolta AF 100-200mm F4.5" },
        { 25601, "Minolta AF 100-200mm F4.5" },
        { 25610, "Minolta AF 75-300mm F4.5-5.6" },
        { 25611, "Minolta AF 75-300mm F4.5-5.6 | "
                 "Sigma 70-300mm F4-5.6 DL Macro | "
                 "Sigma 300mm F4 APO Macro | "
                 "Sigma AF 500mm F4.5 APO | "
                 "Sigma AF 170-500mm F5-6.3 APO Aspherical | "
                 "Tokina AT-X AF 300mm F4 | "
                 "Tokina AT-X AF 400mm F5.6 SD | "
                 "Tokina AF 730 II 75-300mm F4.5-5.6" },
        { 25620, "Minolta AF 50mm F1.4" },
        { 25621, "Minolta AF 50mm F1.4 [New]" },
        { 25630, "Minolta AF 300mm F2.8G APO" },
        { 25631, "Minolta AF 300mm F2.8 APO | "
                 "Sigma AF 50-500mm F4-6.3 EX DG APO | "
                 "Sigma AF 170-500mm F5-6.3 APO Aspherical | "
                 "Sigma AF 500mm F4.5 EX DG APO | "
                 "Sigma 400mm F5.6 APO" },
        { 25640, "Minolta AF 50mm F2.8 Macro" },
        { 25641, "Minolta AF 50mm F2.8 Macro | "
                 "Sigma 50mm F2.8 EX Macro" },
        { 25650, "Minolta AF 600mm F4 APO" },
        { 25651, "Minolta AF 600mm F4 APO" },
        { 25660, "Minolta AF 24mm F2.8" },
        { 25661, "Minolta AF 24mm F2.8 | "
                 "Sigma 17-35mm F2.8-4.0 EX-D" },
        { 25720, "Minolta AF 500mm F8 Reflex" },
        { 25721, "Minolta/Sony AF 500mm F8 Reflex" },
        { 25780, "Minolta/Sony AF 16mm F2.8 Fisheye" },
        { 25781, "Minolta/Sony AF 16mm F2.8 Fisheye | "
                 "Sigma 8mm F4 EX [DG] Fisheye | "
                 "Sigma 14mm F3.5 | "
                 "Sigma 15mm F2.8 Fisheye" },
        { 25790, "Minolta AF 20mm F2.8" },
        { 25791, "Minolta/Sony AF 20mm F2.8" },
        { 25810, "Minolta AF 100mm F2.8 Macro" },
        { 25811, "Minolta AF 100mm F2.8 Macro [New] | "
                 "Sigma AF 90mm F2.8 Macro | "
                 "Sigma AF 105mm F2.8 EX [DG] Macro | "
                 "Sigma 180mm F5.6 Macro | "
                 "Tamron 90mm F2.8 Macro" },
        { 25850, "Minolta AF 35-105mm F3.5-4.5" },
        { 25851, "Beroflex 35-135mm F3.5-4.5" },
        { 25858, "Minolta AF 35-105mm F3.5-4.5 New | "
                 "Tamron 24-135mm F3.5-5.6" },
        { 25880, "Minolta AF 70-210mm F3.5-4.5" },
        { 25881, "Minolta AF 70-210mm F3.5-4.5" },
        { 25890, "Minolta AF 80-200mm F2.8 APO" },
        { 25891, "Minolta AF 80-200mm F2.8 APO | "
                 "Tokina 80-200mm F2.8" },
        { 25910, "Minolta AF 35mm F1.4G" },
        { 25911, "Minolta AF 35mm F1.4" },
        { 25920, "Minolta AF 85mm F1.4G" },
        { 25921, "Minolta AF 85mm F1.4G(D)" },
        { 25930, "Minolta AF 200mm F2.8 APO" },
        { 25931, "Minolta AF 200mm F2.8 G APO" },
        { 25940, "Minolta AF 3X-1X F1.7-2.8 Macro" },
        { 25941, "Minolta AF 3x-1x F1.7-2.8 Macro" },
        { 25960, "Minolta AF 28mm F2" },
        { 25961, "Minolta AF 28mm F2" },
        { 25970, "Minolta AF 35mm F2" },
        { 25971, "Minolta AF 35mm F2 [New]" },
        { 25980, "Minolta AF 100mm F2" },
        { 25981, "Minolta AF 100mm F2" },
        { 26040, "Minolta AF 80-200mm F4.5-5.6" },
        { 26041, "Minolta AF 80-200mm F4.5-5.6" },
        { 26050, "Minolta AF 35-80mm F4-5.6" },
        { 26051, "Minolta AF 35-80mm F4-5.6" },
        { 26060, "Minolta AF 100-300mm F4.5-5.6" },
        { 26061, "Minolta AF 100-300mm F4.5-5.6(D) | "
                 "Sigma 105mm F2.8 Macro EX-DG" },
        { 26070, "Minolta AF 35-80mm F4-5.6" },
        { 26071, "Minolta AF 35-80mm F4-5.6" },
        { 26080, "Minolta AF 300mm F2.8G APO High Speed" },
        { 26081, "Minolta AF 300mm F2.8G" },
        { 26090, "Minolta AF 600mm F4G APO High Speed" },
        { 26091, "Minolta AF 600mm F4 HS-APO G" },
        { 26120, "Minolta AF 200mm F2.8G APO High Speed" },
        { 26121, "Minolta AF 200mm F2.8G(D)" },
        { 26130, "Minolta AF 50mm F1.7" },
        { 26131, "Minolta AF 50mm F1.7 New" },
        { 26150, "Minolta AF 28-105mm F3.5-4.5 Xi" },
        { 26151, "Minolta AF 28-105mm F3.5-4.5 xi" },
        { 26160, "Minolta AF 35-200mm F4.5-5.6 Xi" },
        { 26161, "Minolta AF 35-200mm F4.5-5.6 Xi" },
        { 26180, "Minolta AF 28-80mm F4-5.6 Xi" },
        { 26181, "Minolta AF 28-80mm F4-5.6 xi" },
        { 26190, "Minolta AF 80-200mm F4.5-5.6 Xi" },
        { 26191, "Minolta AF 80-200mm F4.5-5.6 Xi" },
        { 26201, "Minolta AF 28-70mm F2.8 G" },
        { 26210, "Minolta AF 100-300mm F4.5-5.6 Xi" },
        { 26211, "Minolta AF 100-300mm F4.5-5.6 xi" },
        { 26240, "Minolta AF 35-80mm F4-5.6 Power" },
        { 26241, "Minolta AF 35-80mm F4-5.6 Power Zoom" },
        { 26281, "Minolta AF 80-200mm F2.8 G" },
        { 26291, "Minolta AF 85mm F1.4 New" },
        { 26311, "Minolta/Sony AF 100-300mm F4.5-5.6 APO" },
        { 26321, "Minolta AF 24-50mm F4 New" },
        { 26381, "Minolta AF 50mm F2.8 Macro New" },
        { 26391, "Minolta AF 100mm F2.8 Macro" },
        { 26411, "Minolta/Sony AF 20mm F2.8 New" },
        { 26421, "Minolta AF 24mm F2.8 New" },
        { 26441, "Minolta AF 100-400mm F4.5-6.7 APO" },
        { 26621, "Minolta AF 50mm F1.4 New" },
        { 26671, "Minolta AF 35mm F2 New" },
        { 26681, "Minolta AF 28mm F2 New" },
        { 26721, "Minolta AF 24-105mm F3.5-4.5 (D)" },
        { 45671, "Tokina 70-210mm F4-5.6" },
        { 45741, "Minolta AF200mm F2.8G x2 | "
                 "Tokina 300mm F2.8 x2 | "
                 "Tokina RF 500mm F8.0 x2 | "
                 "Tamron SP AF 90mm F2.5"},
        { 45751, "1.4x Teleconverter " },
        { 45851, "Tamron SP AF 300mm F2.8 LD IF" },
        { 65535, "T-Mount | "
                 "Arax MC 35mm F2.8 Tilt+Shift | "
                 "Arax MC 80mm F2.8 Tilt+Shift | "
                 "Zenitar MF 16mm F2.8 Fisheye M42 | "
                 "Samyang 500mm Mirror F8.0 | "
                 "Pentacon Auto 135mm F2.8 | "
                 "Pentacon Auto 29mm F2.8 | "
                 "Helios 44-2 58mm F2.0 | "
                 "No Lens" }
    };

    std::ostream& printMinoltaSonyLensID(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyLensID)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Minolta A100 and all other Sony Alpha camera color mode values to readable labels
    extern const TagDetails minoltaSonyColorMode[] = {
        { 0,   N_("Standard")            },
        { 1,   N_("Vivid Color")         },
        { 2,   N_("Portrait")            },
        { 3,   N_("Landscape")           },
        { 4,   N_("Sunset")              },
        { 5,   N_("Night View/Portrait") },
        { 6,   N_("Black & White")       },
        { 7,   N_("AdobeRGB")            },
        { 12,  N_("Neutral")             },
        { 100, N_("Neutral")             },
        { 101, N_("Clear")               },
        { 102, N_("Deep")                },
        { 103, N_("Light")               },
        { 104, N_("Night View")          },
        { 105, N_("Autumn Leaves")       }
    };

    std::ostream& printMinoltaSonyColorMode(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyColorMode)(os, value, metadata);
    }

    // ----------------------------------------------------------------------------------------------------

    //! Lookup table to translate Minolta/Sony macro mode values to readable labels
    extern const TagDetails minoltaSonyMacroMode[] = {
        { 0, N_("Off") },
        { 1, N_("On")  }
    };

    std::ostream& printMinoltaSonyMacroMode(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(minoltaSonyMacroMode)(os, value, metadata);
    }
}                                       // namespace Exiv2
