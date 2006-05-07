// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      minoltamn.cpp
  Version:   $Rev$
  Author(s): Gilles Caulier (gc) <caulier.gilles@kdemail.net>
             Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   06-May-06, gc: submitted
  Credits:   See header file.
 */

// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "types.hpp"
#include "minoltamn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "tags.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! Lookup table to translate Minolta color mode values to readable labels
    extern const TagDetails minoltaColorMode[] = {
        { 0,  "Natural Color" },
        { 1,  "Black & White" },
        { 2,  "Vivid Color"   },
        { 3,  "Solarization"  },
        { 4,  "AdobeRGB"      },
        { 13, "Natural sRGB"  },
        { 14, "Natural+ sRGB" }
    };

    //! Lookup table to translate Minolta image quality values to readable labels
    extern const TagDetails minoltaImageQuality[] = {
        { 0, "Raw"       },
        { 1, "Superfine" },
        { 2, "Fine"      },
        { 3, "Standard"  },
        { 4, "Economy"   },
        { 5, "Extrafine" }
    };

    //! Lookup table to translate Minolta image stabilization values
    extern const TagDetails minoltaImageStabilization[] = {
        { 1, "On"  },
        { 5, "Off" }
    };

    //! Lookup table to translate Minolta zone matching values
    extern const TagDetails minoltaZoneMatching[] = {
        { 0, "ISO Setting Used" },
        { 1, "High Key"         },
        { 2, "Low Key"          }
    };

    //! Lookup table to translate Minolta Lens id values to readable labels
    extern const TagDetails minoltaLensID[] = {
        { 1,     "AF80-200mm F2.8G" },
        { 2,     "AF28-70mm F2.8G" }, 
        { 6,     "AF24-85mm F3.5-4.5" },
        { 7,     "AF100-400mm F4.5-6.7(D)" },
        { 11,    "AF300mm F4G" },
        { 12,    "AF100mm F2.8 Soft" },
        { 15,    "AF400mm F4.5G" },
        { 16,    "AF17-35mm F3.5G" },
        { 19,    "AF35mm/1.4" },
        { 20,    "STF135mm F2.8[T4.5]" },
        { 23,    "AF200mm F4G Macro" },
        { 24,    "AF24-105mm F3.5-4.5(D) or SIGMA 18-50mm F2.8" },
        { 25,    "AF100-300mm F4.5-5.6(D)" },
        { 27,    "AF85mm F1.4G" },
        { 28,    "AF100mm F2.8 Macro(D)" },
        { 29,    "AF75-300mm F4.5-5.6(D)" }, 
        { 30,    "AF28-80mm F3.5-5.6(D)" }, 
        { 31,    "AF50mm F2.8 Macro(D) or AF50mm F3.5 Macro" }, 
        { 32,    "AF100-400mm F4.5-6.7(D) x1.5" }, 
        { 33,    "AF70-200mm F2.8G SSM" }, 
        { 35,    "AF85mm F1.4G(D) Limited" }, 
        { 38,    "AF17-35mm F2.8-4(D)" }, 
        { 39,    "AF28-75mm F2.8(D)" }, 
        { 40,    "AFDT18-70mm F3.5-5.6(D)" },
        { 128,   "TAMRON 18-200, 28-300 or 80-300mm F3.5-6.3" },
        { 25501, "AF50mm F1.7" }, 
        { 25521, "TOKINA 19-35mm F3.5-4.5 or TOKINA 28-70mm F2.8 AT-X" }, 
        { 25541, "AF35-105mm F3.5-4.5" }, 
        { 25551, "AF70-210mm F4 Macro or SIGMA 70-210mm F4-5.6 APO" }, 
        { 25581, "AF24-50mm F4" }, 
        { 25611, "SIGMA 70-300mm F4-5.6 or SIGMA 300mm F4 APO Macro" }, 
        { 25621, "AF50mm F1.4 NEW" }, 
        { 25631, "AF300mm F2.8G" }, 
        { 25641, "AF50mm F2.8 Macro" },
        { 25661, "AF24mm F2.8" }, 
        { 25721, "AF500mm F8 Reflex" }, 
        { 25781, "AF16mm F2.8 Fisheye or SIGMA 8mm F4 Fisheye" }, 
        { 25791, "AF20mm F2.8" }, 
        { 25811, "AF100mm F2.8 Macro(D), TAMRON 90mm F2.8 Macro or SIGMA 180mm F5.6 Macro" }, 
        { 25858, "TAMRON 24-135mm F3.5-5.6" }, 
        { 25891, "TOKINA 80-200mm F2.8" }, 
        { 25921, "AF85mm F1.4G(D)" }, 
        { 25931, "AF200mm F2.8G" }, 
        { 25961, "AF28mm F2" }, 
        { 25981, "AF100mm F2" }, 
        { 26061, "AF100-300mm F4.5-5.6(D)" }, 
        { 26081, "AF300mm F2.8G" }, 
        { 26121, "AF200mm F2.8G(D)" }, 
        { 26131, "AF50mm F1.7" }, 
        { 26241, "AF35-80mm F4-5.6" }, 
        { 45741, "AF200mm F2.8G x2 or TOKINA 300mm F2.8 x2" }
    };

    //! @cond IGNORE
    MinoltaMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("KONICA MINOLTA*", "*", createMinoltaMakerNote);
        MakerNoteFactory::registerMakerNote("Minolta*", "*", createMinoltaMakerNote);
        MakerNoteFactory::registerMakerNote(
            minoltaIfdId, MakerNote::AutoPtr(new MinoltaMakerNote));

        ExifTags::registerMakerTagInfo(minoltaIfdId, tagInfo_);
    }
    //! @endcond

    // Minolta Tag Info
    const TagInfo MinoltaMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "Version", "Makernote Version", "String 'MLT0' (not null terminated)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0001, "CameraSettingsOld", "Camera Settings (Old)", "Camera settings (old)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0003, "CameraSettingsNew", "Camera Settings (New)", "Camera settings (new)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0004, "CameraSettings7D", "Camera Settings (7D)", "Camera Settings (7D)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0040, "CompressedImageSize", "Compressed Image Size", "Compressed image size", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0081, "Thumbnail", "Thumbnail", "Jpeg thumbnail 640x480 pixels", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0088, "ThumbnailOffset", "Thumbnail Offset", "Offset of the thumbnail", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0089, "ThumbnailLength", "Thumbnail Length", "Size of the thumbnail", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0101, "ColorMode", "Color Mode", "Color mode", minoltaIfdId, makerTags, unsignedLong, printTag<COUNTOF(minoltaColorMode), minoltaColorMode>),
        TagInfo(0x0102, "ImageQuality", "Image Quality", "Image quality", minoltaIfdId, makerTags, unsignedLong, printTag<COUNTOF(minoltaImageQuality), minoltaImageQuality>),
        
        // Todo: Tag 0x0103 : quality or image size (see ExifTool doc).

        TagInfo(0x0107, "ImageStabilization", "Image Stabilization", "Image stabilization", minoltaIfdId, makerTags, unsignedLong, printTag<COUNTOF(minoltaImageStabilization), minoltaImageStabilization>),
        TagInfo(0x010a, "ZoneMatching", "Zone Matching", "Zone matching", minoltaIfdId, makerTags, unsignedLong, printTag<COUNTOF(minoltaZoneMatching), minoltaZoneMatching>),
        
        // Todo: this tag is important. Check ExifTool doc when description will be updated.
        TagInfo(0x010b, "ColorTemperature", "Color Temperature", "Color temperature", minoltaIfdId, makerTags, unsignedLong, printValue),    
        
        TagInfo(0x010c, "LensID", "Lens ID", "Lens ID", minoltaIfdId, makerTags, unsignedLong, printTag<COUNTOF(minoltaLensID), minoltaLensID>),    
        
        TagInfo(0x0114, "CameraSettings5D", "Camera Settings (5D)", "Camera Settings (5D)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0e00, "PIM_IFD", "PIM IFD", "PIM information", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0f00, "CameraSettingsNew2", "Camera Settings (New2)", "Camera Settings (new2)", minoltaIfdId, makerTags, undefined, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownMinoltaMakerNoteTag)", "(UnknownMinoltaMakerNoteTag)", "Unknown MinoltaMakerNote tag", minoltaIfdId, makerTags, invalidTypeId, printValue)
    };

    MinoltaMakerNote::MinoltaMakerNote(bool alloc)
        : IfdMakerNote(minoltaIfdId, alloc)
    {
    }

    MinoltaMakerNote::MinoltaMakerNote(const MinoltaMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    MinoltaMakerNote::AutoPtr MinoltaMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    MinoltaMakerNote* MinoltaMakerNote::create_(bool alloc) const
    {
        return new MinoltaMakerNote(alloc);
    }

    MinoltaMakerNote::AutoPtr MinoltaMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    MinoltaMakerNote* MinoltaMakerNote::clone_() const
    {
        return new MinoltaMakerNote(*this);
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createMinoltaMakerNote(bool alloc,
                                              const byte* /*buf*/,
                                              long /*len*/,
                                              ByteOrder /*byteOrder*/,
                                              long /*offset*/)
    {
        return MakerNote::AutoPtr(new MinoltaMakerNote(alloc));
    }

}                                       // namespace Exiv2
