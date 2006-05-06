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

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! @cond IGNORE
    MinoltaMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote(
            "KONICA MINOLTA*", "*", createMinoltaMakerNote);
        MakerNoteFactory::registerMakerNote(
            minoltaIfdId, MakerNote::AutoPtr(new MinoltaMakerNote));

        ExifTags::registerMakerTagInfo(minoltaIfdId, tagInfo_);
    }
    //! @endcond

    // Minolta Tag Info
    const TagInfo MinoltaMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "Version", "Makernote Version", "String 'MLT0' (not null terminated)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0001, "CameraSettingsOld", "Camera Settings (Old)", "Camera settings (old)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0003, "CameraSettingsNew", "Camera Settings (New)", "Camera settings (old)", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0040, "CompressedImageSize", "Compressed Image Size", "Compressed image size", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0081, "Thumbnail", "Thumbnail", "Jpeg thumbnail 640x480 pixels", minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0088, "ThumbnailOffset", "Thumbnail Offset", "Offset of the thumbnail", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0089, "ThumbnailLength", "Thumbnail Length", "Size of the thumbnail", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0101, "ColorMode", "Color Mode", "Color mode", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0102, "ImageQuality", "Image Quality", "Image quality", minoltaIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0e00, "PIM_IFD", "PIM IFD", "PIM information", minoltaIfdId, makerTags, undefined, printValue),
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
